#pragma once // #

#include "./config.hpp"
#include "./driver.hpp"
#include "./thread.hpp"
#include "./mesh-binding.hpp"


namespace jvi {

// The Switch
#if !defined(NEW_VERSION) && !defined(OLD_VERSION)
#define NEW_VERSION
#endif

#ifdef NEW_VERSION
    // WIP Instances
    // ALSO, RAY-TRACING PIPELINES WILL USE NATIVE BINDING AND ATTRIBUTE READERS
    class Node : public std::enable_shared_from_this<Node> { public: friend Renderer; 
        Node() {};
        Node(const vkt::uni_ptr<Context>& context, const uint32_t& MaxInstanceCount = 64u) : context(context), MaxInstanceCount(MaxInstanceCount) { this->construct(); };
        Node(const std::shared_ptr<Context>& context, const uint32_t& MaxInstanceCount = 64u) : context(context), MaxInstanceCount(MaxInstanceCount) { this->construct(); };
        ~Node() {};

        // 
        public: virtual vkt::uni_ptr<Node> sharedPtr() { return shared_from_this(); };
        //virtual vkt::uni_ptr<Node> sharedPtr() const { return std::shared_ptr<Node>(shared_from_this()); };

        //
        protected: virtual uPTR(Node) construct() {
            this->driver = context->getDriver();
            this->thread = std::make_shared<Thread>(this->driver);
            this->context = context;

            // 
            this->topCreate.type = VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR;

            //
            auto bflgs = vkh::VkBufferCreateFlags::create();
            auto hostUsage = vkh::VkBufferUsageFlags::create([&](auto* it){ it->eTransferSrc = 1, it->eStorageBuffer = 1, it->eUniformBuffer = 1, it->eRayTracing = 1; return it; });
            auto gpuUsage = vkh::VkBufferUsageFlags::create([&](auto* it) { it->eTransferDst = 1, it->eStorageBuffer = 1, it->eUniformBuffer = 1, it->eRayTracing = 1, it->eSharedDeviceAddress = 1; return it; });
            this->rawInstances = vkt::Vector<vkh::VsGeometryInstance>(std::make_shared<vkt::VmaBufferAllocation>(this->driver->getAllocator(), vkh::VkBufferCreateInfo{ .flags = bflgs, .size = sizeof(vkh::VsGeometryInstance) * std::max(uint64_t(MaxInstanceCount), uint64_t(64ull)), .usage = hostUsage}, vkt::VmaMemoryInfo{ .memUsage = VMA_MEMORY_USAGE_CPU_TO_GPU }));
            this->gpuInstances = vkt::Vector<vkh::VsGeometryInstance>(std::make_shared<vkt::VmaBufferAllocation>(this->driver->getAllocator(), vkh::VkBufferCreateInfo{ .flags = bflgs, .size = sizeof(vkh::VsGeometryInstance) * std::max(uint64_t(MaxInstanceCount), uint64_t(64ull)), .usage = gpuUsage }, vkt::VmaMemoryInfo{ .memUsage = VMA_MEMORY_USAGE_GPU_ONLY }));

            //
            this->gpuMeshInfo = vkt::Vector<glm::uvec4>(std::make_shared<vkt::VmaBufferAllocation>(this->driver->getAllocator(), vkh::VkBufferCreateInfo{ .flags = bflgs, .size = uint64_t(16u * std::max(uint64_t(MaxInstanceCount), uint64_t(64ull))), .usage = gpuUsage }, vkt::VmaMemoryInfo{ .memUsage = VMA_MEMORY_USAGE_GPU_ONLY }));

            // FOR BUILD!
            auto bdHeadFlags = vkh::VkBuildAccelerationStructureFlagsKHR{ .eAllowUpdate = 1, .ePreferFastTrace = 1 };
            this->instancHeadInfo = vkh::VkAccelerationStructureBuildGeometryInfoKHR{};
            this->instancHeadInfo.geometryCount = this->instancInfo.size();
            this->instancHeadInfo.ppGeometries = reinterpret_cast<vkh::VkAccelerationStructureGeometryKHR**>((this->instancPtr = this->instancInfo.data()).ptr());
            this->instancHeadInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR;
            this->instancHeadInfo.flags = bdHeadFlags;
            this->instancHeadInfo.geometryArrayOfPointers = false;//true;

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
        public: virtual uPTR(Node) setContext(const vkt::uni_ptr<Context>& context) {
            this->context = context;
            return uTHIS;
        };

        // 
        public: virtual uPTR(Node) setThread(const vkt::uni_ptr<Thread>& thread) {
            this->thread = thread;
            return uTHIS;
        };

        // 
        public: virtual uPTR(Node) setRawInstance(const vkt::Vector<vkh::VsGeometryInstance>& rawInstances = {}, const uint32_t& instanceCounter = 0u) {
            this->rawInstances = rawInstances;
            this->mapMeshes.resize(this->instanceCounter = instanceCounter);
            return uTHIS;
        };

        // 
        public: virtual uPTR(Node) setGpuInstance(const vkt::Vector<vkh::VsGeometryInstance>& gpuInstances = {}) {
            this->gpuInstances = gpuInstances;
            return uTHIS;
        };

        // 
        public: virtual uPTR(Node) resetInstances() {
            for (auto& Mesh : this->meshes) { Mesh->resetInstanceMap(); };
            this->instanceCounter = 0;
            this->mapMeshes.resize(0);
            this->prepareInstances.resize(0);
            return uTHIS;
        };

        //
        public: virtual uPTR(Node) pushInstance(vkt::uni_arg<vkh::VsGeometryInstance> instance = vkh::VsGeometryInstance{}) {
            //if (this->meshes[instance->instanceId] && this->meshes[instance->instanceId]->fullGeometryCount > 0) {
            if (instance->instanceId < this->meshes.size() && this->meshes[instance->instanceId]) {
                const auto instanceID = this->prepareInstances.size(); //this->instanceCounter++;
                const auto meshID = instance->instanceId;
                //this->rawInstances[instanceID] = instance; // List Of Instances

                // Add Instance ID into Mesh, and Map Instance With Mesh ID
                this->prepareInstances.push_back(instance);
                this->mapMeshes.push_back(meshID);

                // 
                //this->meshes[meshID]->linkWithInstance(instanceID); // UnUsed for Rendering
            };
            return uTHIS;
        };

        protected: virtual uPTR(Node) buildGeometryAccelerationStructure(const VkCommandBuffer& cmdBuffer) {
            uintptr_t I = 0u; for (auto& Mesh : this->meshes) {
                auto HLP = glm::uvec4(I++, 0u, 0u, 0u);
                Mesh->buildAccelerationStructure(cmdBuffer, HLP);
            };
            return uTHIS;
        };

        //
        protected: virtual uPTR(Node) mapMeshData(const VkCommandBuffer& cmdBuffer) {
            for (auto& Mesh : this->meshes) { Mesh->resetInstanceMap(); }; uintptr_t I = 0;
            for (uint32_t i = 0; i < this->mapMeshes.size(); i++) {
                auto& Mesh = this->meshes[this->mapMeshes[i]];
                if (Mesh->linkWithInstance(I++)->fullGeometryCount > 0) { // Link With Found Instances?! Finally fixed HERE... 
                    if (!this->prepareInstances[i].accelerationStructureHandle) {
                        this->prepareInstances[i].accelerationStructureHandle = this->driver->getDeviceDispatch()->GetAccelerationStructureDeviceAddressKHR(vkh::VkAccelerationStructureDeviceAddressInfoKHR{ .accelerationStructure = Mesh->accelerationStructure });
                    };
                };
            };
            I = 0u; for (auto& Mesh : this->meshes) {
                auto HLP = glm::uvec4(I++, 0u, 0u, 0u);
                Mesh->buildGeometry(cmdBuffer, HLP);
            };
            return uTHIS;
        };

        // Push Mesh "Template" For Any Other Instances
        public: virtual uintptr_t pushMesh(const vkt::uni_ptr<MeshBinding>& mesh = {}) {
            const uintptr_t ptr = this->meshes.size();
            //if (ptr < 1u)
                this->meshes.push_back(mesh);
            return ptr;
        };

        // 
        public: virtual uintptr_t pushMesh(const std::shared_ptr<MeshBinding>& mesh) { return this->pushMesh(vkt::uni_ptr<MeshBinding>(mesh)); };

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
        protected: virtual uPTR(Node) createDescriptorSet() { // 
            this->bindingsDescriptorSetInfo = vkh::VsDescriptorSetCreateInfoHelper(this->context->bindingsDescriptorSetLayout, this->thread->getDescriptorPool());
            this->meshDataDescriptorSetInfo = vkh::VsDescriptorSetCreateInfoHelper(this->context->meshDataDescriptorSetLayout, this->thread->getDescriptorPool());

            //
            auto bflgs = vkh::VkBufferCreateFlags{};
            auto iflgs = vkh::VkImageCreateFlags{};
            vkt::unlock32(bflgs) = 0u;
            vkt::unlock32(iflgs) = 0u;

            // 
            const auto mapWidth = 1024u, mapHeight = 1024u;
            auto& allocInfo = driver->memoryAllocationInfo();

            // With Additional Elements, For Counters
            auto mapUsage = vkh::VkBufferUsageFlags{.eTransferDst = 1, .eUniformBuffer = 1, .eStorageBuffer = 1, .eRayTracing = 1 };
            this->mapData = vkt::Vector<uint32_t>(std::make_shared<vkt::VmaBufferAllocation>(this->driver->getAllocator(), vkh::VkBufferCreateInfo{ .flags = bflgs, .size = 2048u * 2048u * 4u + 64u, .usage = mapUsage }, vkt::VmaMemoryInfo{ .memUsage = VMA_MEMORY_USAGE_GPU_ONLY }));

            //
            const auto aspectMask = vkh::VkImageAspectFlags{.eColor = 1};
            auto imgUsage = vkh::VkImageUsageFlags{.eTransferDst = 1, .eSampled = 1, .eStorage = 1, .eColorAttachment = 1 };
            this->colImage = vkt::ImageRegion(std::make_shared<vkt::ImageAllocation>(vkh::VkImageCreateInfo{
                .flags = iflgs,
                .format = VK_FORMAT_R32G32B32A32_SFLOAT,
                .extent = {mapWidth,mapHeight,1u},
                .usage = imgUsage,
            }, allocInfo), vkh::VkImageViewCreateInfo{
                .format = VK_FORMAT_R32G32B32A32_SFLOAT,
                .subresourceRange = {.aspectMask = aspectMask},
            });

            // Create Sampler By Reference
            vkh::handleVk(this->driver->getDeviceDispatch()->CreateSampler(vkh::VkSamplerCreateInfo{
                .magFilter = VK_FILTER_LINEAR,
                .minFilter = VK_FILTER_LINEAR,
                .addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
                .addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
                .unnormalizedCoordinates = true,
            }, nullptr, &this->colImage.refSampler()));


            // 
            this->mapImage = vkt::ImageRegion(std::make_shared<vkt::ImageAllocation>(vkh::VkImageCreateInfo{
                .flags = iflgs,
                .format = VK_FORMAT_R32_UINT,
                .extent = {mapWidth,mapHeight,1u},
                .usage = imgUsage,
            }, allocInfo), vkh::VkImageViewCreateInfo{
                .format = VK_FORMAT_R32_UINT,
                .subresourceRange = {.aspectMask = aspectMask},
            });

            // Create Sampler By Reference
            //vkh::handleVk(this->driver->getDeviceDispatch()->CreateSampler(vkh::VkSamplerCreateInfo{
            //    .magFilter = VK_FILTER_NEAREST,
            //    .minFilter = VK_FILTER_NEAREST,
            //    .addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
            //    .addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
            //    .unnormalizedCoordinates = true,
            //}, nullptr, &this->mapImage.refSampler()));

            //
            auto dAspectMask = vkh::VkImageAspectFlags{.eDepth = 1, .eStencil = 1};
            this->depImage = vkt::ImageRegion(std::make_shared<vkt::ImageAllocation>(vkh::VkImageCreateInfo{
                .flags = iflgs,
                .format = VK_FORMAT_D32_SFLOAT_S8_UINT,
                .extent = {mapWidth,mapHeight,1u},
                .usage = imgUsage,
            }, allocInfo), vkh::VkImageViewCreateInfo{
                .format = VK_FORMAT_D32_SFLOAT_S8_UINT,
                .subresourceRange = {.aspectMask = dAspectMask},
            });

            // Create Sampler By Reference
            vkh::handleVk(this->driver->getDeviceDispatch()->CreateSampler(vkh::VkSamplerCreateInfo{
                .magFilter = VK_FILTER_LINEAR,
                .minFilter = VK_FILTER_LINEAR,
                .addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
                .addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
                .unnormalizedCoordinates = true,
            }, nullptr, &this->depImage.refSampler()));

            // Mapping Attachment
            std::vector<VkImageView> attachments = { colImage, depImage };
            vkh::handleVk(this->driver->getDeviceDispatch()->CreateFramebuffer(vkh::VkFramebufferCreateInfo{
                .renderPass = this->context->mapRenderPass,
                .attachmentCount = static_cast<uint32_t>(attachments.size()),
                .pAttachments = attachments.data(),
                .width = mapWidth,
                .height = mapHeight
            }, nullptr, &this->mapFramebuffer));

            // 
            thread->submitOnce([&, this](VkCommandBuffer& cmd) {
                this->driver->getDeviceDispatch()->CmdFillBuffer(cmd, this->mapData, 0u, this->mapData.range(), 0u);
                this->driver->getDeviceDispatch()->CmdClearColorImage(cmd, this->colImage.transfer(cmd), this->colImage.getImageLayout(), vkh::VkClearColorValue{ .float32 = { 0.f,0.f,0.f,0.f } }, 1u, colImage.getImageSubresourceRange());
                this->driver->getDeviceDispatch()->CmdClearColorImage(cmd, this->mapImage.transfer(cmd), this->mapImage.getImageLayout(), vkh::VkClearColorValue{ .uint32 = { 0u,0u,0u,0u } }, 1u, mapImage.getImageSubresourceRange());
                this->driver->getDeviceDispatch()->CmdClearDepthStencilImage(cmd, this->depImage.transfer(cmd), this->depImage.getImageLayout(), vkh::VkClearDepthStencilValue{.depth = 1.0f, .stencil = 0}, 1u, depImage.getImageSubresourceRange());
            });


            // plush descriptor set bindings (i.e. buffer bindings array, every have array too)
            const auto meshCount = std::min(uint64_t(this->meshes.size()), uint64_t(64ull));
            {
                auto &handle = this->meshDataDescriptorSetInfo.pushDescription(vkh::VkDescriptorUpdateTemplateEntry{
                    .dstBinding = 0u,
                    .dstArrayElement = 0u,
                    .descriptorCount = uint32_t(meshCount),
                    .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER
                });

                for (uint32_t i = 0; i < meshCount; i++) {
                    handle.offset<VkBufferView>(i) = this->meshes[i]->bindings[0u].createBufferView(VK_FORMAT_R8_UINT);
                };
            }

            { // unknown mystery reason...
                auto& handle = this->meshDataDescriptorSetInfo.pushDescription(vkh::VkDescriptorUpdateTemplateEntry{
                    .dstBinding = 1u,
                    .dstArrayElement = 0u,
                    .descriptorCount = uint32_t(meshCount),
                    .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER
                });

                for (uint32_t i = 0; i < meshCount; i++) {
                    if (this->meshes[i]->indexData.has()) {
                        handle.offset<VkBufferView>(i) = this->meshes[i]->indexData.createBufferView(VK_FORMAT_R8_UINT); }
                    else {
                        handle.offset<VkBufferView>(i) = this->meshes[i]->bindings[0].createBufferView(VK_FORMAT_R8_UINT);
                    };
                };
            };

            { //
                auto& handle = this->meshDataDescriptorSetInfo.pushDescription(vkh::VkDescriptorUpdateTemplateEntry{
                    .dstBinding = 2u,
                    .dstArrayElement = 0u,
                    .descriptorCount = 1u,
                    .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER
                });

                handle.offset<VkDescriptorBufferInfo>(0u) = mapData;
            };

            //
            {
                auto& handle = this->meshDataDescriptorSetInfo.pushDescription(vkh::VkDescriptorUpdateTemplateEntry{
                    .dstBinding = 3u,
                    .dstArrayElement = 0u,
                    .descriptorCount = 1u,
                    .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE
                });

                handle.offset<VkDescriptorImageInfo>(0u) = mapImage;
            };

            { //
                auto& handle = this->meshDataDescriptorSetInfo.pushDescription(vkh::VkDescriptorUpdateTemplateEntry{
                    .dstBinding = 4u,
                    .dstArrayElement = 0u,
                    .descriptorCount = 2u,
                    .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER
                });

                handle.offset<VkDescriptorImageInfo>(0u) = colImage;
                handle.offset<VkDescriptorImageInfo>(1u) = depImage;
            };


            { //
                auto& handle = this->bindingsDescriptorSetInfo.pushDescription(vkh::VkDescriptorUpdateTemplateEntry{
                    .dstBinding = 5u,
                    .dstArrayElement = 0u,
                    .descriptorCount = uint32_t(meshCount),
                    .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER
                });

                for (uint32_t i = 0; i < meshCount; i++) {
                    handle.offset<vkh::VkDescriptorBufferInfo>(i) = this->meshes[i]->gpuBindings;
                };
            };

            { //
                auto& handle = this->bindingsDescriptorSetInfo.pushDescription(vkh::VkDescriptorUpdateTemplateEntry{
                    .dstBinding = 6u,
                    .dstArrayElement = 0u,
                    .descriptorCount = uint32_t(meshCount),
                    .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER
                });

                for (uint32_t i = 0; i < meshCount; i++) {
                    handle.offset<vkh::VkDescriptorBufferInfo>(i) = this->meshes[i]->gpuAttributes;
                };
            };

            //
            for (uint32_t i = 0; i < meshCount; i++) {
                if (this->meshes[i]->gpuTransformData.has()) {
                    this->bindingsDescriptorSetInfo.pushDescription(vkh::VkDescriptorUpdateTemplateEntry{
                        .dstBinding = 7u,
                        .dstArrayElement = i,
                        .descriptorCount = 1u,
                        .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER
                    }).offset<vkh::VkDescriptorBufferInfo>(0u) = this->meshes[i]->gpuTransformData;
                };
            };

            { //
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

            //
            this->bindingsDescriptorSetInfo.pushDescription(vkh::VkDescriptorUpdateTemplateEntry{
                .dstBinding = 9u,
                .descriptorCount = 1u,
                .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER
            }).offset<vkh::VkDescriptorBufferInfo>(0u) = this->context->uniformGPUData;

            { //
                this->bindingsDescriptorSetInfo.pushDescription(vkh::VkDescriptorUpdateTemplateEntry{
                    .dstBinding = 10u,
                    .dstArrayElement = 0u,
                    .descriptorCount = 1u,//uint32_t(meshCount),
                    .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER
                }).offset<vkh::VkDescriptorBufferInfo>(0u) = this->gpuMeshInfo;
            };

            //
            this->bindingsDescriptorSetInfo.pushDescription(vkh::VkDescriptorUpdateTemplateEntry{
                .dstBinding = 11u,
                .descriptorCount = 1u,
                .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER
            }).offset<vkh::VkDescriptorBufferInfo>(0u) = this->gpuInstances;

            //
            if (this->accelerationStructure) {
                this->bindingsDescriptorSetInfo.pushDescription(vkh::VkDescriptorUpdateTemplateEntry{
                    .dstBinding = 12u,
                    .descriptorCount = 1u,
                    .descriptorType = VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR
                }).offset<VkAccelerationStructureKHR>(0u) = this->accelerationStructure;
            };

            // 
            vkh::handleVk(vkt::AllocateDescriptorSetWithUpdate(driver->getDeviceDispatch(), this->meshDataDescriptorSetInfo, this->meshDataDescriptorSet, this->descriptorUpdatedM));
            vkh::handleVk(vkt::AllocateDescriptorSetWithUpdate(driver->getDeviceDispatch(), this->bindingsDescriptorSetInfo, this->bindingsDescriptorSet, this->descriptorUpdatedB));

            // 
            this->context->descriptorSets[0] = this->meshDataDescriptorSet;
            this->context->descriptorSets[1] = this->bindingsDescriptorSet;

            // remap mesh data
            return uTHIS;
        };


        // 
        protected: virtual uPTR(Node) clearMappedData(const VkCommandBuffer& currentCmd = {}) {
            this->driver->getDeviceDispatch()->CmdFillBuffer(currentCmd, this->mapData, 0u, this->mapData.range(), 0u);
            this->driver->getDeviceDispatch()->CmdClearColorImage(currentCmd, this->colImage, this->colImage.getImageLayout(), vkh::VkClearColorValue{ .float32 = { 0.f,0.f,0.f,0.f } }, 1u, colImage.getImageSubresourceRange());
            this->driver->getDeviceDispatch()->CmdClearColorImage(currentCmd, this->mapImage, this->mapImage.getImageLayout(), vkh::VkClearColorValue{ .uint32 = { 0u,0u,0u,0u } }, 1u, mapImage.getImageSubresourceRange());
            this->driver->getDeviceDispatch()->CmdClearDepthStencilImage(currentCmd, this->depImage, this->depImage.getImageLayout(), vkh::VkClearDepthStencilValue{ .depth = 1.0f, .stencil = 0 }, 1u, depImage.getImageSubresourceRange());
            vkt::commandBarrier(this->driver->getDeviceDispatch(), currentCmd);
            return uTHIS;
        };

        //
        protected: virtual uPTR(Node) mappingGeometry(const VkCommandBuffer& currentCmd = {}) {
            this->clearMappedData(currentCmd);
            this->context->descriptorSets[3] = this->context->smpFlip0DescriptorSet;
            uintptr_t I = 0u; for (uint32_t i = 0u; i < this->instanceCounter; i++) {
                auto& Mesh = this->meshes[this->rawInstances[i].instanceId];
                Mesh->createMappingCommand(currentCmd, glm::uvec4(I = this->rawInstances[i].instanceId, 0u, i, 0u), this->mapFramebuffer);
            };
            vkt::commandBarrier(this->driver->getDeviceDispatch(), currentCmd);
            return uTHIS;
        };

        // 
        protected: virtual uPTR(Node) rasterizeGeometry(const VkCommandBuffer& currentCmd = {}) {
            const auto clearValues = std::vector<vkh::VkClearValue>{
                 {.color = vkh::VkClearColorValue{.float32 = glm::vec4(0.f, 0.f, 0.f, 0.0f)} },
                 {.color = vkh::VkClearColorValue{.float32 = glm::vec4(0.f, 0.f, 0.f, 0.0f)} },
                 {.color = vkh::VkClearColorValue{.float32 = glm::vec4(0.f, 0.f, 0.f, 0.0f)} },
                 {.color = vkh::VkClearColorValue{.float32 = glm::vec4(0.f, 0.f, 0.f, 0.0f)} },
                 {.color = vkh::VkClearColorValue{.float32 = glm::vec4(0.f, 0.f, 0.f, 0.0f)} },
                 {.color = vkh::VkClearColorValue{.float32 = glm::vec4(0.f, 0.f, 0.f, 0.0f)} },
                 {.color = vkh::VkClearColorValue{.float32 = glm::vec4(0.f, 0.f, 0.f, 0.0f)} },
                 {.color = vkh::VkClearColorValue{.float32 = glm::vec4(0.f, 0.f, 0.f, 0.0f)} },
                 {.depthStencil = vkh::VkClearDepthStencilValue{.depth = 1.0f, .stencil = 0} }
            };

            // 
            this->driver->getDeviceDispatch()->CmdClearDepthStencilImage(currentCmd, this->context->depthImage, this->context->depthImage.getImageLayout(), clearValues[8u].depthStencil, 1u, this->context->depthImage.getImageSubresourceRange());
            for (uint32_t i = 0; i < 8u; i++) {
                this->driver->getDeviceDispatch()->CmdClearColorImage(currentCmd, this->context->rastersImages[i], this->context->rastersImages[i].getImageLayout(), clearValues[i].color, 1u, this->context->rastersImages[i].getImageSubresourceRange());
            };
            vkt::commandBarrier(this->driver->getDeviceDispatch(), currentCmd);

            //
            this->context->descriptorSets[3] = this->context->smpFlip0DescriptorSet;
            //uintptr_t I = 0u; for (uint32_t i = 0u; i < this->instanceCounter; i++) {
            uintptr_t I = 0u; for (uint32_t i = 0u; i < this->instanceCounter; i++) {
                auto& Mesh = this->meshes[this->rawInstances[i].instanceId];
                Mesh->createRasterizeCommand(currentCmd, glm::uvec4(I = this->rawInstances[i].instanceId, 0u, i, 0u), true);
            };
            vkt::commandBarrier(this->driver->getDeviceDispatch(), currentCmd);

            // 
            return uTHIS;
        };

        // 
        protected: virtual uPTR(Node) copyMeta(const VkCommandBuffer& copyCommand = {}) { // 
            this->mapMeshData(copyCommand); // Needs to Mapping! NOW!

            auto& I = this->instanceCounter; I = 0u; // Selection Only Accounted Chunks
            for (auto i = 0; i < std::min(this->prepareInstances.size(), this->rawInstances.size()); i++) {
                auto& Instance = this->prepareInstances[i]; auto& Mesh = this->meshes[Instance.instanceId];
                if (Mesh->fullGeometryCount > 0 && Mesh->mapCount > 0) { this->rawInstances[I++] = Instance; };
            };

            // 
            for (uint32_t i = 0; i < this->meshes.size(); i++) { auto& mesh = this->meshes[i];
                driver->getDeviceDispatch()->CmdCopyBuffer(copyCommand, mesh->rawMeshInfo, this->gpuMeshInfo, 1u, vkh::VkBufferCopy{ mesh->rawMeshInfo.offset(), this->gpuMeshInfo.offset() + mesh->rawMeshInfo.range() * i, mesh->rawMeshInfo.range() });
            };
            vkt::commandBarrier(this->driver->getDeviceDispatch(), copyCommand);

            // 
            return uTHIS;
        };

        // INCOMPATIBLE WITH OPENGL! UNKNOWN REASON! F&CKING NVIDIA! PIDORS! PROBABLY, HARDWARE PROBLEMS... 
        protected: virtual uPTR(Node) buildAccelerationStructure(const VkCommandBuffer& buildCommand = {}) {
            const auto hasAlloc = this->gpuScratchBuffer.has();
            if (!this->accelerationStructure || !hasAlloc || (hasAlloc && !this->gpuScratchBuffer.buffer())) { this->createAccelerationStructure(); };

            // 
            this->buildGeometryAccelerationStructure(buildCommand);

            // Copy ONLY when building! And only when needs... 
            driver->getDeviceDispatch()->CmdCopyBuffer(buildCommand, this->rawInstances, this->gpuInstances, 1u, vkh::VkBufferCopy{ .srcOffset = this->rawInstances.offset(), .dstOffset = this->gpuInstances.offset(), .size = this->gpuInstances.range() });
            vkt::commandBarrier(this->driver->getDeviceDispatch(), buildCommand);

            // 
            this->offsetsInfo[0u].primitiveCount = this->instanceCounter;
            this->instancHeadInfo.dstAccelerationStructure = this->accelerationStructure;
            this->instancHeadInfo.ppGeometries = (this->instancPtr = this->instancInfo.data()).ptr();
            this->instancHeadInfo.scratchData = this->gpuScratchBuffer;
            this->instancHeadInfo.geometryCount = this->instancInfo.size();
            this->instancHeadInfo.update = this->needsUpdate;

            // 
            if (this->needsUpdate) {
                this->instancHeadInfo.srcAccelerationStructure = this->instancHeadInfo.dstAccelerationStructure;
            };

            // 
            const auto ptr = (this->offsetsPtr = this->offsetsInfo.data()).ptr<VkAccelerationStructureBuildOffsetInfoKHR*>();

            // 
            if (buildCommand) { // OpenGL Compatibility Finally Broken!
                //vkh::VkAccelerationStructureInfoNV info = {};
                //info.instanceCount = this->offsetsInfo[0].primitiveCount;
                //info.type = VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_NV;
                //info.flags = VkBuildAccelerationStructureFlagBitsNV(VK_BUILD_ACCELERATION_STRUCTURE_ALLOW_UPDATE_BIT_NV | VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_NV);
                //this->driver->getDeviceDispatch()->CmdBuildAccelerationStructureNV(buildCommand, info, gpuInstances.buffer(), gpuInstances.offset(), this->needsUpdate, this->accelerationStructure, {}, this->gpuScratchBuffer.buffer(), this->gpuScratchBuffer.offset());

                //vkt::debugLabel(buildCommand, "Begin building top acceleration structure...", this->driver->getDispatch());
#if !defined(FORCE_RAY_TRACING) && defined(ENABLE_OPENGL_INTEROP)
                if (!this->needsUpdate)
#endif
                {
                    driver->getDeviceDispatch()->CmdBuildAccelerationStructureKHR(buildCommand, 1u, this->instancHeadInfo, ptr); // INCOMPATIBLE WITH OPENGL, DUE PGM! (TOP-LEVELS)
                    //this->needsUpdate = true; // BROKEN UPDATE?!
                };
                vkt::commandBarrier(this->driver->getDeviceDispatch(), buildCommand);
                //vkt::debugLabel(buildCommand, "Ending building top acceleration structure...", this->driver->getDispatch());
            } else {
                vkh::handleVk(driver->getDeviceDispatch()->BuildAccelerationStructureKHR(1u, this->instancHeadInfo, ptr));
            };

            return uTHIS;
        };

        // Create Or Rebuild Acceleration Structure
        protected: virtual uPTR(Node) createAccelerationStructure() { // Re-assign instance count
            this->topCreate.maxGeometryCount = this->topDataCreate.size();
            this->topCreate.pGeometryInfos = this->topDataCreate.data();

            //
            auto bflgs = vkh::VkBufferCreateFlags{};
            vkt::unlock32(bflgs) = 0u;

            // 
            if (!this->accelerationStructure) { // create acceleration structure fastly...
                vkh::handleVk(driver->getDeviceDispatch()->CreateAccelerationStructureKHR(this->topCreate, nullptr, &this->accelerationStructure));

                //
                vkh::VkMemoryRequirements2 requirements = {};
                driver->getDeviceDispatch()->GetAccelerationStructureMemoryRequirementsKHR(vkh::VkAccelerationStructureMemoryRequirementsInfoKHR{
                    .type = VK_ACCELERATION_STRUCTURE_MEMORY_REQUIREMENTS_TYPE_OBJECT_KHR,
                    .buildType = VK_ACCELERATION_STRUCTURE_BUILD_TYPE_DEVICE_KHR,
                    .accelerationStructure = this->accelerationStructure
                }, requirements);

                // TODO: fix memoryProperties issue
                auto usage = vkh::VkBufferUsageFlags{.eTransferDst = 1, .eStorageTexelBuffer = 1, .eStorageBuffer = 1, .eVertexBuffer = 1, .eSharedDeviceAddress = 1 };
                TempBuffer = vkt::Vector<uint8_t>(std::make_shared<vkt::VmaBufferAllocation>(this->driver->getAllocator(), vkh::VkBufferCreateInfo{
                    .flags = bflgs, .size = requirements.memoryRequirements.size, .usage = usage,
                }, vkt::VmaMemoryInfo{ .memUsage = VMA_MEMORY_USAGE_GPU_ONLY, .deviceDispatch = this->driver->getDeviceDispatch() }));

                // 
                vkh::handleVk(driver->getDeviceDispatch()->BindAccelerationStructureMemoryKHR(1u, vkh::VkBindAccelerationStructureMemoryInfoKHR{
                    .accelerationStructure = this->accelerationStructure,
                    .memory = TempBuffer->getAllocationInfo().memory,
                    .memoryOffset = TempBuffer->getAllocationInfo().offset,
                }));
            };

            // 
            if (!this->gpuScratchBuffer.has()) { // 
                vkh::VkMemoryRequirements2 requirements = {};
                driver->getDeviceDispatch()->GetAccelerationStructureMemoryRequirementsKHR(vkh::VkAccelerationStructureMemoryRequirementsInfoKHR{
                    .type = VK_ACCELERATION_STRUCTURE_MEMORY_REQUIREMENTS_TYPE_BUILD_SCRATCH_KHR,
                    .buildType = VK_ACCELERATION_STRUCTURE_BUILD_TYPE_DEVICE_KHR,
                    .accelerationStructure = this->accelerationStructure
                }, requirements);

                //
                auto usage = vkh::VkBufferUsageFlags{ .eStorageBuffer = 1, .eRayTracing = 1, .eSharedDeviceAddress = 1 };
                this->gpuScratchBuffer = vkt::Vector<uint8_t>(std::make_shared<vkt::VmaBufferAllocation>(driver->getAllocator(), vkh::VkBufferCreateInfo{
                    .flags = bflgs, .size = requirements.memoryRequirements.size, .usage = usage
                }, vkt::VmaMemoryInfo{ .memUsage = VMA_MEMORY_USAGE_GPU_ONLY, .deviceDispatch = this->driver->getDeviceDispatch() }));
            };

            // 
            //this->mapMeshData();
            return uTHIS;
        };

    protected: // 
        std::vector<vkt::uni_ptr<MeshBinding>> meshes = {}; // Mesh list as Template for Instances
        std::vector<uint32_t> mapMeshes = {};
        uintptr_t MaxInstanceCount = 64u;


        // 
        vkt::Vector<uint32_t> mapData = {};
        vkt::ImageRegion mapImage = {};
        vkt::ImageRegion colImage = {};
        vkt::ImageRegion depImage = {};
        VkFramebuffer mapFramebuffer = VK_NULL_HANDLE;

        // 
        std::vector<vkh::VsGeometryInstance> prepareInstances = {};
        vkt::Vector<vkh::VsGeometryInstance> rawInstances = {}; // Ray-Tracing instances Will re-located into meshes by Index, and will no depending by mesh list...
        vkt::Vector<vkh::VsGeometryInstance> gpuInstances = {};
        uint32_t instanceCounter = 0u;
        bool needsUpdate = false;
        bool descriptorUpdatedM = false;
        bool descriptorUpdatedB = false;

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
        VkDescriptorSet meshDataDescriptorSet = VK_NULL_HANDLE;
        VkDescriptorSet bindingsDescriptorSet = VK_NULL_HANDLE;
        VkAccelerationStructureKHR accelerationStructure = VK_NULL_HANDLE;
        vkt::Vector<uint8_t> gpuScratchBuffer = {};
        VmaAllocationInfo allocationInfo = {};
        VmaAllocation allocation = VK_NULL_HANDLE;
        
        // 
        vkt::uni_ptr<Driver> driver = {};
        vkt::uni_ptr<Thread> thread = {};
        vkt::uni_ptr<Context> context = {};
    };
#endif

};
