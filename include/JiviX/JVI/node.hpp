#pragma once // #

#include "./config.hpp"
#include "./driver.hpp"
#include "./thread.hpp"
#include "./mesh-binding.hpp"

namespace jvi {

    // WIP Instances
    // ALSO, RAY-TRACING PIPELINES WILL USE NATIVE BINDING AND ATTRIBUTE READERS
    class Node : public std::enable_shared_from_this<Node> { public: friend Renderer;
        Node() {};
        Node(const vkt::uni_ptr<Context>& context, const uint32_t& MaxInstanceCount = 64u) : context(context), MaxInstanceCount(MaxInstanceCount) { this->construct(); };
        Node(const std::shared_ptr<Context>& context, const uint32_t& MaxInstanceCount = 64u) : context(context), MaxInstanceCount(MaxInstanceCount) { this->construct(); };
        ~Node() {};

        // 
        virtual vkt::uni_ptr<Node> sharedPtr() { return shared_from_this(); };
        //virtual vkt::uni_ptr<Node> sharedPtr() const { return std::shared_ptr<Node>(shared_from_this()); };

        //
        virtual uPTR(Node) construct() {
            this->driver = context->getDriver();
            this->thread = std::make_shared<Thread>(this->driver);
            this->context = context;

            // 
            this->topCreate.type = VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR;

            // 
            this->rawInstances = vkt::Vector<vkh::VsGeometryInstance>(std::make_shared<vkt::VmaBufferAllocation>(this->driver->getAllocator(), vkh::VkBufferCreateInfo{ .size = sizeof(vkh::VsGeometryInstance) * MaxInstanceCount, .usage = {.eTransferSrc = 1, .eStorageBuffer = 1, .eRayTracing = 1 } }, VMA_MEMORY_USAGE_CPU_TO_GPU));
            this->gpuInstances = vkt::Vector<vkh::VsGeometryInstance>(std::make_shared<vkt::VmaBufferAllocation>(this->driver->getAllocator(), vkh::VkBufferCreateInfo{ .size = sizeof(vkh::VsGeometryInstance) * MaxInstanceCount, .usage = {.eTransferDst = 1, .eStorageBuffer = 1, .eRayTracing = 1, .eSharedDeviceAddress = 1 } }));

            // 
            this->gpuMeshInfo = vkt::Vector<glm::uvec4>(std::make_shared<vkt::VmaBufferAllocation>(this->driver->getAllocator(), vkh::VkBufferCreateInfo{ .size = 16u * MaxInstanceCount, .usage = { .eTransferDst = 1, .eUniformBuffer = 1, .eStorageBuffer = 1, .eRayTracing = 1 } }, VMA_MEMORY_USAGE_GPU_ONLY));

            // FOR BUILD! 
            this->instancHeadInfo = vkh::VkAccelerationStructureBuildGeometryInfoKHR{};
            this->instancHeadInfo.geometryCount = this->instancInfo.size();
            this->instancHeadInfo.ppGeometries = reinterpret_cast<vkh::VkAccelerationStructureGeometryKHR**>((this->instancPtr = this->instancInfo.data()).ptr());
            this->instancHeadInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR;
            this->instancHeadInfo.flags = VK_BUILD_ACCELERATION_STRUCTURE_ALLOW_UPDATE_BIT_KHR | VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR;
            this->instancHeadInfo.geometryArrayOfPointers = true;

            // FOR BUILD! // originally, it should to be array (like as old version of LancER)
            this->instancInfo[0u] = vkh::VkAccelerationStructureGeometryKHR{ };
            this->instancInfo[0u] = vkh::VkAccelerationStructureGeometryInstancesDataKHR{ .arrayOfPointers = false, .data = this->gpuInstances };
            this->offsetsInfo[0u] = vkh::VkAccelerationStructureBuildOffsetInfoKHR{
                .primitiveCount = 0u, // How many instances used... 
                .primitiveOffset = 0u, // Where read on buffer...
                .firstVertex = 0u, // What is first instance defined...
                .transformOffset = 0u // WTF?..
            };

            // FOR CREATE!
            this->topDataCreate[0u].geometryType = this->instancInfo[0u].geometryType;
            this->topDataCreate[0u].maxPrimitiveCount = static_cast<uint32_t>(MaxInstanceCount);
            this->topDataCreate[0u].maxVertexCount = 0u;
            this->topDataCreate[0u].indexType = VK_INDEX_TYPE_NONE_KHR;
            this->topDataCreate[0u].vertexFormat = VK_FORMAT_UNDEFINED;
            this->topDataCreate[0u].allowsTransforms = true;

            // FOR CREATE!
            this->topCreate.maxGeometryCount = this->topDataCreate.size();
            this->topCreate.pGeometryInfos = this->topDataCreate.data();
            this->topCreate.type = this->instancHeadInfo.type;
            this->topCreate.flags = this->instancHeadInfo.flags;

            // 
            return uTHIS;
        };

        // 
        virtual uPTR(Node) setContext(const vkt::uni_ptr<Context>& context) {
            this->context = context;
            return uTHIS;
        };

        // 
        virtual uPTR(Node) setThread(const vkt::uni_ptr<Thread>& thread) {
            this->thread = thread;
            return uTHIS;
        };

        // 
        virtual uPTR(Node) setRawInstance(const vkt::Vector<vkh::VsGeometryInstance>& rawInstances = {}, const uint32_t& instanceCounter = 0u) {
            this->rawInstances = rawInstances; 
            this->instanceCounter = instanceCounter;
            this->mapMeshes.resize(instanceCounter);
            return uTHIS;
        };

        // 
        virtual uPTR(Node) setGpuInstance(const vkt::Vector<vkh::VsGeometryInstance>& gpuInstances = {}) {
            this->gpuInstances = gpuInstances;
            return uTHIS;
        };

        // 
        virtual uPTR(Node) pushInstance(vkt::uni_arg<vkh::VsGeometryInstance> instance = vkh::VsGeometryInstance{}) {
            if (this->meshes[instance->instanceId] && this->meshes[instance->instanceId]->fullGeometryCount > 0) {
                const auto instanceID = this->instanceCounter++;
                const auto meshID = instance->instanceId;
                this->rawInstances[instanceID] = instance; // List Of Instances

                // Add Instance ID into Mesh
                this->mapMeshes.push_back(meshID);
                this->meshes[this->mapMeshes.back()]->linkWithInstance(instanceID);
            };
            return uTHIS;
        };

        // 
        virtual uPTR(Node) mapMeshData() {
            for (uint32_t i = 0; i < this->mapMeshes.size(); i++) {
                if (this->meshes[this->mapMeshes[i]]->fullGeometryCount > 0 && this->meshes[this->mapMeshes[i]]->mapCount > 0) {
                    this->rawInstances[i].accelerationStructureHandle = this->driver->getDevice().getAccelerationStructureAddressKHR(this->meshes[this->mapMeshes[i]]->accelerationStructure, this->driver->getDispatch());
                };
            };
            return uTHIS;
        };

        // Push Mesh "Template" For Any Other Instances
        virtual uintptr_t pushMesh(const vkt::uni_ptr<MeshBinding>& mesh = {}) {
            const uintptr_t ptr = this->meshes.size();
            this->meshes.push_back(mesh); return ptr;
        };

        // 
        virtual uintptr_t pushMesh(const std::shared_ptr<MeshBinding>& mesh) { return this->pushMesh(vkt::uni_ptr<MeshBinding>(mesh)); };

        /*
        // WARNING!!! NOT RECOMMENDED! 
        [[deprecated]]
        virtual uintptr_t pushMesh(MeshBinding* mesh) {
            const uintptr_t ptr = this->meshes.size();
            this->meshes.push_back(vkt::uni_ptr<MeshBinding>(mesh)); return ptr;
        };

        // WARNING!!! NOT RECOMMENDED! 
        [[deprecated]]
        virtual uintptr_t pushMesh(MeshBinding& mesh) {
            const uintptr_t ptr = this->meshes.size();
            this->meshes.push_back(vkt::uni_ptr<MeshBinding>(&mesh)); return ptr;
        };
        */

        // 
        virtual uPTR(Node) createDescriptorSet() { // 
            this->bindingsDescriptorSetInfo = vkh::VsDescriptorSetCreateInfoHelper(this->context->bindingsDescriptorSetLayout, this->thread->getDescriptorPool());
            this->meshDataDescriptorSetInfo = vkh::VsDescriptorSetCreateInfoHelper(this->context->meshDataDescriptorSetLayout, this->thread->getDescriptorPool());

            // plush descriptor set bindings (i.e. buffer bindings array, every have array too)
            const auto bindingCount = 1u; //8u;
            const auto meshCount = std::min(this->meshes.size(), 64ull);
            for (uint32_t j=0;j<bindingCount;j++) {
                auto& handle = this->meshDataDescriptorSetInfo.pushDescription(vkh::VkDescriptorUpdateTemplateEntry{
                    .dstBinding = j,
                    .dstArrayElement = 0u,
                    .descriptorCount = uint32_t(meshCount),
                    .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER
                });

                for (uint32_t i=0;i<meshCount;i++) { if (j < this->meshes[i]->bindings.size() && this->meshes[i]->bindings[j].has()) {
                    handle.offset<vk::BufferView>(i) = this->meshes[i]->bindings[j].createBufferView(vk::Format::eR8Uint);
                }};
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
                    }
                    else {
                        handle.offset<vk::BufferView>(i) = this->meshes[i]->bindings[0].createBufferView(vk::Format::eR8Uint);
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

            { // [2] material IDs
                auto& handle = this->bindingsDescriptorSetInfo.pushDescription(vkh::VkDescriptorUpdateTemplateEntry{
                    .dstBinding = 8u,
                    .dstArrayElement = 0u,
                    .descriptorCount = uint32_t(meshCount),
                    .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER
                });

                for (uint32_t i = 0; i < meshCount; i++) {
                    handle.offset<vkh::VkDescriptorBufferInfo>(i) = this->meshes[i]->gpuMaterialIDs;
                };
            };

            // [3] acceleration structure
            if (this->accelerationStructure) {
                this->bindingsDescriptorSetInfo.pushDescription(vkh::VkDescriptorUpdateTemplateEntry{
                    .dstBinding = 2u,
                    .descriptorCount = 1u,
                    .descriptorType = VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR
                }).offset<vk::AccelerationStructureKHR>(0u) = this->accelerationStructure;
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

            // 
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

            { // [7] Mesh Data Info (Has Indices, Material ID, etc.)
                this->bindingsDescriptorSetInfo.pushDescription(vkh::VkDescriptorUpdateTemplateEntry{
                    .dstBinding = 5u,
                    .dstArrayElement = 0u,
                    .descriptorCount = 1u,//uint32_t(meshCount),
                    .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER
                }).offset<vkh::VkDescriptorBufferInfo>(0u) = gpuMeshInfo;
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
            //this->mapMeshData();
            return uTHIS;
        };

        // 
        virtual uPTR(Node) copyMeta(const vk::CommandBuffer& copyCommand = {}) {
            vkt::commandBarrier(copyCommand);
            copyCommand.copyBuffer(this->rawInstances, this->gpuInstances, { vkh::VkBufferCopy{.srcOffset = this->rawInstances.offset(), .dstOffset = this->gpuInstances.offset(), .size = this->gpuInstances.range() } });

            // 
            for (uint32_t i = 0; i < this->meshes.size(); i++) {
                auto& mesh = this->meshes[i];
                copyCommand.copyBuffer(mesh->rawMeshInfo, this->gpuMeshInfo, { vk::BufferCopy{ mesh->rawMeshInfo.offset(), this->gpuMeshInfo.offset() + mesh->rawMeshInfo.range() * i, mesh->rawMeshInfo.range() } });
            };
            vkt::commandBarrier(copyCommand);

            // 
            return uTHIS;
        };

        // 
        virtual uPTR(Node) buildAccelerationStructure(const vk::CommandBuffer& buildCommand = {}) {
            if (!this->accelerationStructure) { this->createAccelerationStructure(); };

            // 
            this->offsetsInfo[0u].primitiveCount = this->instanceCounter;

            // 
            this->instancHeadInfo.dstAccelerationStructure = this->accelerationStructure;
            this->instancHeadInfo.ppGeometries = (this->instancPtr = this->instancInfo.data()).ptr();
            this->instancHeadInfo.scratchData = this->gpuScratchBuffer;
            this->instancHeadInfo.geometryCount = this->instancInfo.size();
            this->instancHeadInfo.update = this->needsUpdate;

            // 
            if (buildCommand) { // OpenGL Compatibility Finally Broken!
                vkt::debugLabel(buildCommand, "Begin building top acceleration structure...", this->driver->getDispatch());
                buildCommand.buildAccelerationStructureKHR(1u, this->instancHeadInfo, reinterpret_cast<vk::AccelerationStructureBuildOffsetInfoKHR**>((this->offsetsPtr = this->offsetsInfo.data()).ptr()), this->driver->getDispatch()); // Can only 1
                vkt::commandBarrier(buildCommand); this->needsUpdate = true;
                vkt::debugLabel(buildCommand, "Ending building top acceleration structure...", this->driver->getDispatch());
            } else {
                driver->getDevice().buildAccelerationStructureKHR(1u, this->instancHeadInfo, reinterpret_cast<vk::AccelerationStructureBuildOffsetInfoKHR**>((this->offsetsPtr = this->offsetsInfo.data()).ptr()), this->driver->getDispatch());
            };

            return uTHIS;
        };

        // Create Or Rebuild Acceleration Structure
        virtual uPTR(Node) createAccelerationStructure() { // Re-assign instance count
            this->topCreate.maxGeometryCount = this->topDataCreate.size();
            this->topCreate.pGeometryInfos = this->topDataCreate.data();

            // 
            if (!this->accelerationStructure) { // create acceleration structure fastly...
                this->accelerationStructure = this->driver->getDevice().createAccelerationStructureKHR(this->topCreate, nullptr, this->driver->getDispatch());

                //
                auto requirements = this->driver->getDevice().getAccelerationStructureMemoryRequirementsKHR(vkh::VkAccelerationStructureMemoryRequirementsInfoKHR{
                    .type = VK_ACCELERATION_STRUCTURE_MEMORY_REQUIREMENTS_TYPE_OBJECT_KHR,
                    .buildType = VK_ACCELERATION_STRUCTURE_BUILD_TYPE_DEVICE_KHR,
                    .accelerationStructure = this->accelerationStructure
                }, this->driver->getDispatch());

                // TODO: fix memoryProperties issue
                TempBuffer = vkt::Vector<uint8_t>(std::make_shared<vkt::VmaBufferAllocation>(this->driver->getAllocator(), vkh::VkBufferCreateInfo{
                    .size = requirements.memoryRequirements.size,
                    .usage = {.eTransferDst = 1, .eStorageTexelBuffer = 1, .eStorageBuffer = 1, .eVertexBuffer = 1, .eSharedDeviceAddress = 1 },
                }));

                // 
                this->driver->getDevice().bindAccelerationStructureMemoryKHR(1u, &vkh::VkBindAccelerationStructureMemoryInfoKHR{
                    .accelerationStructure = this->accelerationStructure,
                    .memory = TempBuffer->getAllocationInfo().memory,
                    .memoryOffset = TempBuffer->getAllocationInfo().offset,
                }.hpp(), this->driver->getDispatch());
            };

            // 
            if (!this->gpuScratchBuffer.has()) { // 
                auto requirements = this->driver->getDevice().getAccelerationStructureMemoryRequirementsKHR(vkh::VkAccelerationStructureMemoryRequirementsInfoKHR{
                    .type = VK_ACCELERATION_STRUCTURE_MEMORY_REQUIREMENTS_TYPE_BUILD_SCRATCH_KHR,
                    .buildType = VK_ACCELERATION_STRUCTURE_BUILD_TYPE_DEVICE_KHR,
                    .accelerationStructure = this->accelerationStructure
                }, this->driver->getDispatch());

                // 
                this->gpuScratchBuffer = vkt::Vector<uint8_t>(std::make_shared<vkt::VmaBufferAllocation>(driver->getAllocator(), vkh::VkBufferCreateInfo{
                    .size = requirements.memoryRequirements.size,
                    .usage = { .eStorageBuffer = 1, .eRayTracing = 1, .eSharedDeviceAddress = 1 }
                }, VMA_MEMORY_USAGE_GPU_ONLY));
            };

            // 
            this->mapMeshData();
            return uTHIS;
        };

    protected: // 
        std::vector<vkt::uni_ptr<MeshBinding>> meshes = {}; // Mesh list as Template for Instances
        std::vector<uint32_t> mapMeshes = {};
        uintptr_t MaxInstanceCount = 64u;

        // 
        vkt::Vector<vkh::VsGeometryInstance> rawInstances = {}; // Ray-Tracing instances Will re-located into meshes by Index, and will no depending by mesh list...
        vkt::Vector<vkh::VsGeometryInstance> gpuInstances = {};
        uint32_t instanceCounter = 0u;
        bool needsUpdate = false;

        // 
        vkh::VsDescriptorSetCreateInfoHelper meshDataDescriptorSetInfo = {};
        vkh::VsDescriptorSetCreateInfoHelper bindingsDescriptorSetInfo = {};


        // FOR CREATE (Acceleration Structure)
        vkh::VkAccelerationStructureCreateInfoKHR topCreate = {}; // CREATE SINGLE! 
        std::vector<vkh::VkAccelerationStructureCreateGeometryTypeInfoKHR> topDataCreate = { {} }; // CAN BE MULTIPLE!

        // FOR BUILD! BUT ONLY SINGLE!
        vkh::VkAccelerationStructureBuildGeometryInfoKHR                  instancHeadInfo = { {} };
        //std::vector<vkh::VkAccelerationStructureBuildGeometryInfoKHR>   instancHeadInfo = { {} };
        //vkt::uni_arg<vkh::VkAccelerationStructureBuildGeometryInfoKHR*> instancHeadPtr = {};

        // CAN BE MULTIPLE! (single element of array, array of array[0])
        std::vector<vkh::VkAccelerationStructureBuildOffsetInfoKHR>     offsetsInfo = { {} };
        vkt::uni_arg<vkh::VkAccelerationStructureBuildOffsetInfoKHR*>   offsetsPtr = {};

        // CAN BE MULTIPLE! (single element of array, array of array[0])
        std::vector<vkh::VkAccelerationStructureGeometryKHR>   instancInfo = { {} };
        vkt::uni_arg<vkh::VkAccelerationStructureGeometryKHR*> instancPtr = {};


        // 
        vkt::Vector<uint8_t> TempBuffer = {};
        vkt::Vector<glm::uvec4> gpuMeshInfo = {};

        // 
        vk::DescriptorSet meshDataDescriptorSet = {};
        vk::DescriptorSet bindingsDescriptorSet = {};
        vk::AccelerationStructureKHR accelerationStructure = {};
        vkt::Vector<uint8_t> gpuScratchBuffer = {};
        VmaAllocationInfo allocationInfo = {};
        VmaAllocation allocation = {};
        
        // 
        vkt::uni_ptr<Driver> driver = {};
        vkt::uni_ptr<Thread> thread = {};
        vkt::uni_ptr<Context> context = {};
    };

};
