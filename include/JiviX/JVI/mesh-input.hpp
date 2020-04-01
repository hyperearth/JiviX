#pragma once // #

#include "./config.hpp"
#include "./driver.hpp"
#include "./thread.hpp"
#include "./context.hpp"
#include "./mesh.hpp"

namespace jvi {

    // WARNING!!
    // FOR VULKAN API ONLY!!
    // PLANNED: OPENGL VERSION!!
    class MeshInput : public std::enable_shared_from_this<MeshInput> {
    public: friend Node; friend Renderer; friend MeshBinding;
        MeshInput() {};
        MeshInput(const vkt::uni_ptr<Context> & context) : context(context) { this->construct(); };
        ~MeshInput() {};

        // 
        virtual uPTR(MeshInput) construct() {
            this->driver = context->getDriver();
            this->thread = std::make_shared<Thread>(this->driver);

            // 
            this->pipelineInfo = vkh::VsGraphicsPipelineCreateInfoConstruction();

            // 
            this->quadStage = vkt::makePipelineStageInfo(this->driver->getDevice(), vkt::readBinary("./shaders/rtrace/quad.comp.spv"), vk::ShaderStageFlagBits::eCompute);
            this->counterData = vkt::Vector<vkh::VkAccelerationStructureBuildOffsetInfoKHR>(this->driver->getAllocator(), vkh::VkBufferCreateInfo{ .size = sizeof(vkh::VkAccelerationStructureBuildOffsetInfoKHR), .usage = { .eTransferSrc = 1, .eTransferDst = 1, .eUniformBuffer = 1, .eStorageBuffer = 1, .eIndirectBuffer = 1, .eRayTracing = 1, .eTransformFeedbackCounterBuffer = 1, .eSharedDeviceAddress = 1 } }, VMA_MEMORY_USAGE_GPU_TO_CPU);

            // for faster code, pre-initialize
            this->stages = vkt::vector_cast<vkh::VkPipelineShaderStageCreateInfo, vk::PipelineShaderStageCreateInfo>({
                vkt::makePipelineStageInfo(this->driver->getDevice(), vkt::readBinary("./shaders/rtrace/transform.vert.spv"), vk::ShaderStageFlagBits::eVertex),
                vkt::makePipelineStageInfo(this->driver->getDevice(), vkt::readBinary("./shaders/rtrace/transform.geom.spv"), vk::ShaderStageFlagBits::eGeometry)
            });

            // transformPipelineLayout
            // create required buffers
            this->rawBindings = vkt::Vector<VkVertexInputBindingDescription>(this->driver->getAllocator(), vkh::VkBufferCreateInfo{ .size = sizeof(VkVertexInputBindingDescription) * 8u, .usage = {.eTransferSrc = 1, .eUniformBuffer = 1, .eStorageBuffer = 1, .eRayTracing = 1 } }, VMA_MEMORY_USAGE_CPU_TO_GPU);
            this->gpuBindings = vkt::Vector<VkVertexInputBindingDescription>(this->driver->getAllocator(), vkh::VkBufferCreateInfo{ .size = sizeof(VkVertexInputBindingDescription) * 8u, .usage = {.eTransferDst = 1, .eUniformBuffer = 1, .eStorageBuffer = 1, .eRayTracing = 1 } }, VMA_MEMORY_USAGE_GPU_ONLY);
            this->rawAttributes = vkt::Vector<VkVertexInputAttributeDescription>(this->driver->getAllocator(), vkh::VkBufferCreateInfo{ .size = sizeof(VkVertexInputAttributeDescription) * 8u, .usage = {.eTransferSrc = 1, .eUniformBuffer = 1, .eStorageBuffer = 1, .eRayTracing = 1 } }, VMA_MEMORY_USAGE_CPU_TO_GPU);
            this->gpuAttributes = vkt::Vector<VkVertexInputAttributeDescription>(this->driver->getAllocator(), vkh::VkBufferCreateInfo{ .size = sizeof(VkVertexInputAttributeDescription) * 8u, .usage = {.eTransferDst = 1, .eUniformBuffer = 1, .eStorageBuffer = 1, .eRayTracing = 1 } }, VMA_MEMORY_USAGE_GPU_ONLY);

            // 
            return uTHIS;
        };

        // 
        virtual uPTR(MeshInput) createDescriptorSet() { // 

            // 
            this->descriptorSetHelper = vkh::VsDescriptorSetCreateInfoHelper(this->transformSetLayout[0], this->driver->getDescriptorPool());

            // 
            this->descriptorSetHelper.pushDescription(vkh::VkDescriptorUpdateTemplateEntry{
                .dstBinding = 2u,
                .dstArrayElement = 0u,
                .descriptorCount = 1u,
                .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER
            }).offset<vk::DescriptorBufferInfo>() = gpuBindings;

            // 
            this->descriptorSetHelper.pushDescription(vkh::VkDescriptorUpdateTemplateEntry{
                .dstBinding = 3u,
                .dstArrayElement = 0u,
                .descriptorCount = 1u,
                .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER
            }).offset<vk::DescriptorBufferInfo>() = gpuAttributes;

            // 
            for (uint32_t j = 0; j < this->bindings.size(); j++) {
                auto& handle = this->descriptorSetHelper.pushDescription(vkh::VkDescriptorUpdateTemplateEntry{
                    .dstBinding = 0u,
                    .dstArrayElement = j,
                    .descriptorCount = 1u,
                    .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER
                }).offset<vk::BufferView>(0u) = this->bindings[j].createBufferView(vk::Format::eR8Uint);
            };

            if (this->indexData.has()) {
                auto& handle = this->descriptorSetHelper.pushDescription(vkh::VkDescriptorUpdateTemplateEntry{
                    .dstBinding = 1u,
                    .dstArrayElement = 0u,
                    .descriptorCount = 1u,
                    .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER
                }).offset<vk::BufferView>(0u) = this->indexData.createBufferView(vk::Format::eR8Uint);
            };

            // 
            driver->getDevice().updateDescriptorSets(vkt::vector_cast<vk::WriteDescriptorSet, vkh::VkWriteDescriptorSet>(
                this->descriptorSetHelper.setDescriptorSet((this->descriptorSet = driver->getDevice().allocateDescriptorSets(this->descriptorSetHelper))[0])
            ), {});

            return uTHIS;
        }

        // 
        virtual uPTR(MeshInput) copyMeta(vk::CommandBuffer buildCommand = {}) {
            buildCommand.copyBuffer(this->rawAttributes, this->gpuAttributes, { vk::BufferCopy{ this->rawAttributes.offset(), this->gpuAttributes.offset(), this->gpuAttributes.range() } });
            buildCommand.copyBuffer(this->rawBindings, this->gpuBindings, { vk::BufferCopy{ this->rawBindings.offset(), this->gpuBindings.offset(), this->gpuBindings.range() } });
            return uTHIS;
        }

        // Record Geometry (Transform Feedback)
        virtual uPTR(MeshInput) buildGeometry(const vkt::Vector<uint8_t>& OutPut, vk::CommandBuffer buildCommand = {}) { // 
            bool DirectCommand = false;

            // 
            if (!buildCommand || ignoreIndirection) {
                buildCommand = vkt::createCommandBuffer(this->thread->getDevice(), this->thread->getCommandPool()); DirectCommand = true;
            };

            // TODO: Add QUADs support for GEN-2.0
            if (buildCommand && this->needsQuads) {
                this->needsQuads = false; // FOR MINECRAFT ONLY! 
                this->quadInfo.layout = this->transformPipelineLayout;
                this->quadInfo.stage = this->quadStage;
                this->quadGenerator = vkt::createCompute(driver->getDevice(), vkt::FixConstruction(this->quadStage), vk::PipelineLayout(this->quadInfo.layout), driver->getPipelineCache());

                // 
                buildCommand.bindDescriptorSets(vk::PipelineBindPoint::eCompute, this->transformPipelineLayout, 0ull, this->descriptorSet, {});
                buildCommand.bindPipeline(vk::PipelineBindPoint::eCompute, this->quadGenerator);
                buildCommand.pushConstants<glm::uvec4>(this->transformPipelineLayout, vkh::VkShaderStageFlags{.eVertex = 1, .eGeometry = 1, .eFragment = 1, .eCompute = 1, .eRaygen = 1, .eClosestHit = 1, .eMiss = 1 }.hpp(), 0u, {  /* TODO: Mesh Meta */  });
                buildCommand.dispatch(vkt::tiled(this->currentUnitCount, 1024ull), 1u, 1u);
            } else 

            if (buildCommand && this->needUpdate) { this->needUpdate = false; // 
                std::vector<vk::Buffer> buffers = {}; std::vector<vk::DeviceSize> offsets = {};
                buffers.resize(this->bindings.size()); offsets.resize(this->bindings.size()); uintptr_t I = 0u;
                for (auto& B : this->bindings) { if (B.has()) { const uintptr_t i = I++; buffers[i] = B.buffer(); offsets[i] = B.offset(); }; };

                // 
                const auto& viewport = this->context->refViewport();
                const auto& renderArea = this->context->refScissor();
                const auto clearValues = std::vector<vk::ClearValue>{
                    vk::ClearColorValue(std::array<float,4>{0.f, 0.f, 0.f, 0.0f}),
                    vk::ClearColorValue(std::array<float,4>{0.f, 0.f, 0.f, 0.0f}),
                    vk::ClearColorValue(std::array<float,4>{0.f, 0.f, 0.f, 0.0f}),
                    vk::ClearColorValue(std::array<float,4>{0.f, 0.f, 0.f, 0.0f}),
                    vk::ClearColorValue(std::array<float,4>{0.f, 0.f, 0.f, 0.0f}),
                    vk::ClearColorValue(std::array<float,4>{0.f, 0.f, 0.f, 0.0f}),
                    vk::ClearColorValue(std::array<float,4>{0.f, 0.f, 0.f, 0.0f}),
                    vk::ClearColorValue(std::array<float,4>{0.f, 0.f, 0.f, 0.0f}),
                    vk::ClearDepthStencilValue(1.0f, 0)
                };

                // 
                auto offsetsInfo = vkh::VkAccelerationStructureBuildOffsetInfoKHR{ .primitiveCount = 0u };

                // 
                meta.prmCount = uint32_t(this->currentUnitCount) / 3u;
                meta.indexType = int32_t(this->indexType) + 1;

                // 
                vkt::debugLabel(buildCommand, "Begin building geometry data...", this->driver->getDispatch());
                buildCommand.updateBuffer(counterData.buffer(), counterData.offset(), sizeof(vkh::VkAccelerationStructureBuildOffsetInfoKHR), &offsetsInfo); // Nullify Counters
                buildCommand.beginRenderPass(vk::RenderPassBeginInfo(this->context->refRenderPass(), this->context->deferredFramebuffer, renderArea, static_cast<uint32_t>(clearValues.size()), clearValues.data()), vk::SubpassContents::eInline);
                buildCommand.beginTransformFeedbackEXT(0u, { counterData.buffer() }, { counterData.offset() }, this->driver->getDispatch()); //!!WARNING!!
                buildCommand.setViewport(0, { viewport });
                buildCommand.setScissor(0, { renderArea });
                buildCommand.bindTransformFeedbackBuffersEXT(0u, { OutPut.buffer() }, { OutPut.offset() }, { OutPut->range() }, this->driver->getDispatch()); //!!WARNING!!
                buildCommand.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, this->transformPipelineLayout, 0ull, this->descriptorSet, {});
                buildCommand.bindPipeline(vk::PipelineBindPoint::eGraphics, this->transformState);
                buildCommand.bindVertexBuffers(0u, buffers, offsets);
                buildCommand.pushConstants<jvi::MeshInfo>(this->transformPipelineLayout, vkh::VkShaderStageFlags{ .eVertex = 1, .eGeometry = 1, .eFragment = 1, .eCompute = 1, .eRaygen = 1, .eClosestHit = 1, .eMiss = 1 }.hpp(), 0u, { meta });

                // No need more indices... (SSBO used instead)
                //buildCommand.draw(this->currentUnitCount, 1u, 0u, 0u);

                // 
                if (this->indexType != vk::IndexType::eNoneKHR) { // PLC Mode
                    const uintptr_t voffset = 0u;//this->bindings[this->vertexInputAttributeDescriptions[0u].binding].offset(); // !!WARNING!!
                    buildCommand.bindIndexBuffer(this->indexData, this->indexData.offset(), this->indexType);
                    buildCommand.drawIndexed(this->currentUnitCount, 1u, 0u, voffset, 0u);
                } else { // VAL Mode
                    buildCommand.draw(this->currentUnitCount, 1u, 0u, 0u);
                };

                buildCommand.endTransformFeedbackEXT(0u, { counterData.buffer() }, { counterData.offset() }, this->driver->getDispatch()); //!!WARNING!!
                buildCommand.endRenderPass();
                vkt::debugLabel(buildCommand, "Ending building geometry data...", this->driver->getDispatch());
                //buildCommand.endDebugUtilsLabelEXT(this->driver->getDispatch());
                //buildCommand.insertDebugUtilsLabelEXT(vk::DebugUtilsLabelEXT().setColor({ 1.f,0.75,0.25f }).setPLabelName("Building Geometry Complete.."), this->driver->getDispatch());
                //vkt::commandBarrier(buildCommand);
            }

            if (DirectCommand) {
                vkt::submitCmd(this->thread->getDevice(), this->thread->getQueue(), { buildCommand });
                this->thread->getDevice().freeCommandBuffers(this->thread->getCommandPool(), { buildCommand });
            }

            return uTHIS;
        };

        // 
        template<class T = uint8_t>
        inline uPTR(MeshInput) addBinding(const vkt::Vector<T>& rawData, const vkt::uni_arg<vkh::VkVertexInputBindingDescription>& binding = vkh::VkVertexInputBindingDescription{}) {
            const uintptr_t bindingID = this->vertexInputBindingDescriptions.size();
            this->vertexInputBindingDescriptions.resize(bindingID + 1u);
            this->vertexInputBindingDescriptions[bindingID] = binding;
            this->vertexInputBindingDescriptions[bindingID].binding = static_cast<uint32_t>(bindingID);
            this->rawBindings[bindingID] = this->vertexInputBindingDescriptions[bindingID];

            /* // 
            this->descriptorSetHelper.pushDescription(vkh::VkDescriptorUpdateTemplateEntry{
                .dstBinding = 0u,
                .dstArrayElement = uint32_t(bindingID),
                .descriptorCount = 1u,
                .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER
            }).offset<vk::BufferView>() = rawData.createBufferView(vk::Format::eR8Uint);
            */

            // 
            this->bindRange.resize(bindingID + 1u);
            this->bindRange[this->lastBindID = static_cast<uint32_t>(bindingID)] = rawData.range();
            this->bindings.resize(bindingID+1u);
            this->bindings[bindingID] = rawData;
            return uTHIS;
        };

        // 
        virtual uPTR(MeshInput) addAttribute(const vkt::uni_arg<vkh::VkVertexInputAttributeDescription>& attribute = vkh::VkVertexInputAttributeDescription{}, const bool& NotStub = true) {
            const uintptr_t bindingID = this->lastBindID;//attribute->binding;
            const uintptr_t locationID = attribute->location;
            this->vertexInputAttributeDescriptions.resize(locationID + 1u);
            this->vertexInputAttributeDescriptions[locationID] = attribute;
            this->vertexInputAttributeDescriptions[locationID].binding = static_cast<uint32_t>(bindingID);
            this->vertexInputAttributeDescriptions[locationID].location = static_cast<uint32_t>(locationID);
            this->rawAttributes[locationID] = this->vertexInputAttributeDescriptions[locationID];

            // 
            if (locationID == 0u && NotStub && this->indexType == vk::IndexType::eNoneKHR) {
                this->currentUnitCount = this->bindRange[bindingID] / this->bindings[bindingID].stride();
            };

            // 
            if (rawMeshInfo.has()) {
                if (locationID == 1u && NotStub) { rawMeshInfo[0].hasTexcoord = meta.hasTexcoord = 1; };
                if (locationID == 2u && NotStub) { rawMeshInfo[0].hasNormal = meta.hasNormal = 1; };
                if (locationID == 3u && NotStub) { rawMeshInfo[0].hasTangent = meta.hasTangent = 1; };
            } else {
                if (locationID == 1u && NotStub) { meta.hasTexcoord = 1; };
                if (locationID == 2u && NotStub) { meta.hasNormal = 1; };
                if (locationID == 3u && NotStub) { meta.hasTangent = 1; };
            };

            // 
            return uTHIS;
        };

        // 
        template<class T = uint8_t>
        inline uPTR(MeshInput) setIndexData(const vkt::Vector<T>& rawIndices, const vk::IndexType& type) {
            vk::DeviceSize count = 0u; uint32_t stride = 1u;
            if (rawIndices.has()) {
                switch (type) { // 
                    case vk::IndexType::eUint32:   count = rawIndices->range() / (stride = 4u); break;
                    case vk::IndexType::eUint16:   count = rawIndices->range() / (stride = 2u); break;
                    case vk::IndexType::eUint8EXT: count = rawIndices->range() / (stride = 1u); break;
                    default: count = 0u;
                };
            };

            /* // 
            if (!this->indexData.has()) {
                this->descriptorSetHelper.pushDescription(vkh::VkDescriptorUpdateTemplateEntry{
                    .dstBinding = 1u,
                    .dstArrayElement = 0u,
                    .descriptorCount = 1u,
                    .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER
                }).offset<vk::BufferView>() = rawIndices.createBufferView(vk::Format::eR8Uint);
            };*/

            // 
            this->indexData = rawIndices;
            this->indexType = (rawIndices.has() && type != vk::IndexType::eNoneKHR) ? type : vk::IndexType::eNoneKHR;

            // 
            if (this->indexType != vk::IndexType::eNoneKHR) {
                this->currentUnitCount = rawIndices.range() / stride;
            };

            return uTHIS;
        };

        // 
        virtual uPTR(MeshInput) setIndexData(const vkt::Vector<uint32_t>& rawIndices) { return this->setIndexData(rawIndices, vk::IndexType::eUint32); };
        virtual uPTR(MeshInput) setIndexData(const vkt::Vector<uint16_t>& rawIndices) { return this->setIndexData(rawIndices, vk::IndexType::eUint16); };
        virtual uPTR(MeshInput) setIndexData(const vkt::Vector<uint8_t >& rawIndices) { return this->setIndexData(rawIndices, vk::IndexType::eUint8EXT); };
        virtual uPTR(MeshInput) setIndexData() { return this->setIndexData({}, vk::IndexType::eNoneKHR); };

        // some type dependent
        //template<class T = uint8_t>
        //inline uPTR(MeshInput) setIndexData(const vkt::Vector<T>& rawIndices = {}) { return this->setIndexData(rawIndices); };

        // 
        virtual uPTR(MeshInput) createRasterizePipeline() {
            const auto& viewport = this->context->refViewport();
            const auto& renderArea = this->context->refScissor();
            const auto& TFI = vk::PipelineRasterizationStateStreamCreateInfoEXT().setRasterizationStream(0u);

            // 
            this->transformSetLayoutHelper.pushBinding(vkh::VkDescriptorSetLayoutBinding{ .binding = 0u, .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, .descriptorCount = 8u, .stageFlags = {.eVertex = 1, .eGeometry = 1, .eFragment = 1, .eCompute = 1, .eRaygen = 1, .eClosestHit = 1 } }, vkh::VkDescriptorBindingFlags{ .ePartiallyBound = 1 });
            this->transformSetLayoutHelper.pushBinding(vkh::VkDescriptorSetLayoutBinding{ .binding = 1u, .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, .descriptorCount = 1u, .stageFlags = {.eVertex = 1, .eGeometry = 1, .eFragment = 1, .eCompute = 1, .eRaygen = 1, .eClosestHit = 1 } }, vkh::VkDescriptorBindingFlags{ .ePartiallyBound = 1 });
            this->transformSetLayoutHelper.pushBinding(vkh::VkDescriptorSetLayoutBinding{ .binding = 2u, .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, .descriptorCount = 1u, .stageFlags = {.eVertex = 1, .eGeometry = 1, .eFragment = 1, .eCompute = 1, .eRaygen = 1, .eClosestHit = 1 } });
            this->transformSetLayoutHelper.pushBinding(vkh::VkDescriptorSetLayoutBinding{ .binding = 3u, .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, .descriptorCount = 1u, .stageFlags = {.eVertex = 1, .eGeometry = 1, .eFragment = 1, .eCompute = 1, .eRaygen = 1, .eClosestHit = 1 } });
            this->transformSetLayout[0] = driver->getDevice().createDescriptorSetLayout(transformSetLayoutHelper);

            // 
            std::vector<vkh::VkPushConstantRange> ranges = { {.stageFlags = {.eVertex = 1, .eGeometry = 1, .eFragment = 1, .eCompute = 1, .eRaygen = 1, .eClosestHit = 1, .eMiss = 1 }, .offset = 0u, .size = 16u } };
            this->transformPipelineLayout = driver->getDevice().createPipelineLayout(vkh::VkPipelineLayoutCreateInfo{}.setSetLayouts(transformSetLayout).setPushConstantRanges(ranges));

            // 
            this->pipelineInfo.rasterizationState.pNext = &TFI;
            this->pipelineInfo.rasterizationState.rasterizerDiscardEnable = true;

            // 
            this->pipelineInfo = vkh::VsGraphicsPipelineCreateInfoConstruction();
            this->pipelineInfo.vertexInputAttributeDescriptions = this->vertexInputAttributeDescriptions;
            this->pipelineInfo.vertexInputBindingDescriptions = this->vertexInputBindingDescriptions;
            this->pipelineInfo.stages = this->stages;
            //this->pipelineInfo.depthStencilState = vkh::VkPipelineDepthStencilStateCreateInfo{ .depthTestEnable = true, .depthWriteEnable = true };
            this->pipelineInfo.dynamicStates.push_back(VK_DYNAMIC_STATE_VIEWPORT);
            this->pipelineInfo.dynamicStates.push_back(VK_DYNAMIC_STATE_SCISSOR);
            this->pipelineInfo.graphicsPipelineCreateInfo.renderPass = this->context->renderPass;
            this->pipelineInfo.graphicsPipelineCreateInfo.layout = this->transformPipelineLayout;

            // 
            for (uint32_t i = 0u; i < 8u; i++) {
                this->pipelineInfo.colorBlendAttachmentStates.push_back(vkh::VkPipelineColorBlendAttachmentState{ .blendEnable = true }); // transparency will generated by ray-tracing
            };

            // 
            this->transformState = driver->getDevice().createGraphicsPipeline(driver->getPipelineCache(), this->pipelineInfo);

            // 
            return uTHIS;
        };

        // 
        virtual uPTR(MeshInput) linkCounterBuffer(const vkt::Vector<vkh::VkAccelerationStructureBuildOffsetInfoKHR>& offsetInfo = {}) {
            //this->counterData = offsetInfo; // DANGER!! BSOD BY LOCKING GPU!
            return uTHIS;
        };

    protected: friend Node; friend Renderer; // Partitions
        uint64_t checkoutPointData = 0ull;
        MeshInfo meta = {};

        // 
        vk::PipelineLayout transformPipelineLayout = {};
        std::vector<vk::DescriptorSet> descriptorSet = { {} };
        std::vector<VkDescriptorSetLayout> transformSetLayout = { {} };
        vkh::VsDescriptorSetLayoutCreateInfoHelper transformSetLayoutHelper = {};
        vkh::VsDescriptorSetCreateInfoHelper descriptorSetHelper = {};

        // 
        std::vector<vkh::VkVertexInputBindingDescription> vertexInputBindingDescriptions = {};
        vkt::Vector<vkh::VkVertexInputBindingDescription> rawBindings = {};
        vkt::Vector<vkh::VkVertexInputBindingDescription> gpuBindings = {};

        // 
        std::vector<vkh::VkVertexInputAttributeDescription> vertexInputAttributeDescriptions = {};
        vkt::Vector<vkh::VkVertexInputAttributeDescription> rawAttributes = {};
        vkt::Vector<vkh::VkVertexInputAttributeDescription> gpuAttributes = {};

        // 
        std::vector<vkt::Vector<uint8_t>> bindings = {};
        std::vector<uint32_t> bindRange = { 0 };
        vkt::Vector<MeshInfo> rawMeshInfo = { }; // BROKEN?!
        vk::IndexType indexType = vk::IndexType::eNoneKHR;

        // 
        vkt::Vector<uint8_t> indexData = {};
        vkt::Vector<vkh::VkAccelerationStructureBuildOffsetInfoKHR> counterData = {};
        uint32_t lastBindID = 0u;
        //size_t primitiveCount = 0u;

        // 
        vk::DeviceSize currentUnitCount = 0u;
        bool needsQuads = false, needUpdate = true, ignoreIndirection = false;

        // 
        std::vector<vkh::VkPipelineShaderStageCreateInfo> stages = {};
        vkh::VsGraphicsPipelineCreateInfoConstruction pipelineInfo = {};

        // 
        vkh::VkComputePipelineCreateInfo quadInfo = {};
        vkh::VkPipelineShaderStageCreateInfo quadStage = {};
        vk::Pipeline quadGenerator = {}, transformState = {};

        // 
        vkt::uni_ptr<Driver> driver = {};
        vkt::uni_ptr<Thread> thread = {};
        vkt::uni_ptr<Context> context = {};
        //vkt::uni_ptr<Renderer> renderer = {};
    };
};
