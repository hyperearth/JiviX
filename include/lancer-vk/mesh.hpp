#pragma once
#include "./config.hpp"

namespace lancer {
    
    // WIP Mesh Object
    class Mesh : public std::enable_shared_from_this<Mesh> { public: 
        
        
    protected: // Raw and GPU Data

        // GPU Vertex and Attribute Data
        vkt::Vector<uint8_t> indiceData = {};
        vkt::Vector<uint8_t> vertexData = {};

        // 
        vkt::Vector<uint8_t>     rawData = {};
        vkt::Vector<glm::mat3x4> transformData = {};

        // 
        std::shared_ptr<vkt::VmaBufferAllocation> gpuRawData = {};
        std::shared_ptr<vkt::VmaBufferAllocation> gpuTransformData = {};

        // 
        vkh::VsGraphicsPipelineCreateInfoConstruction pipelineInfo = {};
        vkh::VkAccelerationStructureInfoNV accelerationStructureInfo = {};

        // 
        vk::CommandBuffer rasterizeCommand = {};
        vk::DescriptorSet descriptorSet = {};
        vk::Pipeline rasterizationState = {}; // Vertex Input can changed, so use individual rasterization stages

        // 
        vk::AccelerationStructureNV accelerationStructure = {};
        std::vector<vkh::VkGeometryNV> geometries = {};

        // 
        std::shared_ptr<Driver> driver = {};
    };

};
