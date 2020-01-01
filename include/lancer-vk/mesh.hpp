#pragma once
#include "./config.hpp"
#include "./driver.hpp"

namespace lancer {

    // WIP Mesh Object
    // Sub-Instances Can Be Supported
    class Mesh : public std::enable_shared_from_this<Mesh> { public: friend Instance;
        Mesh() {
            this->accelerationStructureInfo.geometryCount = instanceCount;
            this->accelerationStructureInfo.pGeometries = &geometryTemplate;
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
                this->vertexCount = this->bindings.back().size() / binding.stride;
                this->geometryTemplate.geometry.triangles.vertexOffset = attribute.offset + this->bindings.back().offset();
                this->geometryTemplate.geometry.triangles.vertexFormat = attribute.format;
                this->geometryTemplate.geometry.triangles.vertexStride = binding.stride;
                this->geometryTemplate.geometry.triangles.vertexCount = this->vertexCount;
                this->geometryTemplate.geometry.triangles.vertexData = this->bindings.back();
            };
            return shared_from_this();
        };

        // Uint32_T version
        std::shared_ptr<Mesh> setIndexData(const vkt::Vector<uint32_t>& indices = {}){
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
        std::shared_ptr<Mesh> setIndexData(const vkt::Vector<uint16_t>& indices = {}){
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
        std::shared_ptr<Mesh> setIndexData(const vkt::Vector<uint8_t>& indices = {}){
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
        std::shared_ptr<Mesh> resetIndexData(){
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
        std::shared_ptr<Mesh> setIndexData(const vkt::Vector<uint8_t>& indices, vk::IndexType type){
            switch(type) {
                case vk::IndexType::eUint32: return this->setIndexData(vkt::Vector<uint32_t>(indices)); break; // uint32_t version 
                case vk::IndexType::eUint16: return this->setIndexData(vkt::Vector<uint16_t>(indices)); break; // uint16_t version
                case vk::IndexType::eUint8EXT: return this->setIndexData(vkt::Vector<uint8_t>(indices)); break; // uint8_t version
                default: return resetIndexData();
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

        // TODO: Rasterization and Ray-Tracing Stages
        // TODO: Instancing Support
        // Create Command With Pipeline
        std::shared_ptr<Mesh> createRasterizeCommand() { // UNIT ONLY!
            std::vector<vk::Buffer> buffers = {};
            std::vector<vk::DeviceSize> offsets = {};
            for (auto& B : bindings) { buffers.push_back(B); offsets.push_back(B.offset()); };
            // TODO: Command Buffer
            //secondaryCommand = vkt::createCommandBuffer(*thread, *thread, false, false); // do reference of cmd buffer
            //secondaryCommand.beginRenderPass(vk::RenderPassBeginInfo(renderPass, framebuffers[currentBuffer].frameBuffer, renderArea, clearValues.size(), clearValues.data()), vk::SubpassContents::eInline);
            //secondaryCommand.setViewport(0, { viewport });
            //secondaryCommand.setScissor(0, { renderArea });

            secondaryCommand.bindPipeline(vk::PipelineBindPoint::eGraphics, rasterizationState);
            secondaryCommand.bindVertexBuffers(0u, buffers, offsets);

            // Make Draw Instanced 
            if (indexType != vk::IndexType::eNoneNV) { // PLC Mode
                secondaryCommand.bindIndexBuffer(indexData, indexData.offset(), indexType);
                secondaryCommand.drawIndexed(indexCount, instanceCount, 0u, 0u, 0u);
            } else { // VAL Mode
                secondaryCommand.draw(vertexCount, instanceCount, 0u, 0u);
            };
            
            //secondaryCommand.endRenderPass();
            //secondaryCommand.end();
            return shared_from_this();
        };

        // Create Or Result Acceleration Structure
        std::shared_ptr<Mesh> createAccelerationStructure(){

            // Pre-Initialize Geometries
            // Use Same Geometry for Sub-Instances
            geometries.resize(instanceCount);
            for (uint32_t i = 0u; i < instanceCount; i++) {
                geometries[i] = geometryTemplate;
                geometries[i].geometry.triangles.transformData = gpuTransformData;
                geometries[i].geometry.triangles.transformOffset = sizeof(glm::mat3x4) * i + gpuTransformData.offset(); // Should To Be Different's
            };

            // Re-assign instance count
            this->accelerationStructureInfo.geometryCount = geometries.size();
            this->accelerationStructureInfo.pGeometries = geometries.data();

            // 

            return shared_from_this();
        };


    protected: // GPU Vertex and Attribute Data
        vkt::Vector<uint8_t> indexData = {}; 
        vk::IndexType indexType = vk::IndexType::eNoneNV; 
        vk::DeviceSize indexCount = 0u;
        vk::DeviceSize vertexCount = 0u;
        uint32_t instanceCount = 1u;

        // 
        std::vector<vkt::Vector<uint8_t>> bindings = {};

        // Accumulated by "Instance" for instanced rendering
        vkt::Vector<glm::mat3x4> gpuTransformData = {};

        // construct bindings and attributes
        uint32_t locationCounter = 0u;
        uint32_t lastBindID = 0u;

        // 
        vkh::VsGraphicsPipelineCreateInfoConstruction pipelineInfo = {};
        vkh::VkAccelerationStructureInfoNV accelerationStructureInfo = {};
        std::vector<vkh::VkGeometryNV> geometries = {};

        // 
        vkh::VkGeometryNV geometryTemplate = {};
        vk::CommandBuffer secondaryCommand = {};
        vk::Pipeline rasterizationState = {}; // Vertex Input can changed, so use individual rasterization stages
        vk::AccelerationStructureNV accelerationStructure = {};
        vk::Buffer scratchBuffer = {};

        // 
        std::shared_ptr<Driver> driver = {};
        std::shared_ptr<Thread> thread = {};
    };

};
