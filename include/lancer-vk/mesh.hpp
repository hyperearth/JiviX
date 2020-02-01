#pragma once // #
#include "./config.hpp"
#include "./driver.hpp"
#include "./thread.hpp"
#include "./context.hpp"

namespace lancer {

    struct MeshInfo {
        uint32_t materialID = 0u;
        uint32_t indexType = 0u;
        uint32_t prmCount = 0u;
        //uint32_t flags = 0u;
        uint32_t
            hasTransform : 1,
            hasNormal : 1,
            hasTexcoord : 1;
    };

    // WIP Mesh Object
    // Sub-Instances Can Be Supported
    // TODO: Descriptor Sets
    class Mesh : public std::enable_shared_from_this<Mesh> { public: friend Node; friend Renderer;
        Mesh(const std::shared_ptr<Context>& context) {
            this->driver = context->getDriver();
            this->thread = std::make_shared<Thread>(this->driver);
            this->context = context;

            // 
            this->accelerationStructureInfo.instanceCount = 0u;
            this->accelerationStructureInfo.geometryCount = 1u;//this->instanceCount;
            this->accelerationStructureInfo.pGeometries = &this->geometryTemplate;
            this->accelerationStructureInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_NV;
            this->pipelineInfo = vkh::VsGraphicsPipelineCreateInfoConstruction();

            // create required buffers
            this->rawBindings = vkt::Vector<VkVertexInputBindingDescription>(std::make_shared<vkt::VmaBufferAllocation>(this->driver->getAllocator(), vkh::VkBufferCreateInfo{ .size = sizeof(VkVertexInputBindingDescription)*8u, .usage = { .eTransferSrc = 1, .eUniformBuffer = 1, .eStorageBuffer = 1, .eRayTracing = 1 } }, VMA_MEMORY_USAGE_CPU_TO_GPU));
            this->gpuBindings = vkt::Vector<VkVertexInputBindingDescription>(std::make_shared<vkt::VmaBufferAllocation>(this->driver->getAllocator(), vkh::VkBufferCreateInfo{ .size = sizeof(VkVertexInputBindingDescription)*8u, .usage = { .eTransferDst = 1, .eUniformBuffer = 1, .eStorageBuffer = 1, .eRayTracing = 1 } }, VMA_MEMORY_USAGE_GPU_ONLY));
            this->rawAttributes = vkt::Vector<VkVertexInputAttributeDescription>(std::make_shared<vkt::VmaBufferAllocation>(this->driver->getAllocator(), vkh::VkBufferCreateInfo{ .size = sizeof(VkVertexInputAttributeDescription)*8u, .usage = { .eTransferSrc = 1, .eUniformBuffer = 1, .eStorageBuffer = 1, .eRayTracing = 1 }}, VMA_MEMORY_USAGE_CPU_TO_GPU));
            this->gpuAttributes = vkt::Vector<VkVertexInputAttributeDescription>(std::make_shared<vkt::VmaBufferAllocation>(this->driver->getAllocator(), vkh::VkBufferCreateInfo{ .size = sizeof(VkVertexInputAttributeDescription)*8u, .usage = { .eTransferDst = 1, .eUniformBuffer = 1, .eStorageBuffer = 1, .eRayTracing = 1 }}, VMA_MEMORY_USAGE_GPU_ONLY));

            // 
            //this->gpuMeshInfo = vkt::Vector<MeshInfo>(std::make_shared<vkt::VmaBufferAllocation>(this->driver->getAllocator(), vkh::VkBufferCreateInfo{ .size = 16u, .usage = {.eTransferDst = 1, .eUniformBuffer = 1, .eStorageBuffer = 1, .eRayTracing = 1 } }, VMA_MEMORY_USAGE_GPU_ONLY));
              this->rawMeshInfo = vkt::Vector<MeshInfo>(std::make_shared<vkt::VmaBufferAllocation>(this->driver->getAllocator(), vkh::VkBufferCreateInfo{ .size = 16u, .usage = {.eTransferSrc = 1, .eUniformBuffer = 1, .eStorageBuffer = 1, .eRayTracing = 1 } }, VMA_MEMORY_USAGE_CPU_TO_GPU));

            // Internal Instance Map Per Global Node
            this->rawInstanceMap = vkt::Vector<uint32_t>(std::make_shared<vkt::VmaBufferAllocation>(this->driver->getAllocator(), vkh::VkBufferCreateInfo{ .size = 4u * 64u, .usage = {.eTransferSrc = 1, .eUniformBuffer = 1, .eStorageBuffer = 1, .eRayTracing = 1 } }, VMA_MEMORY_USAGE_CPU_TO_GPU));
            this->gpuInstanceMap = vkt::Vector<uint32_t>(std::make_shared<vkt::VmaBufferAllocation>(this->driver->getAllocator(), vkh::VkBufferCreateInfo{ .size = 4u * 64u, .usage = {.eTransferDst = 1, .eUniformBuffer = 1, .eStorageBuffer = 1, .eRayTracing = 1 } }, VMA_MEMORY_USAGE_GPU_ONLY));

            // for faster code, pre-initialize
            this->stages = vkt::vector_cast<vkh::VkPipelineShaderStageCreateInfo, vk::PipelineShaderStageCreateInfo>({
                vkt::makePipelineStageInfo(this->driver->getDevice(), vkt::readBinary("./shaders/rtrace/rasterize.vert.spv"), vk::ShaderStageFlagBits::eVertex),
                vkt::makePipelineStageInfo(this->driver->getDevice(), vkt::readBinary("./shaders/rtrace/rasterize.geom.spv"), vk::ShaderStageFlagBits::eGeometry),
                vkt::makePipelineStageInfo(this->driver->getDevice(), vkt::readBinary("./shaders/rtrace/rasterize.frag.spv"), vk::ShaderStageFlagBits::eFragment)
            });
        };

        // 
        std::shared_ptr<Mesh> setMaterialID(const uint32_t& materialID = 0u) {
            this->rawMeshInfo[0u].materialID = materialID;
            return shared_from_this();
        };

        // 
        std::shared_ptr<Mesh> setThread(const std::shared_ptr<Thread>& thread) {
            this->thread = thread;
            return shared_from_this();
        };

        // 
        std::shared_ptr<Mesh> addBinding(const vkt::Vector<uint8_t>& vector, const vkh::VkVertexInputBindingDescription& binding = {}) {
            //const uintptr_t bindingID = binding.binding;
            const uintptr_t bindingID = this->vertexInputBindingDescriptions.size();
            this->vertexInputBindingDescriptions.resize(bindingID+1u);
            this->vertexInputBindingDescriptions[bindingID] = binding;
            this->vertexInputBindingDescriptions[bindingID].binding = bindingID;
            this->rawBindings[bindingID] = this->vertexInputBindingDescriptions[bindingID];
            this->bindings.resize(bindingID+1u);
            this->bindings[bindingID] = vector;

            //this->bindings.push_back(vector);
            this->lastBindID = bindingID;
            return shared_from_this();
        };

        // 
        std::shared_ptr<Mesh> addAttribute(const vkh::VkVertexInputAttributeDescription& attribute = {}, const bool& isVertex = false) {
            //const uintptr_t bindingID = attribute.binding;
            //const uintptr_t locationID = this->locationCounter++;
            const uintptr_t bindingID = this->lastBindID;
            const uintptr_t locationID = attribute.location;
            this->vertexInputAttributeDescriptions.resize(locationID+1u);
            this->vertexInputAttributeDescriptions[locationID] = attribute;
            this->vertexInputAttributeDescriptions[locationID].binding = bindingID;
            this->vertexInputAttributeDescriptions[locationID].location = locationID;
            this->rawAttributes[locationID] = this->vertexInputAttributeDescriptions[locationID];
            if (isVertex) { // 
                const auto& binding = this->vertexInputBindingDescriptions[bindingID];
                this->vertexCount = this->bindings[bindingID].range() / binding.stride;
                this->geometryTemplate.geometry.triangles.vertexOffset = attribute.offset + this->bindings[bindingID].offset();
                this->geometryTemplate.geometry.triangles.vertexFormat = attribute.format;
                this->geometryTemplate.geometry.triangles.vertexStride = binding.stride;
                this->geometryTemplate.geometry.triangles.vertexCount = this->vertexCount;
                this->geometryTemplate.geometry.triangles.vertexData = this->bindings[bindingID];

                // Fix vec4 formats into vec3, without alpha (but still can be passed by stride value)
                if (attribute.format == VK_FORMAT_R32G32B32A32_SFLOAT) this->geometryTemplate.geometry.triangles.vertexFormat = VK_FORMAT_R32G32B32_SFLOAT;
                if (attribute.format == VK_FORMAT_R16G16B16A16_SFLOAT) this->geometryTemplate.geometry.triangles.vertexFormat = VK_FORMAT_R16G16B16_SFLOAT;
            };

            if (locationID == 2u) { rawMeshInfo[0u].hasNormal = 1; };
            if (locationID == 1u) { rawMeshInfo[0u].hasTexcoord = 1; };

            return shared_from_this();
        };

        // 
        template<class T = uint8_t>
        std::shared_ptr<Mesh> setIndexData(const vkt::Vector<T>& indices, const vk::IndexType& type){
            vk::DeviceSize count = 0u;
            switch (type) { // 
                case vk::IndexType::eUint32:   count = indices.range() / 4u; break;
                case vk::IndexType::eUint16:   count = indices.range() / 2u; break;
                case vk::IndexType::eUint8EXT: count = indices.range() / 1u; break;
                default: count = 0u;
            };

            // 
            if (indices.has() && type != vk::IndexType::eNoneNV) {
                this->indexData = indices;
                this->indexType = type;
                this->indexCount = count;
                this->rawMeshInfo[0u].indexType = uint32_t(type) + 1u;
            } else {
                this->indexData = {};
                this->indexType = vk::IndexType::eNoneNV;
                this->indexCount = 0u;
                this->rawMeshInfo[0u].indexType = uint32_t(type) + 1u;
            };

            // 
            this->geometryTemplate.geometry.triangles.indexOffset = this->indexData.offset();
            this->geometryTemplate.geometry.triangles.indexType = VkIndexType(this->indexType);
            this->geometryTemplate.geometry.triangles.indexCount = this->indexCount;
            this->geometryTemplate.geometry.triangles.indexData = this->indexData;

            return shared_from_this();
        };

        // 
        std::shared_ptr<Mesh> setIndexData(const vkt::Vector<uint32_t>& indices) { return this->setIndexData(indices,vk::IndexType::eUint32); };
        std::shared_ptr<Mesh> setIndexData(const vkt::Vector<uint16_t>& indices) { return this->setIndexData(indices,vk::IndexType::eUint16); };
        std::shared_ptr<Mesh> setIndexData(const vkt::Vector<uint8_t >& indices) { return this->setIndexData(indices,vk::IndexType::eUint8EXT); };
        std::shared_ptr<Mesh> setIndexData() { return this->setIndexData({},vk::IndexType::eNoneNV); };

        // some type dependent
        template<class T = uint8_t>
        std::shared_ptr<Mesh> setIndexData(const vkt::Vector<T>& indices = {}) { return this->setIndexData(indices); };

        // 
        std::shared_ptr<Mesh> setDriver(const std::shared_ptr<Driver>& driver = {}){
            this->driver = driver;
            return shared_from_this();
        };

        // 
        std::shared_ptr<Mesh> increaseInstanceCount(const uint32_t& mapID = 0u, const uint32_t& instanceCount = 1u) {
            this->rawInstanceMap[this->instanceCount] = mapID;
            this->instanceCount += instanceCount;
            return shared_from_this();
        };

        // 
        std::shared_ptr<Mesh> setInstanceCount(const uint32_t& instanceCount = 1u) {
            this->instanceCount = instanceCount;
            return shared_from_this();
        };

        // MORE useful for instanced data
        std::shared_ptr<Mesh> setTransformData(const vkt::Vector<glm::vec4>& transformData = {}, const uint32_t& stride = sizeof(glm::vec4)) {
            this->geometryTemplate.geometry.triangles.transformOffset = transformData.offset();
            this->geometryTemplate.geometry.triangles.transformData = transformData;
            this->gpuTransformData = transformData;
            this->transformStride = stride; // used for instanced correction
            this->rawMeshInfo[0u].hasTransform = 1u;
            return shared_from_this();
        };

        // 
        std::shared_ptr<Mesh> createRasterizePipeline(){
            const auto& viewport  = this->context->refViewport();
            const auto& renderArea = this->context->refScissor();

            // TODO: Add to main package
            // Enable Conservative Rasterization For Fix Some Antialiasing Issues
            vk::PipelineRasterizationConservativeStateCreateInfoEXT conserv = {};
            conserv.conservativeRasterizationMode = vk::ConservativeRasterizationModeEXT::eOverestimate;

            // 
            this->pipelineInfo = vkh::VsGraphicsPipelineCreateInfoConstruction();
            //this->pipelineInfo.rasterizationState.pNext = &conserv;
            this->pipelineInfo.vertexInputAttributeDescriptions = this->vertexInputAttributeDescriptions;
            this->pipelineInfo.vertexInputBindingDescriptions = this->vertexInputBindingDescriptions;
            for (uint32_t i=0u;i<8u;i++) {
                this->pipelineInfo.colorBlendAttachmentStates.push_back(vkh::VkPipelineColorBlendAttachmentState{ .blendEnable = true }); // transparency will generated by ray-tracing
            };
            this->pipelineInfo.stages = this->stages;
            this->pipelineInfo.depthStencilState = vkh::VkPipelineDepthStencilStateCreateInfo{
                .depthTestEnable = true,
                .depthWriteEnable = true
            };
            this->pipelineInfo.dynamicStates.push_back(VK_DYNAMIC_STATE_VIEWPORT);
            this->pipelineInfo.dynamicStates.push_back(VK_DYNAMIC_STATE_SCISSOR);
            this->pipelineInfo.graphicsPipelineCreateInfo.renderPass = this->context->renderPass;
            this->pipelineInfo.graphicsPipelineCreateInfo.layout = this->context->unifiedPipelineLayout;
            this->pipelineInfo.viewportState.pViewports = &(vkh::VkViewport&)viewport;
            this->pipelineInfo.viewportState.pScissors = &(vkh::VkRect2D&)renderArea;
            this->rasterizationState = driver->getDevice().createGraphicsPipeline(driver->getPipelineCache(),this->pipelineInfo);

            // 
            return shared_from_this();
        };

        // Create Secondary Command With Pipeline
        std::shared_ptr<Mesh> createRasterizeCommand(const vk::CommandBuffer& rasterCommand = {}, const glm::uvec4& meshData = glm::uvec4(0u)) { // UNIT ONLY!
            if (this->instanceCount <= 0u) return shared_from_this();

            // 
            std::vector<vk::Buffer> buffers = {}; std::vector<vk::DeviceSize> offsets = {};
            buffers.resize(this->bindings.size()); offsets.resize(this->bindings.size()); uintptr_t I = 0u;
            for (auto& B : this->bindings) { if (B.has()) { const uintptr_t i = I++; buffers[i] = B.buffer(); offsets[i] = B.offset(); }; };

            // 
            const auto& viewport  = this->context->refViewport();
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
            rasterCommand.beginRenderPass(vk::RenderPassBeginInfo(this->context->refRenderPass(), this->context->deferredFramebuffer, renderArea, clearValues.size(), clearValues.data()), vk::SubpassContents::eInline);
            rasterCommand.setViewport(0, { viewport });
            rasterCommand.setScissor(0, { renderArea });
            rasterCommand.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, this->context->unifiedPipelineLayout, 0ull, this->context->descriptorSets, {});
            rasterCommand.bindPipeline(vk::PipelineBindPoint::eGraphics, this->rasterizationState);
            rasterCommand.bindVertexBuffers(0u, buffers, offsets);
            rasterCommand.pushConstants<glm::uvec4>(this->context->unifiedPipelineLayout, vkh::VkShaderStageFlags{ .eVertex = 1, .eGeometry = 1, .eFragment = 1, .eRaygen = 1, .eClosestHit = 1, .eMiss = 1 }.hpp(), 0u, { meshData });

            // Make Draw Instanced
            if (this->indexType != vk::IndexType::eNoneNV) { // PLC Mode
                this->rawMeshInfo[0u].prmCount = this->indexCount / 3u;
                rasterCommand.bindIndexBuffer(this->indexData, this->indexData.offset(), this->indexType);
                //rasterCommand.drawIndexed(this->indexCount, 1u, 0u, 0u, 0u);
                rasterCommand.drawIndexed(this->indexCount, this->instanceCount, 0u, 0u, 0u);
            } else { // VAL Mode
                this->rawMeshInfo[0u].prmCount = this->vertexCount / 3u;
                //rasterCommand.draw(this->vertexCount, 1u, 0u, 0u);
                rasterCommand.draw(this->vertexCount, this->instanceCount, 0u, 0u);
            };
            rasterCommand.endRenderPass();
            //vkt::commandBarrier(rasterCommand);

            // 
            return shared_from_this();
        };

        // 
        std::shared_ptr<Mesh> copyBuffers(const vk::CommandBuffer& buildCommand = {}) {
            buildCommand.copyBuffer(this->rawAttributes , this->gpuAttributes , { vk::BufferCopy{ this->rawAttributes .offset(), this->gpuAttributes .offset(), this->gpuAttributes .range() } });
            buildCommand.copyBuffer(this->rawBindings   , this->gpuBindings   , { vk::BufferCopy{ this->rawBindings   .offset(), this->gpuBindings   .offset(), this->gpuBindings   .range() } });
            buildCommand.copyBuffer(this->rawInstanceMap, this->gpuInstanceMap, { vk::BufferCopy{ this->rawInstanceMap.offset(), this->gpuInstanceMap.offset(), this->gpuInstanceMap.range() } });
            //vkt::commandBarrier(buildCommand);
            return shared_from_this();
        };

        // 
        std::shared_ptr<Mesh> buildAccelerationStructure(const vk::CommandBuffer& buildCommand = {}) {
            if (!this->accelerationStructure) { this->createAccelerationStructure(); };
            buildCommand.buildAccelerationStructureNV((vk::AccelerationStructureInfoNV&)this->accelerationStructureInfo,{},0ull,this->needsUpdate,this->accelerationStructure,{},this->gpuScratchBuffer,this->gpuScratchBuffer.offset(),this->driver->getDispatch());
            //vkt::commandBarrier(buildCommand);
            this->needsUpdate = true; return shared_from_this();
        };

        // Create Or Rebuild Acceleration Structure
        std::shared_ptr<Mesh> createAccelerationStructure(){

            // Pre-Initialize Geometries
            // Use Same Geometry for Sub-Instances
            //this->geometries.resize(this->instanceCount);
            //this->geometries = {};
            this->geometries.resize(1u);
            for (uint32_t i = 0u; i < this->geometries.size(); i++) {
                //this->geometries.push_back(this->geometryTemplate);
                this->geometries[i] = this->geometryTemplate;
                this->geometries[i].flags = { .eOpaque = 1 };
                if (this->gpuTransformData.has()) {
                    this->geometries[i].geometry.triangles.transformOffset = vk::DeviceSize(this->transformStride) * vk::DeviceSize(i) + this->gpuTransformData.offset(); // Should To Be Different's
                    this->geometries[i].geometry.triangles.transformData = this->gpuTransformData;
                };
            };

            // Re-assign instance count
            this->accelerationStructureInfo.geometryCount = this->geometries.size();
            this->accelerationStructureInfo.pGeometries = this->geometries.data();
            this->accelerationStructureInfo.flags = VK_BUILD_ACCELERATION_STRUCTURE_ALLOW_UPDATE_BIT_NV | VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_BUILD_BIT_NV;
            this->accelerationStructureInfo.instanceCount = 0u;

            // 
            if (!this->accelerationStructure) { // create acceleration structure fastly...
                this->accelerationStructure = this->driver->getDevice().createAccelerationStructureNV(vkh::VkAccelerationStructureCreateInfoNV{
                    .info = this->accelerationStructureInfo
                }, nullptr, this->driver->getDispatch());

                //
                auto requirements = this->driver->getDevice().getAccelerationStructureMemoryRequirementsNV(vkh::VkAccelerationStructureMemoryRequirementsInfoNV{
                    .type = VK_ACCELERATION_STRUCTURE_MEMORY_REQUIREMENTS_TYPE_OBJECT_NV,
                    .accelerationStructure = this->accelerationStructure
                }, this->driver->getDispatch());

                // 
                VmaAllocationCreateInfo allocInfo = {};
                allocInfo.memoryTypeBits |= requirements.memoryRequirements.memoryTypeBits;
                vmaAllocateMemory(this->driver->getAllocator(),&(VkMemoryRequirements&)requirements.memoryRequirements,&allocInfo,&this->allocation,&this->allocationInfo);

                // 
                this->driver->getDevice().bindAccelerationStructureMemoryNV({vkh::VkBindAccelerationStructureMemoryInfoNV{
                    .accelerationStructure = this->accelerationStructure,
                    .memory = this->allocationInfo.deviceMemory,
                    .memoryOffset = this->allocationInfo.offset
                }}, this->driver->getDispatch());
            };

            // 
            if (!this->gpuScratchBuffer.has()) { // 
                auto requirements = this->driver->getDevice().getAccelerationStructureMemoryRequirementsNV(vkh::VkAccelerationStructureMemoryRequirementsInfoNV{
                    .type = VK_ACCELERATION_STRUCTURE_MEMORY_REQUIREMENTS_TYPE_BUILD_SCRATCH_NV,
                    .accelerationStructure = this->accelerationStructure
                }, this->driver->getDispatch());

                // 
                this->gpuScratchBuffer = vkt::Vector<uint8_t>(std::make_shared<vkt::VmaBufferAllocation>(this->driver->getAllocator(), vkh::VkBufferCreateInfo{
                    .size = requirements.memoryRequirements.size,
                    .usage = { .eStorageBuffer = 1, .eRayTracing = 1 }
                }, VMA_MEMORY_USAGE_GPU_ONLY));
            };

            // 
            return shared_from_this();
        };

    // 
    protected: friend Mesh; friend Node; friend Renderer; // GPU Vertex and Attribute Data
        vkt::Vector<uint8_t> indexData = {}; 
        vk::IndexType indexType = vk::IndexType::eNoneNV;
        uint32_t indexCount = 0u, vertexCount = 0u, instanceCount = 0u;
        bool needsUpdate = false;

        // 
        std::vector<vkt::Vector<uint8_t>> bindings = {};
        std::vector<vkh::VkVertexInputBindingDescription> vertexInputBindingDescriptions = {};
        std::vector<vkh::VkVertexInputAttributeDescription> vertexInputAttributeDescriptions = {};
        std::vector<vkh::VkPipelineShaderStageCreateInfo> stages = {};

        // accumulated by "Instance" for instanced rendering
        uint32_t transformStride = sizeof(glm::vec4);
        vkt::Vector<glm::vec4> gpuTransformData = {};
        uint32_t lastBindID = 0u, locationCounter = 0u;

        // 
        vkh::VsGraphicsPipelineCreateInfoConstruction pipelineInfo = {};
        vkh::VkAccelerationStructureInfoNV accelerationStructureInfo = {};
        std::vector<vkh::VkGeometryNV> geometries = {};

        // 
        //vk::CommandBuffer buildCommand = {};
        //vk::CommandBuffer rasterCommand = {};
        vkh::VkGeometryNV geometryTemplate = {};
        vk::Pipeline rasterizationState = {}; // Vertex Input can changed, so use individual rasterization stages

        // WIP buffer bindings
        vkt::Vector<vkh::VkVertexInputAttributeDescription> rawAttributes = {};
        vkt::Vector<vkh::VkVertexInputAttributeDescription> gpuAttributes = {};
        vkt::Vector<vkh::VkVertexInputBindingDescription> rawBindings = {};
        vkt::Vector<vkh::VkVertexInputBindingDescription> gpuBindings = {};
        vkt::Vector<uint8_t> gpuScratchBuffer = {};

        // 
        vkt::Vector<MeshInfo> rawMeshInfo = {};
        vkt::Vector<uint32_t> rawInstanceMap = {};
        vkt::Vector<uint32_t> gpuInstanceMap = {};

        // 
        vk::AccelerationStructureNV accelerationStructure = {};
        VmaAllocationInfo allocationInfo = {};
        VmaAllocation allocation = {};

        // 
        std::shared_ptr<Driver> driver = {};
        std::shared_ptr<Thread> thread = {};
        std::shared_ptr<Context> context = {};
        //std::shared_ptr<Renderer> renderer = {};
    };

};
