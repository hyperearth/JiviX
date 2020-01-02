#pragma once
#include "./config.hpp"
#include "./driver.hpp"

namespace lancer {

    // WIP Instances
    // ALSO, RAY-TRACING PIPELINES WILL USE NATIVE BINDING AND ATTRIBUTE READERS
    class Instance : public std::enable_shared_from_this<Instance> { public: 
        Instance() {
            this->accelerationStructureInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_NV;
            this->accelerationStructureInfo.instanceCount = 1u;
        };

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
        std::shared_ptr<Instance> pushInstance(const vkh::VsGeometryInstance& instance = {}, const uintptr_t meshID = 0ull) {
            const auto instanceID = this->instanceCounter++;
            this->rawInstances[instanceID] = instance;
            this->driver->getDevice().getAccelerationStructureHandleNV(this->meshes[meshID].accelerationStructure, 8ull, &this->rawInstances[instanceID].accelerationStructureHandle);
            return shared_from_this();
        };

        // Push Mesh "Template" For Any Other Instances
        uintptr_t pushMesh(const std::shared_ptr<Mesh>& mesh = {}) {
            const uintptr_t ptr = this->meshes.size();
            this->meshes.push_back(mesh); return ptr;
        };

        // 
        std::shared_ptr<Instance> describeMeshBindings() {
            // plush descriptor set bindings (i.e. buffer bindings array, every have array too)
            const uint32_t bindingCount = 4u;
            for (uint32_t i=0;i<bindingCount;i++) {
                auto& handle = descriptorSetInfo.pushDescription(vkh::VkDescriptorUpdateTemplateEntry{
                    .dstBinding = i,
                    .descriptorCount = meshes.size(),
                    .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER
                });
                for (uint32_t i=0;i<meshes.size();i++) {
                    handle.offset<vkh::VkDescriptorBufferInfo>(i) = meshes[i].bindings[j];
                };
            };

            // plush bindings
            auto bindingSet = bindingDescriptorSet.pushDescription(vkh::VkDescriptorUpdateTemplateEntry{
                .dstBinding = 0u,
                .descriptorCount = meshes.size(),
                .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER
            });

            // plush attributes
            auto attributeSet = bindingDescriptorSet.pushDescription(vkh::VkDescriptorUpdateTemplateEntry{
                .dstBinding = 1u,
                .descriptorCount = meshes.size(),
                .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER
            });

            // plush into descriptor sets
            for (uint32_t i=0;i<meshes.size();i++) {
                bindingSet.offset<vkh::VkDescriptorBufferInfo>(i) = meshes[i].gpuBindingBuffer;
                attributeSet.offset<vkh::VkDescriptorBufferInfo>(i) = meshes[i].gpuAttributeBuffer;
            };

            // 
            return shared_from_this();
        };

        //  
        std::shared_ptr<Instance> buildAccelerationStructure() {
            this->buildCommand = vkt::createCommandBuffer(*thread, *thread, false, false);
            this->buildCommand.buildAccelerationStructureNV(this->accelerationStructureInfo,this->gpuInstances,this->gpuInstances.offset(),needsUpdate,this->accelerationStructure,{},this->gpuScratchBuffer,this->gpuScratchBuffer.offset());
            this->buildCommand.end();
            return shared_from_this();
        };

        // Create Or Rebuild Acceleration Structure
        std::shared_ptr<Instance> createAccelerationStructure() {

            // Re-assign instance count
            this->accelerationStructureInfo.instanceCount = instanceCounter;

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
                this->gpuScratchBuffer = vkt::Vector<uint8_t>(std::make_shared<vkt::VmaBufferAllocation>(fw.getAllocator(), vkh::VkBufferCreateInfo{
                    .size = requirements.memoryRequirements.size,
                    .usage = { .eStorageBuffer = 1, .eRayTracing = 1 }
                }, VMA_MEMORY_USAGE_GPU_ONLY));
            };

            // 
            return shared_from_this();
        };

    protected: // 
        std::vector<std::shared_ptr<Mesh>> meshes = {}; // Mesh list as Template for Instances

        // 
        vkt::Vector<vkh::VsGeometryInstance> rawInstances = {}; // Ray-Tracing instances Will re-located into meshes by Index, and will no depending by mesh list...
        vkt::Vector<vkh::VsGeometryInstance> gpuInstances = {};
        uint32_t instanceCounter = 0u;
        bool needsUpdate = false;

        // 
        vkh::VsDescriptorSetCreateInfoHelper descriptorSetInfo = {};
        vkh::VkAccelerationStructureInfoNV accelerationStructureInfo = {};

        // 
        vk::CommandBuffer buildCommand = {};
        vk::DescriptorSet descriptorSet = {};
        vk::DescriptorSet bindingDescriptorSet = {};
        vk::AccelerationStructureNV accelerationStructure = {};
        vkt::Vector<uint8_t> gpuScratchBuffer = {};

        // 
        std::shared_ptr<Driver> driver = {};
        std::shared_ptr<Thread> thread = {};
    };

};
