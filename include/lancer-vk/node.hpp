#pragma once // #
#include "./config.hpp"
#include "./driver.hpp"
#include "./thread.hpp"
#include "./mesh.hpp"

namespace lancer {

    // WIP Instances
    // ALSO, RAY-TRACING PIPELINES WILL USE NATIVE BINDING AND ATTRIBUTE READERS
    class Node : public std::enable_shared_from_this<Node> { public: friend Renderer;
        Node(const std::shared_ptr<Context>& context) {
            this->driver = context->getDriver();
            this->thread = std::make_shared<Thread>(this->driver);
            this->context = context;

            // 
            this->accelerationStructureInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_NV;
            this->accelerationStructureInfo.instanceCount = 1u;
            this->accelerationStructureInfo.geometryCount = 0u;

            // DEATH POINT FIXED 
            this->rawInstances = vkt::Vector<vkh::VsGeometryInstance>(std::make_shared<vkt::VmaBufferAllocation>(this->driver->getAllocator(), vkh::VkBufferCreateInfo{ .size = sizeof(vkh::VsGeometryInstance)*64u, .usage = { .eTransferSrc = 1, .eStorageBuffer = 1, .eRayTracing = 1 } }, VMA_MEMORY_USAGE_CPU_TO_GPU));
            this->gpuInstances = vkt::Vector<vkh::VsGeometryInstance>(std::make_shared<vkt::VmaBufferAllocation>(this->driver->getAllocator(), vkh::VkBufferCreateInfo{ .size = sizeof(vkh::VsGeometryInstance)*64u, .usage = { .eTransferDst = 1, .eStorageBuffer = 1, .eRayTracing = 1 } }, VMA_MEMORY_USAGE_GPU_ONLY));

            // 
            this->gpuMeshInfo = vkt::Vector<glm::uvec4>(std::make_shared<vkt::VmaBufferAllocation>(this->driver->getAllocator(), vkh::VkBufferCreateInfo{ .size = 16u*64u, .usage = { .eTransferDst = 1, .eUniformBuffer = 1, .eStorageBuffer = 1, .eRayTracing = 1 } }, VMA_MEMORY_USAGE_GPU_ONLY));
        };

        // 
        std::shared_ptr<Node> setContext(const std::shared_ptr<Context>& context) {
            this->context = context;
            return shared_from_this();
        };

        // 
        std::shared_ptr<Node> setThread(const std::shared_ptr<Thread>& thread) {
            this->thread = thread;
            return shared_from_this();
        };

        // 
        std::shared_ptr<Node> setRawInstance(const vkt::Vector<vkh::VsGeometryInstance>& rawInstances = {}, const uint32_t& instanceCounter = 0u) {
            this->rawInstances = rawInstances; 
            this->instanceCounter = instanceCounter;
            this->mapMeshes.resize(instanceCounter);
            return shared_from_this();
        };

        // 
        std::shared_ptr<Node> setGpuInstance(const vkt::Vector<vkh::VsGeometryInstance>& gpuInstances = {}) {
            this->gpuInstances = gpuInstances;
            return shared_from_this();
        };

        // 
        std::shared_ptr<Node> pushInstance(const vkh::VsGeometryInstance& instance = {}) {
            const auto instanceID = this->instanceCounter++;
            const uint32_t meshID = instance.instanceId;
            this->rawInstances[instanceID] = instance;
            //this->rawInstances[instanceID].instanceId = meshID; // Customize Per Mesh
            this->mapMeshes.push_back(meshID);
            if (this->meshes[meshID]->accelerationStructure) {
                this->driver->getDevice().getAccelerationStructureHandleNV(this->meshes[meshID]->accelerationStructure, 8ull, &this->rawInstances[instanceID].accelerationStructureHandle, this->driver->getDispatch());
            };
            return shared_from_this();
        };

        // 
        std::shared_ptr<Node> mapMeshData() {
            for (uint32_t i = 0; i < this->mapMeshes.size(); i++) {
                this->driver->getDevice().getAccelerationStructureHandleNV(this->meshes[this->mapMeshes[i]]->accelerationStructure, sizeof(uint64_t), &this->rawInstances[i].accelerationStructureHandle, this->driver->getDispatch());
                //std::cout << this->rawInstances[i].accelerationStructureHandle << std::endl;
            };
            return shared_from_this();
        };

        // Push Mesh "Template" For Any Other Instances
        uintptr_t pushMesh(const std::shared_ptr<Mesh>& mesh = {}) {
            const uintptr_t ptr = this->meshes.size();
            this->meshes.push_back(mesh); return ptr;
        };

        // 
        std::shared_ptr<Node> createDescriptorSet() { // 
            this->bindingsDescriptorSetInfo = vkh::VsDescriptorSetCreateInfoHelper(this->context->bindingsDescriptorSetLayout, this->thread->getDescriptorPool());
            this->meshDataDescriptorSetInfo = vkh::VsDescriptorSetCreateInfoHelper(this->context->meshDataDescriptorSetLayout, this->thread->getDescriptorPool());

            // plush descriptor set bindings (i.e. buffer bindings array, every have array too)
            const auto bindingCount = 8u;
            const auto meshCount = std::min(this->meshes.size(), 64ull);
            for (uint32_t j=0;j<bindingCount;j++) {
                //auto& handle = this->meshDataDescriptorSetInfo.pushDescription(vkh::VkDescriptorUpdateTemplateEntry{
                //    .dstBinding = j,
                //    .dstArrayElement = 0u,
                //    .descriptorCount = uint32_t(meshCount),
                //    .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER
                //});

                //for (uint32_t i=0;i<meshCount;i++) { if (j < this->meshes[i]->bindings.size() && this->meshes[i]->bindings[j].has()) {
                //    handle.offset<vk::BufferView>(i) = this->meshes[i]->bindings[j].createBufferView(vk::Format::eR8Uint);
                //}};

                for (uint32_t i = 0; i < meshCount; i++) {
                    if (j < this->meshes[i]->bindings.size() && this->meshes[i]->bindings[j].has()) {
                        this->meshDataDescriptorSetInfo.pushDescription(vkh::VkDescriptorUpdateTemplateEntry{
                            .dstBinding = j,
                            .dstArrayElement = i,
                            .descriptorCount = 1u,
                            .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER
                        }).offset<vk::BufferView>(0u) = this->meshes[i]->bindings[j].createBufferView(vk::Format::eR8Uint);
                    };
                };
            };

            { // [0] Plush Index Data (vk::BufferView)
                auto& handle = this->meshDataDescriptorSetInfo.pushDescription(vkh::VkDescriptorUpdateTemplateEntry{
                    .dstBinding = 8u,
                    .dstArrayElement = 0u,
                    .descriptorCount = uint32_t(meshCount),
                    .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER
                });

                for (uint32_t i = 0; i < meshCount; i++) {
                    if (this->meshes[i]->indexData.has()) {
                        handle.offset<vk::BufferView>(i) = this->meshes[i]->indexData.createBufferView(vk::Format::eR8Uint);
                    };
                };
            };

            { // [1] plush bindings
                auto& handle = this->bindingsDescriptorSetInfo.pushDescription(vkh::VkDescriptorUpdateTemplateEntry{
                    .dstBinding = 0u,
                    .dstArrayElement = 0u,
                    .descriptorCount = uint32_t(meshCount),
                    .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER
                });

                for (uint32_t i = 0; i < meshCount; i++) {
                    handle.offset<vkh::VkDescriptorBufferInfo>(i) = this->meshes[i]->gpuBindings;
                };
            };

            { // [2] plush attributes
                auto& handle = this->bindingsDescriptorSetInfo.pushDescription(vkh::VkDescriptorUpdateTemplateEntry{
                    .dstBinding = 1u,
                    .dstArrayElement = 0u,
                    .descriptorCount = uint32_t(meshCount),
                    .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER
                });

                for (uint32_t i = 0; i < meshCount; i++) {
                    handle.offset<vkh::VkDescriptorBufferInfo>(i) = this->meshes[i]->gpuAttributes;
                };
            };

            // [3] acceleration structure
            if (this->accelerationStructure) {
                this->bindingsDescriptorSetInfo.pushDescription(vkh::VkDescriptorUpdateTemplateEntry{
                    .dstBinding = 2u,
                    .descriptorCount = 1u,
                    .descriptorType = VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_NV
                }).offset<vk::AccelerationStructureNV>(0u) = this->accelerationStructure;
            };

            // [4] plush uniforms 
            this->bindingsDescriptorSetInfo.pushDescription(vkh::VkDescriptorUpdateTemplateEntry{
                .dstBinding = 3u,
                .descriptorCount = 1u,
                .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER
            }).offset<vkh::VkDescriptorBufferInfo>(0u) = this->context->uniformGPUData;

            // [5] Mesh Data Info (Has Indices, Material ID, etc.)
            this->bindingsDescriptorSetInfo.pushDescription(vkh::VkDescriptorUpdateTemplateEntry{
                .dstBinding = 6u,
                .descriptorCount = 1u,
                .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER
            }).offset<vkh::VkDescriptorBufferInfo>(0u) = this->gpuInstances;

            { // [6] Push Transform Data Instanced Into... 
                //auto& handle = this->bindingsDescriptorSetInfo.pushDescription(vkh::VkDescriptorUpdateTemplateEntry{
                //    .dstBinding = 4u,
                //    .dstArrayElement = 0u,
                //    .descriptorCount = uint32_t(meshCount),
                //    .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER
                //});

                //for (uint32_t i = 0; i < meshCount; i++) {
                //    if (this->meshes[i]->gpuTransformData.has()) {
                //        handle.offset<vkh::VkDescriptorBufferInfo>(i) = this->meshes[i]->gpuTransformData;
                //    };
                //};

                for (uint32_t i = 0; i < meshCount; i++) {
                    if (this->meshes[i]->gpuTransformData.has()) {
                        this->bindingsDescriptorSetInfo.pushDescription(vkh::VkDescriptorUpdateTemplateEntry{
                            .dstBinding = 4u,
                            .dstArrayElement = i,
                            .descriptorCount = 1u,
                            .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER
                        }).offset<vkh::VkDescriptorBufferInfo>(0u) = this->meshes[i]->gpuTransformData;
                    };
                };
            };

            { // [7] Mesh Data Info (Has Indices, Material ID, etc.)
                this->bindingsDescriptorSetInfo.pushDescription(vkh::VkDescriptorUpdateTemplateEntry{
                    .dstBinding = 5u,
                    .dstArrayElement = 0u,
                    .descriptorCount = 1u,//uint32_t(meshCount),
                    .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER
                }).offset<vkh::VkDescriptorBufferInfo>(0u) = gpuMeshInfo;

                //for (uint32_t i = 0; i < meshCount; i++) {
                //    handle.offset<vkh::VkDescriptorBufferInfo>(i) = gpuMeshInfo;
                //};
            };

            { // [8] 
                auto& handle = this->bindingsDescriptorSetInfo.pushDescription(vkh::VkDescriptorUpdateTemplateEntry{
                    .dstBinding = 7u,
                    .dstArrayElement = 0u,
                    .descriptorCount = uint32_t(meshCount),
                    .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER
                });
                for (uint32_t i = 0; i < meshCount; i++) {
                    handle.offset<vkh::VkDescriptorBufferInfo>(i) = this->meshes[i]->gpuInstanceMap;
                };
            };

            // 
            driver->getDevice().updateDescriptorSets(vkt::vector_cast<vk::WriteDescriptorSet,vkh::VkWriteDescriptorSet>(this->meshDataDescriptorSetInfo.setDescriptorSet(
                this->context->descriptorSets[0] = (this->meshDataDescriptorSet = driver->getDevice().allocateDescriptorSets(this->meshDataDescriptorSetInfo)[0])
            )),{});

            // 
            driver->getDevice().updateDescriptorSets(vkt::vector_cast<vk::WriteDescriptorSet,vkh::VkWriteDescriptorSet>(this->bindingsDescriptorSetInfo.setDescriptorSet(
                this->context->descriptorSets[1] = (this->bindingsDescriptorSet = driver->getDevice().allocateDescriptorSets(this->bindingsDescriptorSetInfo)[0])
            )),{});

            // remap mesh data
            this->mapMeshData();

            // 
            return shared_from_this();
        };

        // 
        std::shared_ptr<Node> buildAccelerationStructure(const vk::CommandBuffer& buildCommand = {}) {
            if (!this->accelerationStructure) { this->createAccelerationStructure(); };
            buildCommand.copyBuffer(this->rawInstances, this->gpuInstances, { vkh::VkBufferCopy{ .srcOffset = this->rawInstances.offset(), .dstOffset = this->gpuInstances.offset(), .size = this->gpuInstances.range() } });

            for (uint32_t i = 0; i < this->meshes.size(); i++) {
                auto& mesh = this->meshes[i];
                buildCommand.copyBuffer(mesh->rawMeshInfo, this->gpuMeshInfo, { vk::BufferCopy{ mesh->rawMeshInfo.offset(), this->gpuMeshInfo.offset() + mesh->rawMeshInfo.range() * i, mesh->rawMeshInfo.range() } });
            };

            vkt::commandBarrier(buildCommand);
            buildCommand.buildAccelerationStructureNV((vk::AccelerationStructureInfoNV&)this->accelerationStructureInfo,this->gpuInstances,this->gpuInstances.offset(),this->needsUpdate,this->accelerationStructure,{},this->gpuScratchBuffer,this->gpuScratchBuffer.offset(), this->driver->getDispatch());
            vkt::commandBarrier(buildCommand);
            this->needsUpdate = true; return shared_from_this();
        };

        // Create Or Rebuild Acceleration Structure
        std::shared_ptr<Node> createAccelerationStructure() { // Re-assign instance count
            this->accelerationStructureInfo.instanceCount = instanceCounter;
            this->accelerationStructureInfo.flags = VK_BUILD_ACCELERATION_STRUCTURE_ALLOW_UPDATE_BIT_NV | VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_NV;
            this->accelerationStructureInfo.geometryCount = 0u;

            // 
            if (!this->accelerationStructure) { // create acceleration structure fastly...
                this->accelerationStructure = this->driver->getDevice().createAccelerationStructureNV(vkh::VkAccelerationStructureCreateInfoNV{
                    .info = this->accelerationStructureInfo
                }, nullptr, this->driver->getDispatch());
                
                //
                auto requirements = this->driver->getDevice().getAccelerationStructureMemoryRequirementsNV(vkh::VkAccelerationStructureMemoryRequirementsInfoNV{
                    .type = VK_ACCELERATION_STRUCTURE_MEMORY_REQUIREMENTS_TYPE_OBJECT_NV,
                    .accelerationStructure = this->accelerationStructure
                }, this->driver->getDispatch());
                
                // 
                VmaAllocationCreateInfo allocInfo = {};
                allocInfo.memoryTypeBits |= requirements.memoryRequirements.memoryTypeBits;
                vmaAllocateMemory(this->driver->getAllocator(),&(VkMemoryRequirements&)requirements.memoryRequirements,&allocInfo,&this->allocation,&this->allocationInfo);
                
                // 
                this->driver->getDevice().bindAccelerationStructureMemoryNV({vkh::VkBindAccelerationStructureMemoryInfoNV{
                    .accelerationStructure = this->accelerationStructure,
                    .memory = this->allocationInfo.deviceMemory,
                    .memoryOffset = this->allocationInfo.offset
                }}, this->driver->getDispatch());
            };

            // 
            if (!this->gpuScratchBuffer.has()) { // 
                auto requirements = this->driver->getDevice().getAccelerationStructureMemoryRequirementsNV(vkh::VkAccelerationStructureMemoryRequirementsInfoNV{
                    .type = VK_ACCELERATION_STRUCTURE_MEMORY_REQUIREMENTS_TYPE_BUILD_SCRATCH_NV,
                    .accelerationStructure = this->accelerationStructure
                }, this->driver->getDispatch());

                // 
                this->gpuScratchBuffer = vkt::Vector<uint8_t>(std::make_shared<vkt::VmaBufferAllocation>(driver->getAllocator(), vkh::VkBufferCreateInfo{
                    .size = requirements.memoryRequirements.size,
                    .usage = { .eStorageBuffer = 1, .eRayTracing = 1 }
                }, VMA_MEMORY_USAGE_GPU_ONLY));
            };

            // 
            this->mapMeshData();

            // 
            return shared_from_this();
        };

    protected: // 
        std::vector<std::shared_ptr<Mesh>> meshes = {}; // Mesh list as Template for Instances
        std::vector<uint32_t> mapMeshes = {};

        // 
        vkt::Vector<vkh::VsGeometryInstance> rawInstances = {}; // Ray-Tracing instances Will re-located into meshes by Index, and will no depending by mesh list...
        vkt::Vector<vkh::VsGeometryInstance> gpuInstances = {};
        uint32_t instanceCounter = 0u;
        bool needsUpdate = false;

        // 
        vkh::VsDescriptorSetCreateInfoHelper meshDataDescriptorSetInfo = {};
        vkh::VsDescriptorSetCreateInfoHelper bindingsDescriptorSetInfo = {};
        vkh::VkAccelerationStructureInfoNV accelerationStructureInfo = {};

        // 
        vkt::Vector<glm::uvec4> gpuMeshInfo = {};

        // 
        vk::DescriptorSet meshDataDescriptorSet = {};
        vk::DescriptorSet bindingsDescriptorSet = {};
        vk::AccelerationStructureNV accelerationStructure = {};
        vkt::Vector<uint8_t> gpuScratchBuffer = {};
        VmaAllocationInfo allocationInfo = {};
        VmaAllocation allocation = {};
        
        // 
        std::shared_ptr<Driver> driver = {};
        std::shared_ptr<Thread> thread = {};
        std::shared_ptr<Context> context = {};
    };

};
