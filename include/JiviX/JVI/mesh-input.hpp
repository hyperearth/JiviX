#pragma once // #

#include "./config.hpp"
#include "./driver.hpp"
#include "./thread.hpp"
#include "./context.hpp"
#include "./mesh.hpp"

namespace jvi {

    // TODO: Make Mesh Shaded and Geometry Complex
    class MeshInput : public std::enable_shared_from_this<MeshInput> {
    public: friend Node; friend Renderer;
        MeshInput() {};
        MeshInput(const vkt::uni_ptr<Context> & context, vk::DeviceSize AllocationUnitCount = 32768, vk::DeviceSize MaxStride = sizeof(glm::vec4)) : context(context) { this->construct(); };
        ~MeshInput() {};

        // 
        virtual uPTR(MeshInput) construct() {
            this->quadStage = vkt::makePipelineStageInfo(this->driver->getDevice(), vkt::readBinary("./shaders/rtrace/quad.comp.spv"), vk::ShaderStageFlagBits::eCompute);
            this->counterData = vkt::Vector<uint32_t>(this->driver->getAllocator(), vkh::VkBufferCreateInfo{ .size = 4u, .usage = {.eTransferDst = 1, .eUniformBuffer = 1, .eStorageBuffer = 1, .eRayTracing = 1, .eTransformFeedbackCounterBuffer = 1 } }, VMA_MEMORY_USAGE_GPU_ONLY);

            // for faster code, pre-initialize
            this->stages = vkt::vector_cast<vkh::VkPipelineShaderStageCreateInfo, vk::PipelineShaderStageCreateInfo>({
                vkt::makePipelineStageInfo(this->driver->getDevice(), vkt::readBinary("./shaders/rtrace/transform.vert.spv"), vk::ShaderStageFlagBits::eVertex),
                vkt::makePipelineStageInfo(this->driver->getDevice(), vkt::readBinary("./shaders/rtrace/transform.geom.spv"), vk::ShaderStageFlagBits::eGeometry)
            });
        };

        // Record Geometry (Transform Feedback)
        virtual uPTR(MeshInput) buildGeometry(const vkt::uni_arg<vkt::Vector<uint8_t>>& OutPut, const vk::CommandBuffer& buildCommand = {}, const glm::uvec4& meshData = glm::uvec4(0u)) { // 
            if (this->needsQuads && buildCommand) { // FOR MINECRAFT ONLY! 
                this->quadInfo.layout = this->context->unifiedPipelineLayout;
                this->quadInfo.stage = this->quadStage;
                this->quadGenerator = vkt::createCompute(driver->getDevice(), vkt::FixConstruction(this->quadStage), vk::PipelineLayout(this->quadInfo.layout), driver->getPipelineCache());

                // 
                buildCommand.bindDescriptorSets(vk::PipelineBindPoint::eCompute, this->context->unifiedPipelineLayout, 0ull, this->context->descriptorSets, {});
                buildCommand.bindPipeline(vk::PipelineBindPoint::eCompute, this->quadGenerator);
                buildCommand.pushConstants<glm::uvec4>(this->context->unifiedPipelineLayout, vkh::VkShaderStageFlags{.eVertex = 1, .eGeometry = 1, .eFragment = 1, .eCompute = 1, .eRaygen = 1, .eClosestHit = 1, .eMiss = 1 }.hpp(), 0u, { meshData });
                buildCommand.dispatch(vkt::tiled(uint32_t(this->primitiveCount), 256u), 1u, 1u);
                this->needsQuads = false;
            } else 

            // USE Advanced Transform Feedback!
            if (buildCommand) { // 
                std::vector<vk::Buffer> buffers = {}; std::vector<vk::DeviceSize> offsets = {};
                buffers.resize(this->bindings.size()); offsets.resize(this->bindings.size()); uintptr_t I = 0u;
                for (auto& B : this->bindings) { if (B.has()) { const uintptr_t i = I++; buffers[i] = B.buffer(); offsets[i] = B.offset(); }; };

                // covergence
                buildCommand.fillBuffer(counterData.buffer(), counterData.offset(), counterData.range(), 0u); // Nullify Counters
                buildCommand.beginTransformFeedbackEXT(0u, { counterData.buffer() }, { counterData.offset() }); //!!WARNING!!
                buildCommand.bindTransformFeedbackBuffersEXT(0u, { OutPut->buffer() }, { OutPut->offset() }, { OutPut->range() }); //!!WARNING!!
                buildCommand.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, this->context->unifiedPipelineLayout, 0ull, this->context->descriptorSets, {});
                buildCommand.bindPipeline(vk::PipelineBindPoint::eGraphics, this->transformState);
                buildCommand.bindVertexBuffers(0u, buffers, offsets);
                buildCommand.pushConstants<glm::uvec4>(this->context->unifiedPipelineLayout, vkh::VkShaderStageFlags{.eVertex = 1, .eGeometry = 1, .eFragment = 1, .eCompute = 1, .eRaygen = 1, .eClosestHit = 1, .eMiss = 1 }.hpp(), 0u, { meshData });
                if (this->indexType != vk::IndexType::eNoneKHR) { // PLC Mode
                    const uintptr_t voffset = this->bindings[this->vertexInputAttributeDescriptions[0u].binding].offset(); // !!WARNING!!
                    buildCommand.bindIndexBuffer(this->indexData, this->indexData.offset(), this->indexType);
                    buildCommand.drawIndexed(this->currentUnitCount, 1u, 0u, voffset, 0u);
                } else { // VAL Mode
                    buildCommand.draw(this->currentUnitCount, 1u, 0u, 0u);
                };
                buildCommand.endTransformFeedbackEXT(0u, { counterData.buffer() }, { counterData.offset() }); //!!WARNING!!
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
            this->bindRange[this->lastBindID = static_cast<uint32_t>(bindingID)] = rawData.range();
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
            return uTHIS;
        };

        // 
        inline uPTR(MeshInput) setIndexData(const vkt::uni_arg<vkt::Vector<>>& rawIndices, const vk::IndexType& type = vk::IndexType::eNoneKHR) {
            vk::DeviceSize count = 0u; uint32_t stride = 1u;
            if (rawIndices.has()) {
                switch (type) { // 
                    case vk::IndexType::eUint32:   count = rawIndices->range / (stride = 4u); break;
                    case vk::IndexType::eUint16:   count = rawIndices->range / (stride = 2u); break;
                    case vk::IndexType::eUint8EXT: count = rawIndices->range / (stride = 1u); break;
                    default: count = 0u;
                };
            };

            // 
            this->indexData = rawIndices;
            this->indexType = (rawIndices.has() && type != vk::IndexType::eNoneKHR) ? type : vk::IndexType::eNoneKHR;
            return uTHIS;
        };

        // 
        virtual uPTR(MeshInput) setIndexData(const vkt::uni_arg<vkt::Vector<uint32_t>>& rawIndices) { return this->setIndexData(vkt::Vector<>(*rawIndices), vk::IndexType::eUint32); };
        virtual uPTR(MeshInput) setIndexData(const vkt::uni_arg<vkt::Vector<uint16_t>>& rawIndices) { return this->setIndexData(vkt::Vector<>(*rawIndices), vk::IndexType::eUint16); };
        virtual uPTR(MeshInput) setIndexData(const vkt::uni_arg<vkt::Vector<uint8_t >>& rawIndices) { return this->setIndexData(vkt::Vector<>(*rawIndices), vk::IndexType::eUint8EXT); };
        virtual uPTR(MeshInput) setIndexData() { return this->setIndexData({}, vk::IndexType::eNoneKHR); };

        // some type dependent
        template<class T = uint8_t>
        inline uPTR(MeshInput) setIndexData(const vkt::uni_arg<vkt::Vector<T>>& rawIndices = {}) { return this->setIndexData(rawIndices); };

        // 
        virtual uPTR(MeshInput) createRasterizePipeline() {
            const auto& viewport = this->context->refViewport();
            const auto& renderArea = this->context->refScissor();
            const auto& TFI = vk::PipelineRasterizationStateStreamCreateInfoEXT().setRasterizationStream(0u);

            // 
            this->pipelineInfo.rasterizationState.pNext = &TFI;
            this->pipelineInfo.rasterizationState.rasterizerDiscardEnable = true;

            // 
            this->pipelineInfo = vkh::VsGraphicsPipelineCreateInfoConstruction();
            this->pipelineInfo.vertexInputAttributeDescriptions = this->vertexInputAttributeDescriptions;
            this->pipelineInfo.vertexInputBindingDescriptions = this->vertexInputBindingDescriptions;
            this->pipelineInfo.stages = this->stages;
            //this->pipelineInfo.depthStencilState = vkh::VkPipelineDepthStencilStateCreateInfo{ .depthTestEnable = true, .depthWriteEnable = true };
            //this->pipelineInfo.dynamicStates.push_back(VK_DYNAMIC_STATE_VIEWPORT);
            //this->pipelineInfo.dynamicStates.push_back(VK_DYNAMIC_STATE_SCISSOR);
            //this->pipelineInfo.graphicsPipelineCreateInfo.renderPass = this->context->renderPass;
            this->pipelineInfo.graphicsPipelineCreateInfo.layout = this->context->unifiedPipelineLayout;

            // 
            this->transformState = driver->getDevice().createGraphicsPipeline(driver->getPipelineCache(), this->pipelineInfo);

            // 
            return uTHIS;
        };



    protected: friend Node; friend Renderer; // Partitions
        std::vector<vkh::VkVertexInputBindingDescription> vertexInputBindingDescriptions = {};
        std::vector<vkh::VkVertexInputAttributeDescription> vertexInputAttributeDescriptions = {};

        // 
        std::array<vkt::Vector<uint8_t>, 8> bindings = {};
//#ifdef ENABLE_OPENGL_INTEROP
//        std::array<std::pair<GLuint, GLuint>, 8> bindingsOGL = {};
//#endif
        std::array<uint32_t, 8> bindRange = { 0 };

        // 
        vkt::Vector<uint8_t> indexData = {};
        vkt::Vector<uint32_t> counterData = {};
        uint32_t lastBindID = 0u;
        size_t primitiveCount = 0u;

        // 
        vk::DeviceSize currentUnitCount = 0u;
        vk::IndexType indexType = vk::IndexType::eNoneKHR;
        bool needsQuads = false;

        // 
        std::vector<vkh::VkPipelineShaderStageCreateInfo> stages = {};
        vkh::VsGraphicsPipelineCreateInfoConstruction pipelineInfo = {};

        // 
        vkh::VkComputePipelineCreateInfo quadInfo = {};
        vkh::VkPipelineShaderStageCreateInfo quadStage = {};
        vk::Pipeline quadGenerator = {};
        vk::Pipeline transformState = {};

        // 
        vkt::uni_ptr<Driver> driver = {};
        vkt::uni_ptr<Thread> thread = {};
        vkt::uni_ptr<Context> context = {};
        //vkt::uni_ptr<Renderer> renderer = {};
    };
};
