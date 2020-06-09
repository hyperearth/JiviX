#pragma once // #

#include "./config.hpp"
#include "./driver.hpp"
#include "./thread.hpp"
#include "./context.hpp"
#include "./bview-set.hpp"

namespace jvi {

    // WARNING!!
    // FOR VULKAN API ONLY!!
    // PLANNED: OPENGL VERSION!!
    class MeshInput : public std::enable_shared_from_this<MeshInput> {
    public: friend Node; friend Renderer; friend MeshBinding;
        MeshInput() {};
        MeshInput(const vkt::uni_ptr<Context>& context) : context(context) { this->construct(); };
        MeshInput(const std::shared_ptr<Context>& context) : context(context) { this->construct(); };
        ~MeshInput() {};

        // 
        public: virtual vkt::uni_ptr<MeshInput> sharedPtr() { return shared_from_this(); };
        //virtual vkt::uni_ptr<MeshInput> sharedPtr() const { return std::shared_ptr<MeshInput>(shared_from_this()); };

        //
        public: virtual uPTR(MeshInput) makeQuad(const bool& quad = true) { this->needsQuads = quad; return uTHIS; };

        // 
        protected: virtual uPTR(MeshInput) construct() {
            this->driver = context->getDriver();
            this->thread = std::make_shared<Thread>(this->driver);

            // 
            this->pipelineInfo = vkh::VsGraphicsPipelineCreateInfoConstruction();

            // 
            this->quadStage = vkt::makePipelineStageInfo(this->driver->getDeviceDispatch(), vkt::readBinary(std::string("./shaders/rtrace/quad.comp.spv")), VK_SHADER_STAGE_COMPUTE_BIT);

            // for faster code, pre-initialize
            this->stages = {
                vkt::makePipelineStageInfo(this->driver->getDeviceDispatch(), vkt::readBinary(std::string("./shaders/rtrace/transform.vert.spv")), VK_SHADER_STAGE_VERTEX_BIT),
                vkt::makePipelineStageInfo(this->driver->getDeviceDispatch(), vkt::readBinary(std::string("./shaders/rtrace/transform.geom.spv")), VK_SHADER_STAGE_GEOMETRY_BIT)
            };

            auto hostUsage = vkh::VkBufferUsageFlags{.eTransferSrc = 1, .eUniformBuffer = 1, .eStorageBuffer = 1, .eRayTracing = 1 };
            auto gpuUsage = vkh::VkBufferUsageFlags{.eTransferDst = 1, .eUniformBuffer = 1, .eStorageBuffer = 1, .eRayTracing = 1 };

            // transformPipelineLayout
            // create required buffers
            this->rawBindings = vkt::Vector<vkh::VkVertexInputBindingDescription>(std::make_shared<vkt::VmaBufferAllocation>(this->driver->getAllocator(), vkh::VkBufferCreateInfo{ .size = sizeof(vkh::VkVertexInputBindingDescription) * 8u, .usage = hostUsage}, vkt::VmaMemoryInfo{ .memUsage = VMA_MEMORY_USAGE_CPU_TO_GPU }));
            this->gpuBindings = vkt::Vector<vkh::VkVertexInputBindingDescription>(std::make_shared<vkt::VmaBufferAllocation>(this->driver->getAllocator(), vkh::VkBufferCreateInfo{ .size = sizeof(vkh::VkVertexInputBindingDescription) * 8u, .usage = gpuUsage}, vkt::VmaMemoryInfo{ .memUsage = VMA_MEMORY_USAGE_GPU_ONLY }));
            this->rawAttributes = vkt::Vector<vkh::VkVertexInputAttributeDescription>(std::make_shared<vkt::VmaBufferAllocation>(this->driver->getAllocator(), vkh::VkBufferCreateInfo{ .size = sizeof(vkh::VkVertexInputAttributeDescription) * 8u, .usage = hostUsage }, vkt::VmaMemoryInfo{ .memUsage = VMA_MEMORY_USAGE_CPU_TO_GPU }));
            this->gpuAttributes = vkt::Vector<vkh::VkVertexInputAttributeDescription>(std::make_shared<vkt::VmaBufferAllocation>(this->driver->getAllocator(), vkh::VkBufferCreateInfo{ .size = sizeof(vkh::VkVertexInputAttributeDescription) * 8u, .usage = gpuUsage }, vkt::VmaMemoryInfo{ .memUsage = VMA_MEMORY_USAGE_GPU_ONLY }));

            // 
            return uTHIS;
        };

        // 
        protected: virtual uPTR(MeshInput) createDescriptorSet() { // 
            if (this->descriptorSetInitialized) {
                return uTHIS; // TODO: Optional Un-Protect
            };

            // 
            this->descriptorSetHelper = vkh::VsDescriptorSetCreateInfoHelper(this->transformSetLayout[0], this->driver->getDescriptorPool());

            // 
            this->descriptorSetHelper.pushDescription(vkh::VkDescriptorUpdateTemplateEntry{
                .dstBinding = 0u,
                .dstArrayElement = 0u,
                .descriptorCount = 1u,
                .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER
            }).offset<vkh::VkDescriptorBufferInfo>() = gpuBindings;

            // 
            this->descriptorSetHelper.pushDescription(vkh::VkDescriptorUpdateTemplateEntry{
                .dstBinding = 1u,
                .dstArrayElement = 0u,
                .descriptorCount = 1u,
                .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER
            }).offset<vkh::VkDescriptorBufferInfo>() = gpuAttributes;

            // 
            vkh::handleVk(vkt::AllocateDescriptorSetWithUpdate(this->driver->getDeviceDispatch(), this->descriptorSetHelper, this->descriptorSet[0], this->descriptorUpdated));

            // 
            if (this->bvs) { this->bvs->createDescriptorSet(); };
            this->descriptorSetInitialized = true;

            // 
            return uTHIS;
        };

        // 
        protected: virtual uPTR(MeshInput) copyMeta(VkCommandBuffer buildCommand = {}) {
            this->driver->getDeviceDispatch()->CmdCopyBuffer(buildCommand, this->rawAttributes, this->gpuAttributes, 1u, vkh::VkBufferCopy{ this->rawAttributes.offset(), this->gpuAttributes.offset(), this->gpuAttributes.range() });
            this->driver->getDeviceDispatch()->CmdCopyBuffer(buildCommand, this->rawBindings, this->gpuBindings, 1u, vkh::VkBufferCopy{ this->rawBindings.offset(), this->gpuBindings.offset(), this->gpuBindings.range() });
            return uTHIS;
        };

        /*// 
        virtual uPTR(MeshInput) formatQuads(const std::shared_ptr<jvi::MeshBinding>& binding, vkt::uni_arg<glm::u64vec4> offsetHelp, vkt::uni_arg<VkCommandBuffer> buildCommand = {}) {
            return this->formatQuads(binding, offsetHelp, buildCommand);
        };

        //
        virtual uPTR(MeshInput) formatQuads(const vkt::uni_ptr<jvi::MeshBinding>& binding, vkt::uni_arg<glm::u64vec4> offsetHelp, vkt::uni_arg<VkCommandBuffer> buildCommand = {}) {
            return this->formatQuads(binding, offsetHelp, VkCommandBuffer(*buildCommand));
        };*/

        // 
        protected: virtual uPTR(MeshInput) formatQuads(const vkt::uni_ptr<jvi::MeshBinding>& binding, vkt::uni_arg<glm::u64vec4> offsetHelp, vkt::uni_arg<VkCommandBuffer> buildCommand = {}) { // 
            bool DirectCommand = false, HasCommand = buildCommand.has() && *buildCommand;

            // Initialize Input (Early)
            if (this->needsQuads) { this->createRasterizePipeline()->createDescriptorSet(); };

            // 
            if (!HasCommand || ignoreIndirection) {
                buildCommand = vkt::createCommandBuffer(this->driver->getDeviceDispatch(), this->thread->getCommandPool()); DirectCommand = true;
            };

            // 
            if (this->indexData) {
                this->meta.indexID = *this->indexData;
                this->meta.indexType = int32_t(this->indexType) + 1;
            };

            // 
            if (this->bvs) {
                this->descriptorSet.resize(2u);
                this->descriptorSet[1u] = this->bvs->getDescriptorSet();
            };

            // NO! Please, re-make QUAD internally! (P.S. MeshBinding)
            if (HasCommand && this->needsQuads) { // TODO: scratch buffer
                this->needsQuads = false; // FOR MINECRAFT ONLY! 
                this->quadInfo.layout = this->transformPipelineLayout;
                this->quadInfo.stage = this->quadStage;
                if (!this->quadGenerator) {
                    this->quadGenerator = vkt::createCompute(this->driver->getDeviceDispatch(), vkt::FixConstruction(this->quadStage), VkPipelineLayout(this->quadInfo.layout), this->driver->getPipelineCache());
                };

                // 
                const auto originalCt = this->currentUnitCount;
                const uint32_t ucount = vkt::tiled(uint64_t(originalCt), uint64_t(1024ull));
                auto pstage = vkh::VkShaderStageFlags{.eVertex = 1, .eGeometry = 1, .eFragment = 1, .eCompute = 1, .eRaygen = 1, .eClosestHit = 1, .eMiss = 1 };

                // 
                this->driver->getDeviceDispatch()->CmdBindPipeline(buildCommand, VK_PIPELINE_BIND_POINT_COMPUTE, this->quadGenerator);
                this->driver->getDeviceDispatch()->CmdBindDescriptorSets(buildCommand, VK_PIPELINE_BIND_POINT_COMPUTE, this->transformPipelineLayout, 0u, this->descriptorSet.size(), this->descriptorSet.data(), 0u, nullptr);
                this->driver->getDeviceDispatch()->CmdPushConstants(buildCommand, this->transformPipelineLayout, pstage, 0u, sizeof(glm::uvec4), &meta);
                this->driver->getDeviceDispatch()->CmdDispatch(buildCommand, ucount, 1u, 1u);
                vkt::commandBarrier(this->driver->getDeviceDispatch(), buildCommand);

                // Now should to be triangles!
                if (this->indexData) {
                    this->setIndexData(this->meta.indexID, this->indexType)->manifestIndex(VK_INDEX_TYPE_UINT32)->setIndexCount(vkt::tiled(uint64_t(originalCt), uint64_t(4ull)) * 6u);
                };
            };

            if (DirectCommand) {
                vkt::submitCmd(this->driver->getDeviceDispatch(), this->thread->getQueue(), { buildCommand });
                this->driver->getDeviceDispatch()->FreeCommandBuffers(this->thread->getCommandPool(), 1u, buildCommand);
            };

            return uTHIS;
        };

        /*
        // 
        virtual uPTR(MeshInput) buildGeometry(const std::shared_ptr<jvi::MeshBinding>& binding, vkt::uni_arg<glm::u64vec4> offsetHelp, vkt::uni_arg<VkCommandBuffer> buildCommand = {}) {
            return this->buildGeometry(binding, offsetHelp, buildCommand);
        };

        //
        virtual uPTR(MeshInput) buildGeometry(const vkt::uni_ptr<jvi::MeshBinding>& binding, vkt::uni_arg<glm::u64vec4> offsetHelp, vkt::uni_arg<VkCommandBuffer> buildCommand = {}){
            return this->buildGeometry(binding, offsetHelp, VkCommandBuffer(*buildCommand));
        };*/

        // Record Geometry (Transform Feedback), Re-Implemented in another file... 
        protected: virtual uPTR(MeshInput) buildGeometry(const vkt::uni_ptr<jvi::MeshBinding>& binding, vkt::uni_arg<glm::u64vec4> offsetHelp, vkt::uni_arg<VkCommandBuffer> buildCommand = {});

        // 
        public: template<class T = uint8_t>
        inline uPTR(MeshInput) addBinding(const uint32_t& rawData, vkt::uni_arg<vkh::VkVertexInputBindingDescription> binding = vkh::VkVertexInputBindingDescription{}) {
            const uintptr_t bindingID = this->vertexInputBindingDescriptions.size();
            this->vertexInputBindingDescriptions.resize(bindingID + 1u);
            this->vertexInputBindingDescriptions[bindingID] = binding;
            this->vertexInputBindingDescriptions[bindingID].binding = static_cast<uint32_t>(bindingID);
            this->rawBindings[bindingID] = this->vertexInputBindingDescriptions[bindingID];
            this->rawBindings[bindingID].binding = rawData;

            // 
            this->bindRange.resize(bindingID + 1u);
            this->bindRange[this->lastBindID = static_cast<uint32_t>(bindingID)] = this->bvs->get(rawData).range();
            this->bindings.resize(bindingID+1u);
            this->bindings[bindingID] = rawData;
            return uTHIS;
        };

        // 
        public: virtual uPTR(MeshInput) manifestIndex(const VkIndexType& type = VK_INDEX_TYPE_NONE_KHR) {
            if (this->rawMeshInfo.has()) {
                this->rawMeshInfo[0u].indexType = uint32_t(this->indexType = type) + 1u;
            } else {
                this->indexType = type;
            }
            return uTHIS;
        };

        // 
        public: virtual uPTR(MeshInput) addAttribute(vkt::uni_arg<vkh::VkVertexInputAttributeDescription> attribute = vkh::VkVertexInputAttributeDescription{}, const bool& NotStub = true) {
            const uintptr_t bindingID = this->lastBindID;//attribute->binding;
            const uintptr_t locationID = attribute->location;
            this->vertexInputAttributeDescriptions.resize(locationID + 1u);
            this->vertexInputAttributeDescriptions[locationID] = attribute;
            this->vertexInputAttributeDescriptions[locationID].binding = static_cast<uint32_t>(bindingID);
            this->vertexInputAttributeDescriptions[locationID].location = static_cast<uint32_t>(locationID);
            this->rawAttributes[locationID] = this->vertexInputAttributeDescriptions[locationID];

            // 
            if (locationID == 0u && NotStub && this->indexType == VK_INDEX_TYPE_NONE_KHR) {
                this->currentUnitCount = this->bindRange[bindingID] / this->bvs->get(this->bindings[bindingID]).stride();
            };

            // 
            if (rawMeshInfo.has()) {
                if (locationID == 1u && NotStub) { rawMeshInfo[0].hasTexcoord = meta.hasTexcoord = 1; };
                if (locationID == 2u && NotStub) { rawMeshInfo[0].hasNormal = meta.hasNormal = 1; };
                if (locationID == 3u && NotStub) { rawMeshInfo[0].hasTangent = meta.hasTangent = 1; };
                if (locationID == 0u && NotStub && this->needsQuads) { // PIDORS IN MICROSOFT!
                    this->rawMeshInfo[0u].indexType = uint32_t(this->indexType = VK_INDEX_TYPE_UINT32) + 1u;
                };
            } else {
                if (locationID == 1u && NotStub) { meta.hasTexcoord = 1; };
                if (locationID == 2u && NotStub) { meta.hasNormal = 1; };
                if (locationID == 3u && NotStub) { meta.hasTangent = 1; };
            };

            // 
            return uTHIS;
        };

        // 
        public: virtual uPTR(MeshInput) setIndexOffset(const VkDeviceSize& offset = 0ull) { this->indexOffset = offset; return uTHIS; };
        public: virtual uPTR(MeshInput) setIndexCount(const VkDeviceSize& count = 65536u * 3u) { this->currentUnitCount = count; return uTHIS; };
        public: virtual uPTR(MeshInput) setPrimitiveCount(const VkDeviceSize& count = 65536u) { this->setIndexCount(count * 3u); return uTHIS; };

        // 
        public: virtual const VkDeviceSize& getIndexCount() const { return this->currentUnitCount; };
        public: virtual VkDeviceSize& getIndexCount() { return this->currentUnitCount; };

        // 
        public: template<class T = uint8_t>
        inline uPTR(MeshInput) setIndexData(const uint32_t& rawIndices, const VkIndexType& type) {
        //inline uPTR(MeshInput) setIndexData(const vkt::Vector<T>& rawIndices, const VkIndexType& type) {
            VkDeviceSize count = 0u; uint32_t stride = 1u;
            //if (rawIndices) {
                switch (type) { // 
                    case VK_INDEX_TYPE_UINT32   : count = this->bvs->get(rawIndices)->range() / (stride = 4u); break;
                    case VK_INDEX_TYPE_UINT16   : count = this->bvs->get(rawIndices)->range() / (stride = 2u); break;
                    case VK_INDEX_TYPE_UINT8_EXT: count = this->bvs->get(rawIndices)->range() / (stride = 1u); break;
                    default: count = 0u;
                };
            //};
            // 
            this->indexData = rawIndices; // 
            this->indexType = (this->indexData && type != VK_INDEX_TYPE_NONE_KHR) ? type : VK_INDEX_TYPE_NONE_KHR;

            // 
            if (this->indexType != VK_INDEX_TYPE_NONE_KHR) {
                this->currentUnitCount = this->bvs->get(rawIndices).range() / stride;
            };

            return uTHIS;
        };

        // 
        //virtual uPTR(MeshInput) setIndexData(const vkt::Vector<uint32_t>& rawIndices) { return this->setIndexData(rawIndices, VkIndexType::eUint32); };
        //virtual uPTR(MeshInput) setIndexData(const vkt::Vector<uint16_t>& rawIndices) { return this->setIndexData(rawIndices, VkIndexType::eUint16); };
        //virtual uPTR(MeshInput) setIndexData(const vkt::Vector<uint8_t >& rawIndices) { return this->setIndexData(rawIndices, VkIndexType::eUint8EXT); };
        public: virtual uPTR(MeshInput) setIndexData() { return this->setIndexData({}, VK_INDEX_TYPE_NONE_KHR); };

        // some type dependent
        //template<class T = uint8_t>
        //inline uPTR(MeshInput) setIndexData(const vkt::Vector<T>& rawIndices = {}) { return this->setIndexData(rawIndices); };

        // 
        public: virtual uPTR(MeshInput) createRasterizePipeline() {
            if (this->transformState) {
                return uTHIS; // TODO: Optional Un-Protect
            };

            const auto& viewport = this->context->refViewport();
            const auto& renderArea = this->context->refScissor();
            const auto& TFI = vkh::VkPipelineRasterizationStateStreamCreateInfoEXT{};

            //
            auto stage = vkh::VkShaderStageFlags{.eVertex = 1, .eGeometry = 1, .eFragment = 1, .eCompute = 1, .eRaygen = 1, .eClosestHit = 1, .eMiss = 1 };
            this->transformSetLayoutHelper.pushBinding(vkh::VkDescriptorSetLayoutBinding{ .binding = 0u, .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, .descriptorCount = 1u, .stageFlags = stage });
            this->transformSetLayoutHelper.pushBinding(vkh::VkDescriptorSetLayoutBinding{ .binding = 1u, .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, .descriptorCount = 1u, .stageFlags = stage });
            vkh::handleVk(this->driver->getDeviceDispatch()->CreateDescriptorSetLayout(transformSetLayoutHelper, nullptr, &this->transformSetLayout[0]));

            // 
            if (this->bvs) {
                this->transformSetLayout.resize(2u);
                this->transformSetLayout[1] = this->bvs->getDescriptorLayout();
            };

            // 
            std::vector<vkh::VkPushConstantRange> ranges = { vkh::VkPushConstantRange{.stageFlags = stage, .offset = 0u, .size = 16u } };
            vkh::handleVk(this->driver->getDeviceDispatch()->CreatePipelineLayout(vkh::VkPipelineLayoutCreateInfo{}.setSetLayouts(transformSetLayout).setPushConstantRanges(ranges), nullptr, &this->transformPipelineLayout));

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
            vkh::handleVk(this->driver->getDeviceDispatch()->CreateGraphicsPipelines(driver->getPipelineCache(), 1u, this->pipelineInfo, nullptr, &this->transformState));

            // 
            return uTHIS;
        };

        // 
        protected: virtual uPTR(MeshInput) linkCounterBuffer(const vkt::Vector<vkh::VkAccelerationStructureBuildOffsetInfoKHR>& offsetInfo = {}) {
            //this->counterData = offsetInfo; // DANGER!! BSOD BY LOCKING GPU!
            return uTHIS;
        };

        // 
        public: virtual uPTR(MeshInput) linkBViewSet(const std::shared_ptr<BufferViewSet>& bufferViewSet = {}) {
            return this->linkBViewSet(vkt::uni_ptr<BufferViewSet>(bufferViewSet));
        };

        // 
        public: virtual uPTR(MeshInput) linkBViewSet(const vkt::uni_ptr<BufferViewSet>& bufferViewSet = {}) {
            this->bvs = bufferViewSet;
            return uTHIS;
        };

    protected: friend Node; friend Renderer; // Partitions
        uint64_t checkoutPointData = 0ull;
        MeshInfo meta = {};

        // 
        VkPipelineLayout transformPipelineLayout = {};
        std::vector<VkDescriptorSet> descriptorSet = { {}, {} };
        std::vector<VkDescriptorSetLayout> transformSetLayout = { {}, {} };
        vkh::VsDescriptorSetLayoutCreateInfoHelper transformSetLayoutHelper = {};
        vkh::VsDescriptorSetCreateInfoHelper descriptorSetHelper = {};

        // 
        std::vector<vkh::VkVertexInputBindingDescription> vertexInputBindingDescriptions = {};
        vkt::Vector<vkh::VkVertexInputBindingDescription> rawBindings = {}, gpuBindings = {};

        // 
        std::vector<vkh::VkVertexInputAttributeDescription> vertexInputAttributeDescriptions = {};
        vkt::Vector<vkh::VkVertexInputAttributeDescription> rawAttributes = {}, gpuAttributes = {};

        // 
        std::vector<uint32_t> bindings = {}, bindRange = { 0 };
        vkt::Vector<MeshInfo> rawMeshInfo = { }; // BROKEN?!
        VkIndexType indexType = VK_INDEX_TYPE_NONE_KHR;

        // 
        VkDeviceSize indexOffset = 0ull;
        std::optional<uint32_t> indexData;

        // 
        vkt::uni_ptr<BufferViewSet> bvs = {};
        uint32_t lastBindID = 0u;

        // 
        VkDeviceSize currentUnitCount = 0u;
        bool needsQuads = false, needUpdate = true, ignoreIndirection = false;
        bool descriptorSetInitialized = false;
        bool descriptorUpdated = false;

        // 
        std::vector<vkh::VkPipelineShaderStageCreateInfo> stages = {};
        vkh::VsGraphicsPipelineCreateInfoConstruction pipelineInfo = {};
        //vkh::VkAccelerationStructureBuildOffsetInfoKHR offsetMeta = {};

        // 
        vkh::VkComputePipelineCreateInfo quadInfo = {};
        vkh::VkPipelineShaderStageCreateInfo quadStage = {};
        VkPipeline quadGenerator = {}, transformState = {};

        // 
        vkt::uni_ptr<Driver> driver = {};
        vkt::uni_ptr<Thread> thread = {};
        vkt::uni_ptr<Context> context = {};
        //vkt::uni_ptr<Renderer> renderer = {};
    };
};
