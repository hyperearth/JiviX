#pragma once
#include "./config.hpp"

namespace lancer {

    // WIP Mesh Object
    class Mesh : public std::enable_shared_from_this<Mesh> { public: 
        
        
    protected: // 
        vkh::VsGraphicsPipelineCreateInfoConstruction pipelineInfo = {};
        vkh::VkAccelerationStructureInfoNV accelerationStructureInfo = {};
        std::vector<vkh::VkGeometryNV> geometries = {};
        vk::DescriptorSet descriptorSet = {};
        vk::Pipeline rasterizationState = {};
        vk::AccelerationStructureNV accelerationStructure = {};
        vk::CommandBuffer rasterizeCommand = {};
    };

};
