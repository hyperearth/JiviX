#pragma once
#include "./config.hpp"
#include "./driver.hpp"

namespace lancer {

    // WIP Instances
    // ALSO, RAY-TRACING PIPELINES WILL USE NATIVE BINDING AND ATTRIBUTE READERS
    class Instance : public std::enable_shared_from_this<Instance> { public: 
        Instance() {};

        // 
        std::shared_ptr<Instance> setRawInstance(const vkt::Vector<vkh::VsGeometryInstance>& rawInstances = {}, const uint32_t& instanceCounter = 0u) {
            this->rawInstances = rawInstances; this->instanceCounter = instanceCounter;
            return shared_from_this();
        };

        // 
        std::shared_ptr<Instance> setGpuInstance(const vkt::Vector<vkh::VsGeometryInstance>& gpuInstances = {}) {
            this->gpuInstances = gpuInstances;
            return shared_from_this();
        };

        // 
        std::shared_ptr<Instance> pushInstance(const vkh::VsGeometryInstance& instance = {}) {
            this->rawInstances[this->instanceCounter++] = instance;
            return shared_from_this();
        };

        // 
        uintptr_t pushMesh(const std::shared_ptr<Mesh>& mesh = {}) {
            const uintptr_t ptr = this->meshes.size();
            this->meshes.push_back(mesh);
            return ptr;
        };

        // 
        std::shared_ptr<Instance> describeMeshBindings() {
            // Polyfill Geometry Bindings
            // TODO: Attributes Support

            const uint32_t bindingCount = 4u;
            for (uint32_t i=0;i<bindingCount;i++) {
                auto& handle = handles.push_back(descriptorSetInfo.pushDescription(vkh::VkDescriptorUpdateTemplateEntry{
                    .dstBinding = i,
                    .dstArrayElement = 1u,
                    .descriptorCount = meshes.size(),
                    .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER
                }));
                for (uint32_t i=0;i<meshes.size();i++) {
                    handle.offset<vkh::VkDescriptorBufferInfo>(i) = meshes[i].bindings[j];
                };
            };

            return shared_from_this();
        };

               // TODO: Build Acceleration Structure 
    protected: // TODO: Attribute and Binding Data Buffers
        std::vector<std::shared_ptr<Mesh>> meshes = {}; // Mesh list as Template for Instances

        // 
        vkt::Vector<vkh::VsGeometryInstance> rawInstances = {}; // Ray-Tracing instances Will re-located into meshes by Index, and will no depending by mesh list...
        vkt::Vector<vkh::VsGeometryInstance> gpuInstances = {};
        uint32_t instanceCounter = 0u;

        // 
        vkh::VsDescriptorSetCreateInfoHelper descriptorSetInfo = {};
        vkh::VkAccelerationStructureInfoNV accelerationStructureInfo = {};

        // 
        vk::CommandBuffer drawCommand = {};
        vk::DescriptorSet descriptorSet = {};
        vk::AccelerationStructureNV accelerationStructure = {};
        vk::Buffer scratchBuffer = {};

        // 
        std::shared_ptr<Driver> driver = {};
        std::shared_ptr<Thread> thread = {};
    };

};
