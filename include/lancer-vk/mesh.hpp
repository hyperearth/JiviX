#pragma once
#include "./config.hpp"
#include "./driver.hpp"

namespace lancer {
    
    // WIP Mesh Object
    class Mesh : public std::enable_shared_from_this<Mesh> { public: 
        Mesh() {
            this->accelerationStructure.geometryCount = 1;
            this->accelerationStructure.geometries = geometry;
            
        };

        // 
        std::shared_ptr<Mesh> addBinding(const vkt::Vector<uint8_t>& vector, const vkh::VkVertexInputBindingDescription& binding = {}){
            this->lastBindID = pipelineInfo.vertexInputBindingDescriptions.size();
            this->pipelineInfo.vertexInputBindingDescriptions.push_back(binding);
            this->pipelineInfo.vertexInputBindingDescriptions.back().binding = lastBindID;
            this->bindings.push_back(vector);
            return shared_from_this();
        };

        // 
        std::shared_ptr<Mesh> addAttribute(const vkh::VkVertexInputAttributeDescription& attribute = {}, const bool& isVertex = false){
            this->pipelineInfo.vertexInputAttributeDescriptions.push_back(attribute);
            this->pipelineInfo.vertexInputAttributeDescriptions.back().binding = lastBindID;
            this->pipelineInfo.vertexInputAttributeDescriptions.back().location = locationCounter++;

            // 
            if (isVertex) { // TODO: fix from vec4 into vec3
                const auto& binding = this->pipelineInfo.vertexInputBindingDescriptions.back();
                this->geometry.geometry.triangles.vertexOffset = attribute.offset + this->bindings.back().offset();
                this->geometry.geometry.triangles.vertexFormat = attribute.format;
                this->geometry.geometry.triangles.vertexStride = binding.stride;
                this->geometry.geometry.triangles.vertexSize = this->bindings.back().size() / binding.stride;
                this->geometry.geometry.triangles.vertexData = this->bindings.back();
            };
            return shared_from_this();
        };

        // Uint32_T version
        std::shared_ptr<Mesh> setIndexData(const vkt::Vector<uint32_t>& indices = {}){
            this->indexData = vk::IndexType::eUint32;
            this->indexType = type;
            this->indexCount = indices.size();
            this->geometry.geometry.triangles.indexoffset = indices.offset();
            this->geometry.geometry.triangles.indexType = vk::IndexType::eUint32;
            this->geometry.geometry.triangles.indexCount = indices.size();
            this->geometry.geometry.triangles.indexData = indices;
            return shared_from_this();
        };

        // Uint16_T version
        std::shared_ptr<Mesh> setIndexData(const vkt::Vector<uint16_t>& indices = {}){
            this->indexData = vk::IndexType::eUint16;
            this->indexType = type;
            this->indexCount = indices.size();
            this->geometry.geometry.triangles.indexoffset = indices.offset();
            this->geometry.geometry.triangles.indexType = vk::IndexType::eUint16;
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
        // Create Command With Pipeline
        std::shared_ptr<Mesh> createRasterizeCommand(){
            
            return shared_from_this();
        };

        // Create Or Result Acceleration Structure
        std::shared_ptr<Mesh> createAccelerationStructure(){
            
            return shared_from_this();
        };


    protected: // GPU Vertex and Attribute Data
        vkt::Vector<uint8_t> indexData = {}; 
        vk::IndexType indexType = vk::IndexType::eUint16; 
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

        // 
        vk::CommandBuffer rasterizeCommand = {};
        vk::DescriptorSet descriptorSet = {};
        vk::Pipeline rasterizationState = {}; // Vertex Input can changed, so use individual rasterization stages

        // 
        vkh::VkGeometryNV geometry = {};
        vkh::VkAccelerationStructureNV accelerationStructure = {};
        std::vector<vkh::VkGeometryNV> geometries = {};

        // 
        std::shared_ptr<Driver> driver = {};
    };

};
