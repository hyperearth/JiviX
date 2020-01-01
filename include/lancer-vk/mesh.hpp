#pragma once
#include "./config.hpp"
#include "./driver.hpp"

namespace lancer {
    
    // WIP Mesh Object
    class Mesh : public std::enable_shared_from_this<Mesh> { public: 
        Mesh() {};
        
    protected: // GPU Vertex and Attribute Data
        vkt::Vector<uint8_t> indiceData = {};
        std::vector<vkt::Vector<uint8_t>> bindings = {};
        vkt::Vector<glm::mat3x4> transformData = {};

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
