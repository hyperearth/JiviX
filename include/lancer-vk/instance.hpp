#pragma once
#include "./config.hpp"
#include "./driver.hpp"

namespace lancer {

    // WIP Instances
    // ALSO, RAY-TRACING PIPELINES WILL USE NATIVE BINDING AND ATTRIBUTE READERS
    class Instance : public std::enable_shared_from_this<Instance> { public: 
        Instance() {};
        
    protected: // TODO: Attribute and Binding Data Buffers
        vkt::Vector<vkh::VsGeometryInstance> rawInstances = {}; // Ray-Tracing instances Will re-located into meshes by Index, and will no depending by mesh list...
        vkt::Vector<vkh::VsGeometryInstance> gpuInstances = {};
        std::vector<std::shared_ptr<Mesh>> meshes = {}; // Mesh list as Template for instances

        // 
        vkh::VkAccelerationStructureInfoNV accelerationStructureInfo = {};

        // 
        vk::AccelerationStructureNV accelerationStructure = {};
        vk::CommandBuffer drawCommand = {};
        vk::Buffer scratchBuffer = {};

        // 
        std::shared_ptr<vkt::GPUFramework> driver = {};
    };

};
