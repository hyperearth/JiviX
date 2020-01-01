#pragma once
#include "./config.hpp"
#include "./driver.hpp"
#include "./thread.hpp"
#include "./context.hpp"

namespace lancer {

    // WIP Mesh Object
    // Sub-Instances Can Be Supported
    class Mesh : public std::enable_shared_from_this<Mesh> { public: //friend Instance;
        Mesh() {
            this->accelerationStructureInfo.geometryCount = instanceCount;
            this->accelerationStructureInfo.pGeometries = &geometryTemplate;
            this->accelerationStructureInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_NV;
        };

        // 
        std::shared_ptr<Mesh> addBinding(const vkt::Vector<uint8_t>& vector, const vkh::VkVertexInputBindingDescription& binding = {}) {
            this->lastBindID = pipelineInfo.vertexInputBindingDescriptions.size();
            this->pipelineInfo.vertexInputBindingDescriptions.push_back(binding);
            this->pipelineInfo.vertexInputBindingDescriptions.back().binding = lastBindID;
            this->bindings.push_back(vector);
            return shared_from_this();
        };

        // 
        std::shared_ptr<Mesh> addAttribute(const vkh::VkVertexInputAttributeDescription& attribute = {}, const bool& isVertex = false) {
            this->pipelineInfo.vertexInputAttributeDescriptions.push_back(attribute);
            this->pipelineInfo.vertexInputAttributeDescriptions.back().binding = lastBindID;
            this->pipelineInfo.vertexInputAttributeDescriptions.back().location = locationCounter++;

            // 
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

        // 
        std::shared_ptr<Mesh> setTransformData(const vkt::Vector<glm::mat3x4>& transformData = {}) {
            this->geometryTemplate.geometry.triangles.transformOffset = transformData.offset();
            this->geometryTemplate.geometry.triangles.transformData = transformData;
            this->gpuTransformData = transformData;
            return shared_from_this();
        };

        // TOOD: Context Object
        std::shared_ptr<Mesh> createPipeline() {
            this->pipelineInfo.graphicsPipelineCreateInfo.layout = this->context->unifiedPipelineLayout;
            this->pipelineInfo.graphicsPipelineCreateInfo.renderPass = this->context->refViewport;
            this->driver->getDevice().createGraphicsPipeline(this->driver->getPipelineCache(), this->pipelineInfo);
            return shared_from_this();
        };

        // 
        //vkh::VsGraphicsPipelineCreateInfoConstruction& refPipelineCreateInfo() { return pipelineInfo; };
        //const vkh::VsGraphicsPipelineCreateInfoConstruction& refPipelineCreateInfo() const { return pipelineInfo; };

        // TODO: Rasterization and Ray-Tracing Stages
        // TODO: Instancing Support
        // Create Command With Pipeline
        std::shared_ptr<Mesh> createRasterizeCommand() { // UNIT ONLY!
            std::vector<vk::Buffer> buffers = {};
            std::vector<vk::DeviceSize> offsets = {};
            for (auto& B : this->bindings) { buffers.push_back(B); offsets.push_back(B.offset()); };

            // 
            const auto& renderArea = this->context->refScissor();
            const auto& viewport = this->context->refViewport();

            // TODO: Fix Clear Values
            const auto  clearValues = std::vector<vk::ClearValue>{ vk::ClearColorValue(std::array<float,4>{0.f, 0.f, 0.f, 0.0f}), vk::ClearDepthStencilValue(1.0f, 0) };

            // 
            this->secondaryCommand = vkt::createCommandBuffer(*thread, *thread, false, false); // do reference of cmd buffer
            this->secondaryCommand.beginRenderPass(vk::RenderPassBeginInfo(this->context->refRenderPass, this->context->refFramebuffer, renderArea, clearValues.size(), clearValues.data()), vk::SubpassContents::eInline);
            this->secondaryCommand.setViewport(0, { viewport });
            this->secondaryCommand.setScissor(0, { renderArea });
            this->secondaryCommand.bindPipeline(vk::PipelineBindPoint::eGraphics, this->rasterizationState);
            this->secondaryCommand.bindVertexBuffers(0u, buffers, offsets);

            // Make Draw Instanced 
            if (indexType != vk::IndexType::eNoneNV) { // PLC Mode
                this->secondaryCommand.bindIndexBuffer(this->indexData, this->indexData.offset(), this->indexType);
                this->secondaryCommand.drawIndexed(this->indexCount, this->instanceCount, 0u, 0u, 0u);
            } else { // VAL Mode
                this->secondaryCommand.draw(this->vertexCount, this->instanceCount, 0u, 0u);
            };

            // 
            this->secondaryCommand.endRenderPass();
            this->secondaryCommand.end();

            return shared_from_this();
        };

        // Create Or Rebuild Acceleration Structure
        std::shared_ptr<Mesh> createAccelerationStructure(){

            // Pre-Initialize Geometries
            // Use Same Geometry for Sub-Instances
            this->geometries.resize(instanceCount);
            for (uint32_t i = 0u; i < instanceCount; i++) {
                this->geometries[i] = this->geometryTemplate;
                this->geometries[i].geometry.triangles.transformOffset = sizeof(glm::mat3x4) * i + this->gpuTransformData.offset(); // Should To Be Different's
                //geometries[i].geometry.triangles.transformData = gpuTransformData;
            };

            // Re-assign instance count
            this->accelerationStructureInfo.geometryCount = this->geometries.size();
            this->accelerationStructureInfo.pGeometries = this->geometries.data();

            // 
            if (!this->accelerationStructure) { // create acceleration structure fastly...
                this->accelerationStructure = this->driver->getDevice().createAccelerationStructureNV(vkh::VkAccelerationStructureCreateInfoNV{
                    .info = this->accelerationStructureInfo
                });
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
            return shared_from_this();
        };


    protected: friend Mesh; friend Instance; friend Renderer; // GPU Vertex and Attribute Data
        vkt::Vector<uint8_t> indexData = {}; 
        vk::IndexType indexType = vk::IndexType::eNoneNV;
        uint32_t indexCount = 0u, vertexCount = 0u, instanceCount = 1u;

        // 
        std::vector<vkt::Vector<uint8_t>> bindings = {};

        // Accumulated by "Instance" for instanced rendering
        vkt::Vector<glm::mat3x4> gpuTransformData = {};

        // construct bindings and attributes
        uint32_t lastBindID = 0u, locationCounter = 0u;

        // 
        vkh::VsGraphicsPipelineCreateInfoConstruction pipelineInfo = {};
        vkh::VkAccelerationStructureInfoNV accelerationStructureInfo = {};
        std::vector<vkh::VkGeometryNV> geometries = {};

        // 
        vkh::VkGeometryNV geometryTemplate = {};
        vk::CommandBuffer secondaryCommand = {};
        vk::Pipeline rasterizationState = {}; // Vertex Input can changed, so use individual rasterization stages

        // 
        vk::Buffer scratchBuffer = {};

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
