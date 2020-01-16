#pragma once
#include "./config.hpp"
#include "./driver.hpp"
#include "./thread.hpp"
#include "./context.hpp"

namespace lancer {

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
            this->accelerationStructureInfo.geometryCount = this->instanceCount;
            this->accelerationStructureInfo.pGeometries = &this->geometryTemplate;
            this->accelerationStructureInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_NV;
            this->pipelineInfo = vkh::VsGraphicsPipelineCreateInfoConstruction();

            // create required buffers
            this->rawBindings = vkt::Vector<VkVertexInputBindingDescription>(std::make_shared<vkt::VmaBufferAllocation>(this->driver->getAllocator(), vkh::VkBufferCreateInfo{ .size = sizeof(VkVertexInputBindingDescription)*8u, .usage = { .eTransferSrc = 1, .eUniformBuffer = 1, .eStorageBuffer = 1, .eRayTracing = 1 } }, VMA_MEMORY_USAGE_CPU_TO_GPU));
            this->gpuBindings = vkt::Vector<VkVertexInputBindingDescription>(std::make_shared<vkt::VmaBufferAllocation>(this->driver->getAllocator(), vkh::VkBufferCreateInfo{ .size = sizeof(VkVertexInputBindingDescription)*8u, .usage = { .eTransferDst = 1, .eUniformBuffer = 1, .eStorageBuffer = 1, .eRayTracing = 1 } }, VMA_MEMORY_USAGE_GPU_ONLY));
            this->rawAttributes = vkt::Vector<VkVertexInputAttributeDescription>(std::make_shared<vkt::VmaBufferAllocation>(this->driver->getAllocator(), vkh::VkBufferCreateInfo{ .size = sizeof(VkVertexInputAttributeDescription)*8u, .usage = { .eTransferSrc = 1, .eUniformBuffer = 1, .eStorageBuffer = 1, .eRayTracing = 1 }}, VMA_MEMORY_USAGE_CPU_TO_GPU));
            this->gpuAttributes = vkt::Vector<VkVertexInputAttributeDescription>(std::make_shared<vkt::VmaBufferAllocation>(this->driver->getAllocator(), vkh::VkBufferCreateInfo{ .size = sizeof(VkVertexInputAttributeDescription)*8u, .usage = { .eTransferDst = 1, .eUniformBuffer = 1, .eStorageBuffer = 1, .eRayTracing = 1 }}, VMA_MEMORY_USAGE_GPU_ONLY));

            // 
            this->rawMeshInfo = vkt::Vector<glm::uvec4>(std::make_shared<vkt::VmaBufferAllocation>(this->driver->getAllocator(), vkh::VkBufferCreateInfo{ .size = 16u, .usage = {.eTransferSrc = 1, .eUniformBuffer = 1, .eStorageBuffer = 1, .eRayTracing = 1 } }, VMA_MEMORY_USAGE_CPU_TO_GPU));
            this->gpuMeshInfo = vkt::Vector<glm::uvec4>(std::make_shared<vkt::VmaBufferAllocation>(this->driver->getAllocator(), vkh::VkBufferCreateInfo{ .size = 16u, .usage = {.eTransferSrc = 1, .eUniformBuffer = 1, .eStorageBuffer = 1, .eRayTracing = 1 } }, VMA_MEMORY_USAGE_GPU_ONLY));
        };

        // 
        std::shared_ptr<Mesh> setMaterialID(const uint32_t& materialID = 0u) {
            this->rawMeshInfo[0u][0u] = materialID;
            return shared_from_this();
        };

        // 
        std::shared_ptr<Mesh> setThread(const std::shared_ptr<Thread>& thread) {
            this->thread = thread;
            return shared_from_this();
        };

        // 
        std::shared_ptr<Mesh> addBinding(const vkt::Vector<uint8_t>& vector, const vkh::VkVertexInputBindingDescription& binding = {}) {
            this->lastBindID = this->vertexInputBindingDescriptions.size();
            this->vertexInputBindingDescriptions.push_back(binding);
            this->vertexInputBindingDescriptions.back().binding = this->lastBindID;
            this->rawBindings[this->lastBindID] = this->vertexInputBindingDescriptions.back();
            this->bindings.push_back(vector);
            return shared_from_this();
        };

        // 
        std::shared_ptr<Mesh> addAttribute(const vkh::VkVertexInputAttributeDescription& attribute = {}, const bool& isVertex = false) {
            const uintptr_t locationID = this->locationCounter++;
            this->vertexInputAttributeDescriptions.push_back(attribute);
            this->vertexInputAttributeDescriptions.back().binding = this->lastBindID;
            this->vertexInputAttributeDescriptions.back().location = locationID;
            this->rawAttributes[locationID] = this->vertexInputAttributeDescriptions.back();
            if (isVertex) { // 
                const auto& binding = this->vertexInputBindingDescriptions.back();
                this->vertexCount = this->bindings.back().range() / binding.stride;
                this->geometryTemplate.geometry.triangles.vertexOffset = attribute.offset + this->bindings.back().offset();
                this->geometryTemplate.geometry.triangles.vertexFormat = attribute.format;
                this->geometryTemplate.geometry.triangles.vertexStride = binding.stride;
                this->geometryTemplate.geometry.triangles.vertexCount = this->vertexCount;
                this->geometryTemplate.geometry.triangles.vertexData = this->bindings.back();
                
                // Fix vec4 formats into vec3, without alpha (but still can be passed by stride value)
                if (attribute.format == VK_FORMAT_R32G32B32A32_SFLOAT) this->geometryTemplate.geometry.triangles.vertexFormat = VK_FORMAT_R32G32B32_SFLOAT;
                if (attribute.format == VK_FORMAT_R16G16B16A16_SFLOAT) this->geometryTemplate.geometry.triangles.vertexFormat = VK_FORMAT_R16G16B16_SFLOAT;
            };
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
                this->rawMeshInfo[0u][1u] = 1u;
            } else {
                this->indexData = {};
                this->indexType = vk::IndexType::eNoneNV;
                this->indexCount = 0u;
                this->rawMeshInfo[0u][1u] = 0u;
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
        std::shared_ptr<Mesh> increaseInstanceCount(const uint32_t& instanceCount = 1u) {
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
            return shared_from_this();
        };

        // 
        std::shared_ptr<Mesh> createRasterizePipeline(){
            const auto& viewport  = this->context->refViewport();
            const auto& renderArea = this->context->refScissor();

            //this->context->descriptorSets[3] = this->context->smpFlip1DescriptorSet;

            this->pipelineInfo = vkh::VsGraphicsPipelineCreateInfoConstruction();
            this->pipelineInfo.vertexInputAttributeDescriptions = this->vertexInputAttributeDescriptions;
            this->pipelineInfo.vertexInputBindingDescriptions = this->vertexInputBindingDescriptions;
            for (uint32_t i=0u;i<4u;i++) {
                this->pipelineInfo.colorBlendAttachmentStates.push_back(vkh::VkPipelineColorBlendAttachmentState{ .blendEnable = true }); // transparency will generated by ray-tracing
            };
            this->pipelineInfo.stages = vkt::vector_cast<vkh::VkPipelineShaderStageCreateInfo, vk::PipelineShaderStageCreateInfo>({
                vkt::makePipelineStageInfo(this->driver->getDevice(), vkt::readBinary("./shaders/rtrace/rasterize.vert.spv"), vk::ShaderStageFlagBits::eVertex),
                vkt::makePipelineStageInfo(this->driver->getDevice(), vkt::readBinary("./shaders/rtrace/rasterize.frag.spv"), vk::ShaderStageFlagBits::eFragment)
            });
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
            for (auto& B : this->bindings) { buffers.push_back(B); offsets.push_back(B.offset()); };

            // 
            const auto& viewport  = this->context->refViewport();
            const auto& renderArea = this->context->refScissor();
            const auto clearValues = std::vector<vk::ClearValue>{ 
                vk::ClearColorValue(std::array<float,4>{0.f, 0.f, 0.f, 1.0f}), 
                vk::ClearColorValue(std::array<float,4>{0.f, 0.f, 0.f, 1.0f}), 
                vk::ClearColorValue(std::array<float,4>{0.f, 0.f, 0.f, 1.0f}), 
                vk::ClearColorValue(std::array<float,4>{0.f, 0.f, 0.f, 1.0f}), 
                vk::ClearDepthStencilValue(1.0f, 0)
            };

            // 
            rasterCommand.beginRenderPass(vk::RenderPassBeginInfo(this->context->refRenderPass(), this->context->deferredFramebuffer, renderArea, clearValues.size(), clearValues.data()), vk::SubpassContents::eInline);
            rasterCommand.setViewport(0, { viewport });
            rasterCommand.setScissor(0, { renderArea });
            rasterCommand.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, this->context->unifiedPipelineLayout, 0ull, this->context->descriptorSets, {});
            rasterCommand.bindPipeline(vk::PipelineBindPoint::eGraphics, this->rasterizationState);
            rasterCommand.bindVertexBuffers(0u, buffers, offsets);
            rasterCommand.pushConstants<glm::uvec4>(this->context->unifiedPipelineLayout, vk::ShaderStageFlags(VkShaderStageFlags(vkh::VkShaderStageFlags{ .eVertex = 1, .eFragment = 1, .eRaygen = 1, .eClosestHit = 1 })), 0u, { meshData });

            // Make Draw Instanced
            if (this->indexType != vk::IndexType::eNoneNV) { // PLC Mode
                rasterCommand.bindIndexBuffer(this->indexData, this->indexData.offset(), this->indexType);
                rasterCommand.drawIndexed(this->indexCount, this->instanceCount, 0u, 0u, 0u);
            } else { // VAL Mode
                rasterCommand.draw(this->vertexCount, this->instanceCount, 0u, 0u);
            };
            rasterCommand.endRenderPass();

            // 
            return shared_from_this();
        };

        // 
        std::shared_ptr<Mesh> copyBuffers(const vk::CommandBuffer& buildCommand = {}) {
            buildCommand.copyBuffer(this->rawAttributes, this->gpuAttributes, { vk::BufferCopy{ this->rawAttributes.offset(), this->gpuAttributes.offset(), this->gpuAttributes.range() } });
            buildCommand.copyBuffer(this->rawBindings  , this->gpuBindings  , { vk::BufferCopy{ this->rawBindings  .offset(), this->gpuBindings  .offset(), this->gpuBindings  .range() } });
            buildCommand.copyBuffer(this->rawMeshInfo  , this->gpuMeshInfo  , { vk::BufferCopy{ this->rawMeshInfo  .offset(), this->gpuMeshInfo  .offset(), this->gpuMeshInfo  .range() } });
            return shared_from_this();
        };

        // 
        std::shared_ptr<Mesh> buildAccelerationStructure(const vk::CommandBuffer& buildCommand = {}) {
            if (!this->accelerationStructure) { this->createAccelerationStructure(); };
            buildCommand.buildAccelerationStructureNV((vk::AccelerationStructureInfoNV&)this->accelerationStructureInfo,{},0ull,this->needsUpdate,this->accelerationStructure,{},this->gpuScratchBuffer,this->gpuScratchBuffer.offset(),this->driver->getDispatch());
            this->needsUpdate = true; return shared_from_this();
        };

        // Create Or Rebuild Acceleration Structure
        std::shared_ptr<Mesh> createAccelerationStructure(){

            // Pre-Initialize Geometries
            // Use Same Geometry for Sub-Instances
            this->geometries.resize(this->instanceCount);
            for (uint32_t i = 0u; i < this->instanceCount; i++) {
                this->geometries[i] = this->geometryTemplate;
                this->geometries[i].flags = { .eOpaque = 1 };
                if (this->gpuTransformData.has()) {
                    this->geometries[i].geometry.triangles.transformOffset = this->transformStride * i + this->gpuTransformData.offset(); // Should To Be Different's
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

        vkt::Vector<glm::uvec4> rawMeshInfo = {};
        vkt::Vector<glm::uvec4> gpuMeshInfo = {};

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
