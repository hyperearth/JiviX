#pragma once
#include "./config.hpp"
#include "./driver.hpp"

namespace lancer {
    
    // WIP Mesh Object
    class Mesh : public std::enable_shared_from_this<Mesh> { public: friend Instance;
        Mesh() {
            this->accelerationStructureInfo.geometryCount = 1;
            this->accelerationStructureInfo.pGeometries = geometry;
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
                this->geometry.geometry.triangles.vertexOffset = attribute.offset + this->bindings.back().offset();
                this->geometry.geometry.triangles.vertexFormat = attribute.format;
                this->geometry.geometry.triangles.vertexStride = binding.stride;
                this->geometry.geometry.triangles.vertexCount = this->bindings.back().size() / binding.stride;
                this->geometry.geometry.triangles.vertexData = this->bindings.back();
            };
            return shared_from_this();
        };

        // Uint32_T version
        std::shared_ptr<Mesh> setIndexData(const vkt::Vector<uint32_t>& indices = {}){
            this->indexData = indices;
            this->indexType = vk::IndexType::eUint32;
            this->indexCount = indices.size();
            this->geometry.geometry.triangles.indexOffset = indices.offset();
            this->geometry.geometry.triangles.indexType = VkIndexType(this->indexType);
            this->geometry.geometry.triangles.indexCount = indices.size();
            this->geometry.geometry.triangles.indexData = indices;
            return shared_from_this();
        };

        // Uint16_T version
        std::shared_ptr<Mesh> setIndexData(const vkt::Vector<uint16_t>& indices = {}){
            this->indexData = indices;
            this->indexType = vk::IndexType::eUint16;
            this->indexCount = indices.size();
            this->geometry.geometry.triangles.indexOffset = indices.offset();
            this->geometry.geometry.triangles.indexType = VkIndexType(this->indexType);
            this->geometry.geometry.triangles.indexCount = indices.size();
            this->geometry.geometry.triangles.indexData = indices;
            return shared_from_this();
        };

        // 
        std::shared_ptr<Mesh> setDriver(const std::shared_ptr<Driver>& driver = {}){
            this->driver = driver;
            return shared_from_this();
        };

        // 
        std::shared_ptr<Mesh> setTransformData(const vkt::Vector<glm::mat3x4>& transformData = {}) {
            this->geometry.geometry.triangles.transformOffset = transformData.offset();
            this->geometry.geometry.triangles.transformData = transformData;
            this->transformData = transformData;
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
            //rasterizeCommand = vkt::createCommandBuffer(*thread, *thread, false, false); // do reference of cmd buffer
            //rasterizeCommand.beginRenderPass(vk::RenderPassBeginInfo(renderPass, framebuffers[currentBuffer].frameBuffer, renderArea, clearValues.size(), clearValues.data()), vk::SubpassContents::eInline);
            //rasterizeCommand.setViewport(0, { viewport });
            //rasterizeCommand.setScissor(0, { renderArea });
        if (indexType != vk::IndexType::eNoneNV)
            secondaryCommand.bindIndexBuffer(indexData, indexData.offset(), indexType);
            secondaryCommand.bindPipeline(vk::PipelineBindPoint::eGraphics, rasterizationState);
            secondaryCommand.bindVertexBuffers(0u, buffers, offsets);
            secondaryCommand.drawIndexed(indexCount, 1u, 0u, 0u, 0u);
            return shared_from_this();
        };

        // Create Or Result Acceleration Structure
        std::shared_ptr<Mesh> createAccelerationStructure(){
            return shared_from_this();
        };


    protected: // GPU Vertex and Attribute Data
        vkt::Vector<uint8_t> indexData = {}; 
        vk::IndexType indexType = vk::IndexType::eNoneNV; 
        vk::DeviceSize indexCount = 0u;

        // 
        std::vector<vkt::Vector<uint8_t>> bindings = {};
        vkt::Vector<glm::mat3x4> transformData = {};

        // construct bindings and attributes
        uint32_t locationCounter = 0u;
        uint32_t lastBindID = 0u;

        // 
        vkh::VsGraphicsPipelineCreateInfoConstruction pipelineInfo = {};
        vkh::VkAccelerationStructureInfoNV accelerationStructureInfo = {};
        vkh::VkGeometryNV geometry = {};
        //std::vector<vkh::VkGeometryNV> geometries = {};

        // 
        vk::CommandBuffer secondaryCommand = {};
        //vk::DescriptorSet descriptorSet = {};
          vk::Pipeline rasterizationState = {}; // Vertex Input can changed, so use individual rasterization stages
        vk::AccelerationStructureNV accelerationStructure = {};
        vk::Buffer scratchBuffer = {};

        // 
        //std::shared_ptr<Driver> driver = {};
        std::shared_ptr<vkt::GPUFramework> driver = {};
    };

};
