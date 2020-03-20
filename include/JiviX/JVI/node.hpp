#pragma once // #

#include "./config.hpp"
#include "./driver.hpp"
#include "./thread.hpp"
#include "./mesh.hpp"

namespace jvi {

    // WIP Instances
    // ALSO, RAY-TRACING PIPELINES WILL USE NATIVE BINDING AND ATTRIBUTE READERS
    class Node : public std::enable_shared_from_this<Node> { public: friend Renderer;
        Node() {};
        Node(const vkt::uni_ptr<Context>& context) : context(context) { this->construct(); };
        //Node(Context* context) : context(context) { this->context = vkt::uni_ptr<Context>(context); this->construct(); };
        ~Node() {};

        // 
        virtual vkt::uni_ptr<Node> sharedPtr() { return shared_from_this(); };
        virtual vkt::uni_ptr<const Node> sharedPtr() const { return shared_from_this(); };

        //
        virtual uPTR(Node) construct() {
            this->driver = context->getDriver();
            this->thread = std::make_shared<Thread>(this->driver);
            this->context = context;

            // 
            this->createInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR;

            // 
            auto allocInfo = vkt::MemoryAllocationInfo{};
            allocInfo.device = *driver;
            allocInfo.memoryProperties = driver->getMemoryProperties().memoryProperties;
            allocInfo.dispatch = driver->getDispatch();


            this->rawInstances = vkt::Vector<vkh::VsGeometryInstance>(this->driver->getAllocator(), vkh::VkBufferCreateInfo{ .size = sizeof(vkh::VsGeometryInstance) * MaxInstanceCount, .usage = {.eTransferSrc = 1, .eStorageBuffer = 1, .eRayTracing = 1 } }, VMA_MEMORY_USAGE_CPU_TO_GPU);
            this->gpuInstances = vkt::Vector<vkh::VsGeometryInstance>(allocInfo, vkh::VkBufferCreateInfo{ .size = sizeof(vkh::VsGeometryInstance) * MaxInstanceCount, .usage = {.eTransferDst = 1, .eStorageBuffer = 1, .eRayTracing = 1, .eSharedDeviceAddress = 1 } });

            // 
            this->gpuMeshInfo = vkt::Vector<glm::uvec4>(this->driver->getAllocator(), vkh::VkBufferCreateInfo{ .size = 16u * 64u, .usage = { .eTransferDst = 1, .eUniformBuffer = 1, .eStorageBuffer = 1, .eRayTracing = 1 } }, VMA_MEMORY_USAGE_GPU_ONLY);
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
        virtual uPTR(Node) pushInstance(const vkt::uni_arg<vkh::VsGeometryInstance>& instance = vkh::VsGeometryInstance{}) {
            const auto instanceID = this->instanceCounter++;
            const uint32_t meshID = instance->instanceId;
            this->rawInstances[instanceID] = instance;

            //this->rawInstances[instanceID].instanceId = meshID; // Customize Per Mesh
            this->mapMeshes.push_back(meshID);
            if (!this->meshes[meshID]->accelerationStructure) {
                this->meshes[meshID]->buildAccelerationStructure();
            };
            if (this->meshes[meshID]->accelerationStructure) {
                this->rawInstances[instanceID].accelerationStructureHandle = this->driver->getDevice().getAccelerationStructureAddressKHR(this->meshes[meshID]->accelerationStructure, this->driver->getDispatch());
                //this->driver->getDevice().getAccelerationStructureAddressKHR(this->meshes[meshID]->accelerationStructure, 8ull, &this->rawInstances[instanceID].accelerationStructureHandle, this->driver->getDispatch());
            };

            return uTHIS;
        };

        // 
        virtual uPTR(Node) mapMeshData() {
            for (uint32_t i = 0; i < this->mapMeshes.size(); i++) {
                this->rawInstances[i].accelerationStructureHandle = this->driver->getDevice().getAccelerationStructureAddressKHR(this->meshes[this->mapMeshes[i]]->accelerationStructure, this->driver->getDispatch());
                //this->driver->getDevice().getAccelerationStructureAddressKHR(this->meshes[this->mapMeshes[i]]->accelerationStructure, sizeof(uint64_t), &this->rawInstances[i].accelerationStructureHandle, this->driver->getDispatch());
                //std::cout << this->rawInstances[i].accelerationStructureHandle << std::endl;
            };
            return uTHIS;
        };

        // Push Mesh "Template" For Any Other Instances
        virtual uintptr_t pushMesh(const vkt::uni_ptr<Mesh>& mesh = {}) {
            const uintptr_t ptr = this->meshes.size();
            this->meshes.push_back(mesh); return ptr;
        };

        // WARNING!!! NOT RECOMMENDED! 
        [[deprecated]]
        virtual uintptr_t pushMesh(Mesh* mesh) {
            const uintptr_t ptr = this->meshes.size();
            this->meshes.push_back(vkt::uni_ptr<Mesh>(mesh)); return ptr;
        };

        // WARNING!!! NOT RECOMMENDED! 
        [[deprecated]]
        virtual uintptr_t pushMesh(Mesh& mesh) {
            const uintptr_t ptr = this->meshes.size();
            this->meshes.push_back(vkt::uni_ptr<Mesh>(&mesh)); return ptr;
        };

        // 
        virtual uPTR(Node) createDescriptorSet() { // 
            this->bindingsDescriptorSetInfo = vkh::VsDescriptorSetCreateInfoHelper(this->context->bindingsDescriptorSetLayout, this->thread->getDescriptorPool());
            this->meshDataDescriptorSetInfo = vkh::VsDescriptorSetCreateInfoHelper(this->context->meshDataDescriptorSetLayout, this->thread->getDescriptorPool());

            // plush descriptor set bindings (i.e. buffer bindings array, every have array too)
            const auto bindingCount = 8u;
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
            this->mapMeshData();

            // 
            return uTHIS;
        };

        // 
        virtual uPTR(Node) buildAccelerationStructure(const vk::CommandBuffer& buildCommand = {}) {
            if (!this->accelerationStructure) { this->createAccelerationStructure(); };
            buildCommand.copyBuffer(this->rawInstances, this->gpuInstances, { vkh::VkBufferCopy{ .srcOffset = this->rawInstances.offset(), .dstOffset = this->gpuInstances.offset(), .size = this->gpuInstances.range() } });

            // 
            for (uint32_t i = 0; i < this->meshes.size(); i++) {
                auto& mesh = this->meshes[i];
                buildCommand.copyBuffer(mesh->rawMeshInfo, this->gpuMeshInfo, { vk::BufferCopy{ mesh->rawMeshInfo.offset(), this->gpuMeshInfo.offset() + mesh->rawMeshInfo.range() * i, mesh->rawMeshInfo.range() } });
            };

            // 
            this->dataInfo.resize(1u);
            this->dataInfo[0u].geometryType = VK_GEOMETRY_TYPE_INSTANCES_KHR;
            this->dataInfo[0u].geometry.instances.data = this->gpuInstances;//vkt::Vector<vkh::VsGeometryInstance>(this->gpuInstances.getAllocation(), sizeof(vkh::VsGeometryInstance) * instanceID, sizeof(vkh::VsGeometryInstance));

            // 
            this->offsetsInfo.resize(1u);
            this->offsetsInfo[0u].primitiveOffset = 0u;
            this->offsetsInfo[0u].primitiveCount = this->instanceCounter;

            // 
            this->instancInfo[0].flags = VK_BUILD_ACCELERATION_STRUCTURE_ALLOW_UPDATE_BIT_KHR | VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR;
            this->instancInfo[0].dstAccelerationStructure = this->accelerationStructure;
            this->instancInfo[0].geometryCount = 1u;
            this->instancInfo[0].ppGeometries = (this->dataPtr = this->dataInfo.data()).ptr();
            this->instancInfo[0].type = VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR;
            this->instancInfo[0].scratchData = this->gpuScratchBuffer;

            // 
            vkt::commandBarrier(buildCommand);
            //buildCommand.buildAccelerationStructureKHR(1u, &this->instancInfo[0].hpp(), reinterpret_cast<vk::AccelerationStructureBuildOffsetInfoKHR**>((offsetsPtr = this->offsetsInfo.data()).ptr()), this->driver->getDispatch()); // Can only 1
            driver->getDevice().buildAccelerationStructureKHR(1u, &this->instancInfo[0].hpp(), reinterpret_cast<vk::AccelerationStructureBuildOffsetInfoKHR**>((offsetsPtr = this->offsetsInfo.data()).ptr()), this->driver->getDispatch());
            vkt::commandBarrier(buildCommand);
            this->needsUpdate = true; return uTHIS;
        };

        // Create Or Rebuild Acceleration Structure
        virtual uPTR(Node) createAccelerationStructure() { // Re-assign instance count
            this->instanceCreateInfos[0u].geometryType = VK_GEOMETRY_TYPE_INSTANCES_KHR;
            this->instanceCreateInfos[0u].maxPrimitiveCount = static_cast<uint32_t>(MaxInstanceCount);
            this->instanceCreateInfos[0u].allowsTransforms = true;

            // 
            this->createInfo.flags = VK_BUILD_ACCELERATION_STRUCTURE_ALLOW_UPDATE_BIT_KHR | VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR;
            this->createInfo.maxGeometryCount = this->instanceCreateInfos.size();
            this->createInfo.pGeometryInfos = this->instanceCreateInfos.data();
            this->createInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR;

            // 
            if (!this->accelerationStructure) { // create acceleration structure fastly...
                this->accelerationStructure = this->driver->getDevice().createAccelerationStructureKHR(this->createInfo, nullptr, this->driver->getDispatch());

                //
                auto requirements = this->driver->getDevice().getAccelerationStructureMemoryRequirementsKHR(vkh::VkAccelerationStructureMemoryRequirementsInfoKHR{
                    .type = VK_ACCELERATION_STRUCTURE_MEMORY_REQUIREMENTS_TYPE_OBJECT_KHR,
                    .accelerationStructure = this->accelerationStructure
                }, this->driver->getDispatch());

                // 
                auto allocInfo = vkt::MemoryAllocationInfo{};
                allocInfo.device = *driver;
                allocInfo.memoryProperties = driver->getMemoryProperties().memoryProperties;
                allocInfo.dispatch = driver->getDispatch();
                allocInfo.reqSize = requirements.memoryRequirements.size;
                allocInfo.range = requirements.memoryRequirements.size;

                // 
                this->driver->getDevice().bindAccelerationStructureMemoryKHR(1u, &vkh::VkBindAccelerationStructureMemoryInfoKHR{
                    .accelerationStructure = this->accelerationStructure,
                    .memory = (TempBuffer = vkt::Vector<uint8_t>(allocInfo, vkh::VkBufferCreateInfo{
                        .size = requirements.memoryRequirements.size,
                        .usage = {.eTransferDst = 1, .eStorageTexelBuffer = 1, .eStorageBuffer = 1, .eVertexBuffer = 1, .eSharedDeviceAddress = 1 },
                    }))->getAllocationInfo().memory,
                }.hpp(), this->driver->getDispatch());
            };

            // 
            if (!this->gpuScratchBuffer.has()) { // 
                auto requirements = this->driver->getDevice().getAccelerationStructureMemoryRequirementsKHR(vkh::VkAccelerationStructureMemoryRequirementsInfoKHR{
                    .type = VK_ACCELERATION_STRUCTURE_MEMORY_REQUIREMENTS_TYPE_BUILD_SCRATCH_KHR,
                    .accelerationStructure = this->accelerationStructure
                }, this->driver->getDispatch());

                // 
                this->gpuScratchBuffer = vkt::Vector<uint8_t>(driver->getAllocator(), vkh::VkBufferCreateInfo{
                    .size = requirements.memoryRequirements.size,
                    .usage = { .eStorageBuffer = 1, .eRayTracing = 1, .eSharedDeviceAddress = 1 }
                }, VMA_MEMORY_USAGE_GPU_ONLY);
            };

            // 
            this->mapMeshData();
            return uTHIS;
        };

    protected: // 
        std::vector<vkt::uni_ptr<Mesh>> meshes = {}; // Mesh list as Template for Instances
        std::vector<uint32_t> mapMeshes = {};
        uintptr_t MaxInstanceCount = 64;

        // 
        vkt::Vector<vkh::VsGeometryInstance> rawInstances = {}; // Ray-Tracing instances Will re-located into meshes by Index, and will no depending by mesh list...
        vkt::Vector<vkh::VsGeometryInstance> gpuInstances = {};
        uint32_t instanceCounter = 0u;
        bool needsUpdate = false;

        // 
        vkh::VsDescriptorSetCreateInfoHelper meshDataDescriptorSetInfo = {};
        vkh::VsDescriptorSetCreateInfoHelper bindingsDescriptorSetInfo = {};

        // ONLY SINGLE!
        vkh::VkAccelerationStructureCreateInfoKHR createInfo = {};
        std::vector<vkh::VkAccelerationStructureCreateGeometryTypeInfoKHR> instanceCreateInfos = { {} };

        // 
        std::vector<vkh::VkAccelerationStructureBuildGeometryInfoKHR>   instancInfo = { {} };
        std::vector<vkh::VkAccelerationStructureBuildOffsetInfoKHR>     offsetsInfo = { {} };
        vkt::uni_arg<vkh::VkAccelerationStructureBuildGeometryInfoKHR*> instancPtr = {};
        vkt::uni_arg<vkh::VkAccelerationStructureBuildOffsetInfoKHR*>   offsetsPtr = {};

        // Element data
        std::vector<vkh::VkAccelerationStructureGeometryKHR> dataInfo = { {} };
        vkt::uni_arg<vkh::VkAccelerationStructureGeometryKHR*> dataPtr = {};

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
