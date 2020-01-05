#pragma once
#include "./config.hpp"
#include "./driver.hpp"
#include "./thread.hpp"
#include "./context.hpp"

namespace lancer {

    // WIP Mesh Object
    // Sub-Instances Can Be Supported
    // TODO: Descriptor Sets
    class Mesh : public std::enable_shared_from_this<Mesh> { public: friend Instance; friend Renderer;
        Mesh(const std::shared_ptr<Driver>& driver) {
            this->driver = driver;
            this->thread = std::make_shared<Thread>(this->driver);

            // 
            this->accelerationStructureInfo.geometryCount = this->instanceCount;
            this->accelerationStructureInfo.pGeometries = &this->geometryTemplate;
            this->accelerationStructureInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_NV;

            // create required buffers
            this->rawBindings = vkt::Vector<uint8_t>(std::make_shared<vkt::VmaBufferAllocation>(this->driver->getAllocator(), vkh::VkBufferCreateInfo{ .size = sizeof(VkVertexInputBindingDescription)*8u, .usage = { .eUniformBuffer = 1, .eRayTracing = 1 } }, VMA_MEMORY_USAGE_CPU_TO_GPU));
            this->gpuBindings = vkt::Vector<uint8_t>(std::make_shared<vkt::VmaBufferAllocation>(this->driver->getAllocator(), vkh::VkBufferCreateInfo{ .size = sizeof(VkVertexInputBindingDescription)*8u, .usage = { .eUniformBuffer = 1, .eRayTracing = 1 } }, VMA_MEMORY_USAGE_GPU_ONLY));
            this->rawAttributes = vkt::Vector<uint8_t>(std::make_shared<vkt::VmaBufferAllocation>(this->driver->getAllocator(), vkh::VkBufferCreateInfo{ .size = sizeof(VkVertexInputAttributeDescription)*8u, .usage = { .eUniformBuffer = 1, .eRayTracing = 1 }}, VMA_MEMORY_USAGE_CPU_TO_GPU));
            this->gpuAttributes = vkt::Vector<uint8_t>(std::make_shared<vkt::VmaBufferAllocation>(this->driver->getAllocator(), vkh::VkBufferCreateInfo{ .size = sizeof(VkVertexInputAttributeDescription)*8u, .usage = { .eUniformBuffer = 1, .eRayTracing = 1 }}, VMA_MEMORY_USAGE_GPU_ONLY));
        };

        // 
        std::shared_ptr<Mesh> setThread(const std::shared_ptr<Thread>& thread) {
            this->thread = thread;
            return shared_from_this();
        };

        // 
        std::shared_ptr<Mesh> addBinding(const vkt::Vector<uint8_t>& vector, const vkh::VkVertexInputBindingDescription& binding = {}) {
            this->lastBindID = pipelineInfo.vertexInputBindingDescriptions.size();
            this->pipelineInfo.vertexInputBindingDescriptions.push_back(binding);
            this->pipelineInfo.vertexInputBindingDescriptions.back().binding = this->lastBindID;
            this->rawBindings[this->lastBindID] = this->pipelineInfo.vertexInputBindingDescriptions.back();
            this->bindings.push_back(vector);
            return shared_from_this();
        };

        // 
        std::shared_ptr<Mesh> addAttribute(const vkh::VkVertexInputAttributeDescription& attribute = {}, const bool& isVertex = false) {
            const uintptr_t locationID = this->locationCounter++;
            this->pipelineInfo.vertexInputAttributeDescriptions.push_back(attribute);
            this->pipelineInfo.vertexInputAttributeDescriptions.back().binding = this->lastBindID;
            this->pipelineInfo.vertexInputAttributeDescriptions.back().location = locationID;
            this->rawAttributes[locationID] = this->pipelineInfo.vertexInputAttributeDescriptions.back();
            if (isVertex) { // TODO: fix from vec4 into vec3
                const auto& binding = this->pipelineInfo.vertexInputBindingDescriptions.back();
                this->vertexCount = this->bindings.back().range() / binding.stride;
                this->geometryTemplate.geometry.triangles.vertexOffset = attribute.offset + this->bindings.back().offset();
                this->geometryTemplate.geometry.triangles.vertexFormat = attribute.format;
                this->geometryTemplate.geometry.triangles.vertexStride = binding.stride;
                this->geometryTemplate.geometry.triangles.vertexCount = this->vertexCount;
                this->geometryTemplate.geometry.triangles.vertexData = this->bindings.back();
            };
            return shared_from_this();
        };

        // Uint32_T version
        std::shared_ptr<Mesh> setIndexData(const vkt::Vector<uint32_t>& indices){
            this->indexData = indices;
            this->indexType = vk::IndexType::eUint32;
            this->indexCount = indices.size();
            this->geometryTemplate.geometry.triangles.indexOffset = indices.offset();
            this->geometryTemplate.geometry.triangles.indexType = VkIndexType(this->indexType);
            this->geometryTemplate.geometry.triangles.indexCount = indices.size();
            this->geometryTemplate.geometry.triangles.indexData = indices;
            return shared_from_this();
        };

        // Uint16_T version
        std::shared_ptr<Mesh> setIndexData(const vkt::Vector<uint16_t>& indices){
            this->indexData = indices;
            this->indexType = vk::IndexType::eUint16;
            this->indexCount = indices.size();
            this->geometryTemplate.geometry.triangles.indexOffset = indices.offset();
            this->geometryTemplate.geometry.triangles.indexType = VkIndexType(this->indexType);
            this->geometryTemplate.geometry.triangles.indexCount = indices.size();
            this->geometryTemplate.geometry.triangles.indexData = indices;
            return shared_from_this();
        };

        // Uint8_T version
        std::shared_ptr<Mesh> setIndexData(const vkt::Vector<uint8_t>& indices){
            this->indexData = indices;
            this->indexType = vk::IndexType::eUint8EXT;
            this->indexCount = indices.size();
            this->geometryTemplate.geometry.triangles.indexOffset = indices.offset();
            this->geometryTemplate.geometry.triangles.indexType = VkIndexType(this->indexType);
            this->geometryTemplate.geometry.triangles.indexCount = indices.size();
            this->geometryTemplate.geometry.triangles.indexData = indices;
            return shared_from_this();
        };

        // Reset Indices
        std::shared_ptr<Mesh> setIndexData(){
            this->indexData = {};
            this->indexType = vk::IndexType::eNoneNV;
            this->indexCount = 0u;
            this->geometryTemplate.geometry.triangles.indexOffset = 0u;
            this->geometryTemplate.geometry.triangles.indexType = VkIndexType(this->indexType);
            this->geometryTemplate.geometry.triangles.indexCount = 0u;
            this->geometryTemplate.geometry.triangles.indexData = {};
            return shared_from_this();
        };

        // 
        template<class T = uint8_t> // Use Unified and Dynamic Version
        std::shared_ptr<Mesh> setIndexData(const vkt::Vector<T>& indices, const vk::IndexType& type){
            switch(type) {
                case vk::IndexType::eUint32: return this->setIndexData(vkt::Vector<uint32_t>(indices)); break; // uint32_t version 
                case vk::IndexType::eUint16: return this->setIndexData(vkt::Vector<uint16_t>(indices)); break; // uint16_t version
                case vk::IndexType::eUint8EXT: return this->setIndexData(vkt::Vector<uint8_t>(indices)); break; // uint8_t version
                default: return this->setIndexData();
            };
            return shared_from_this();
        };

        // 
        std::shared_ptr<Mesh> setDriver(const std::shared_ptr<Driver>& driver = {}){
            this->driver = driver;
            return shared_from_this();
        };

        // 
        std::shared_ptr<Mesh> setInstanceCount(const uint32_t& instanceCount = 1u) {
            this->instanceCount = instanceCount;
            return shared_from_this();
        };

        // MORE useful for instanced data
        std::shared_ptr<Mesh> setTransformData(const vkt::Vector<glm::mat3x4>& transformData = {}, const uint32_t& stride = sizeof(glm::mat3x4)) {
            this->geometryTemplate.geometry.triangles.transformOffset = transformData.offset();
            this->geometryTemplate.geometry.triangles.transformData = transformData;
            this->gpuTransformData = transformData;
            this->transformStride = stride; // used for instanced correction
            return shared_from_this();
        };

        // Create Secondary Command With Pipeline
        std::shared_ptr<Mesh> createRasterizeCommand() { // UNIT ONLY!
            std::vector<vk::Buffer> buffers = {}; std::vector<vk::DeviceSize> offsets = {};
            for (auto& B : this->bindings) { buffers.push_back(B); offsets.push_back(B.offset()); };

            // 
            const auto& viewport  = this->context->refViewport();
            const auto& renderArea = this->context->refScissor();
            const auto clearValues = std::vector<vk::ClearValue>{ 
                vk::ClearColorValue(std::array<float,4>{0.f, 0.f, 0.f, 0.0f}), 
                vk::ClearColorValue(std::array<float,4>{0.f, 0.f, 0.f, 0.0f}), 
                vk::ClearColorValue(std::array<float,4>{0.f, 0.f, 0.f, 0.0f}), 
                vk::ClearColorValue(std::array<float,4>{0.f, 0.f, 0.f, 0.0f}), 
                vk::ClearDepthStencilValue(1.0f, 0)
            };

            // 
            for (uint32_t i=0u;i<4u;i++) {
                this->pipelineInfo.colorBlendAttachmentStates.push_back(vkh::VkPipelineColorBlendAttachmentState{ .blendEnable = true }); // transparency will generated by ray-tracing
            };
            this->pipelineInfo.stages = vkt::vector_cast<vkh::VkPipelineShaderStageCreateInfo, vk::PipelineShaderStageCreateInfo>({
                vkt::makePipelineStageInfo(this->driver->getDevice(), vkt::readBinary("./shaders/rasterize.vert.spv"), vk::ShaderStageFlagBits::eVertex),
                vkt::makePipelineStageInfo(this->driver->getDevice(), vkt::readBinary("./shaders/rasterize.frag.spv"), vk::ShaderStageFlagBits::eFragment)
            });
            this->pipelineInfo.depthStencilState = vkh::VkPipelineDepthStencilStateCreateInfo{
                .depthTestEnable = true,
                .depthWriteEnable = true
            };
            this->pipelineInfo.graphicsPipelineCreateInfo.renderPass = this->context->renderPass;
            this->pipelineInfo.graphicsPipelineCreateInfo.layout = this->context->unifiedPipelineLayout;
            this->pipelineInfo.viewportState.pViewports = &(vkh::VkViewport&)viewport;
            this->pipelineInfo.viewportState.pScissors = &(vkh::VkRect2D&)renderArea;
            this->rasterizationState = driver->getDevice().createGraphicsPipeline(driver->getPipelineCache(),this->pipelineInfo);
            this->rasterCommand = vkt::createCommandBuffer(*thread, *thread, true, false); // do reference of cmd buffer
            this->rasterCommand.beginRenderPass(vk::RenderPassBeginInfo(this->context->refRenderPass, this->context->deferredFramebuffer, renderArea, clearValues.size(), clearValues.data()), vk::SubpassContents::eInline);
            this->rasterCommand.setViewport(0, { viewport });
            this->rasterCommand.setScissor(0, { renderArea });
            this->rasterCommand.bindPipeline(vk::PipelineBindPoint::eGraphics, this->rasterizationState);
            this->rasterCommand.bindVertexBuffers(0u, buffers, offsets);

            // Make Draw Instanced 
            if (indexType != vk::IndexType::eNoneNV) { // PLC Mode
                this->rasterCommand.bindIndexBuffer(this->indexData, this->indexData.offset(), this->indexType);
                this->rasterCommand.drawIndexed(this->indexCount, this->instanceCount, 0u, 0u, 0u);
            } else { // VAL Mode
                this->rasterCommand.draw(this->vertexCount, this->instanceCount, 0u, 0u);
            };

            // 
            this->rasterCommand.endRenderPass();
            vkt::commandBarrier(this->rasterCommand);
            this->rasterCommand.end();

            // 
            return shared_from_this();
        };

        // 
        std::shared_ptr<Mesh> buildAccelerationStructure() {
            if (!this->accelerationStructure) { this->createAccelerationStructure(); };
            this->buildCommand = vkt::createCommandBuffer(*this->thread, *this->thread, true, false);
            this->buildCommand.copyBuffer(this->rawBindings  , this->gpuBindings  , { vk::BufferCopy{ this->rawBindings  .offset(), this->gpuBindings.  offset(), this->gpuBindings.  range() } });
            this->buildCommand.copyBuffer(this->rawAttributes, this->gpuAttributes, { vk::BufferCopy{ this->rawAttributes.offset(), this->gpuAttributes.offset(), this->gpuAttributes.range() } });
            vkt::commandBarrier(this->buildCommand);
            this->buildCommand.buildAccelerationStructureNV(this->accelerationStructureInfo,{},0ull,this->needsUpdate,this->accelerationStructure,{},this->gpuScratchBuffer,this->gpuScratchBuffer.offset());
            vkt::commandBarrier(this->buildCommand);
            this->buildCommand.end();
            return shared_from_this();
        };

        // Create Or Rebuild Acceleration Structure
        std::shared_ptr<Mesh> createAccelerationStructure(){

            // Pre-Initialize Geometries
            // Use Same Geometry for Sub-Instances
            this->geometries.resize(instanceCount);
            for (uint32_t i = 0u; i < instanceCount; i++) {
                this->geometries[i] = this->geometryTemplate;
                this->geometries[i].geometry.triangles.transformOffset = this->transformStride * i + this->gpuTransformData.offset(); // Should To Be Different's
                this->geometries[i].geometry.triangles.transformData = gpuTransformData;
            };

            // Re-assign instance count
            this->accelerationStructureInfo.geometryCount = this->geometries.size();
            this->accelerationStructureInfo.pGeometries = this->geometries.data();

            // 
            if (!this->accelerationStructure) { // create acceleration structure fastly...
                this->accelerationStructure = this->driver->getDevice().createAccelerationStructureNV(vkh::VkAccelerationStructureCreateInfoNV{
                    .info = this->accelerationStructureInfo
                });

                //
                auto requirements = this->driver->getDevice().getAccelerationStructureMemoryRequirementsNV(vkh::VkAccelerationStructureMemoryRequirementsInfoNV{
                    .type = VK_ACCELERATION_STRUCTURE_MEMORY_REQUIREMENTS_TYPE_OBJECT_NV,
                    .accelerationStructure = this->accelerationStructure
                });

                // 
                VmaAllocationCreateInfo allocInfo = {};
                allocInfo.memoryTypeBits |= requirements.memoryRequirements.memoryTypeBits;
                vmaAllocateMemory(this->driver->getAllocator(),&(VkMemoryRequirements&)requirements.memoryRequirements,&allocInfo,&this->allocation,&this->allocationInfo);

                // 
                this->driver->getDevice().bindAccelerationStructureMemoryNV({vkh::VkBindAccelerationStructureMemoryInfoNV{
                    .accelerationStructure = this->accelerationStructure,
                    .memory = this->allocationInfo.deviceMemory,
                    .memoryOffset = this->allocationInfo.offset
                }});
            };

            // 
            if (!this->gpuScratchBuffer.has()) { // 
                auto requirements = this->driver->getDevice().getAccelerationStructureMemoryRequirementsNV(vkh::VkAccelerationStructureMemoryRequirementsInfoNV{
                    .type = VK_ACCELERATION_STRUCTURE_MEMORY_REQUIREMENTS_TYPE_BUILD_SCRATCH_NV,
                    .accelerationStructure = this->accelerationStructure
                });

                // 
                this->gpuScratchBuffer = vkt::Vector<uint8_t>(std::make_shared<vkt::VmaBufferAllocation>(this->driver->getAllocator(), vkh::VkBufferCreateInfo{
                    .size = requirements.memoryRequirements.size,
                    .usage = { .eStorageBuffer = 1, .eRayTracing = 1 }
                }, VMA_MEMORY_USAGE_GPU_ONLY));
            };

            // 
            return shared_from_this();
        };


    protected: friend Mesh; friend Instance; friend Renderer; // GPU Vertex and Attribute Data
        vkt::Vector<uint8_t> indexData = {}; 
        vk::IndexType indexType = vk::IndexType::eNoneNV;
        uint32_t indexCount = 0u, vertexCount = 0u, instanceCount = 1u;
        bool needsUpdate = false;

        // 
        std::vector<vkt::Vector<uint8_t>> bindings = {};

        // accumulated by "Instance" for instanced rendering
        uint32_t transformStride = sizeof(glm::mat3x4);
        vkt::Vector<glm::mat3x4> gpuTransformData = {};
        uint32_t lastBindID = 0u, locationCounter = 0u;

        // 
        vkh::VsGraphicsPipelineCreateInfoConstruction pipelineInfo = {};
        vkh::VkAccelerationStructureInfoNV accelerationStructureInfo = {};
        std::vector<vkh::VkGeometryNV> geometries = {};

        // 
        vk::CommandBuffer buildCommand = {};
        vk::CommandBuffer rasterCommand = {};
        vkh::VkGeometryNV geometryTemplate = {};
        vk::Pipeline rasterizationState = {}; // Vertex Input can changed, so use individual rasterization stages

        // WIP buffer bindings
        vkt::Vector<VkVertexInputAttributeDescription> rawAttributes = {};
        vkt::Vector<VkVertexInputAttributeDescription> gpuAttributes = {};
        vkt::Vector<VkVertexInputBindingDescription> rawBindings = {};
        vkt::Vector<VkVertexInputBindingDescription> gpuBindings = {};
        vkt::Vector<uint8_t> gpuScratchBuffer = {};

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
