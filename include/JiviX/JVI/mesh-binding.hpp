#pragma once // #

#include "./config.hpp"
#include "./driver.hpp"
#include "./thread.hpp"
#include "./context.hpp"
#include "./mesh-input.hpp"

namespace jvi {

    // WIP Mesh Object
    // Sub-Instances Can Be Supported
    // TODO: Descriptor Sets
    class MeshBinding : public std::enable_shared_from_this<MeshBinding> { public: friend Node; friend Renderer;
        MeshBinding() {};
        MeshBinding(const vkt::uni_ptr<Context>& context, vk::DeviceSize MaxPrimitiveCount = MAX_PRIM_COUNT, vk::DeviceSize MaxStride = DEFAULT_STRIDE) : context(context), MaxPrimitiveCount(MaxPrimitiveCount), MaxStride(MaxStride){ this->construct(); };
        ~MeshBinding() {};

        // 
        virtual vkt::uni_ptr<MeshBinding> sharedPtr() { return shared_from_this(); };
        virtual vkt::uni_ptr<const MeshBinding> sharedPtr() const { return shared_from_this(); };

        // 
        virtual uPTR(MeshBinding) construct() {
            this->driver = context->getDriver();
            this->thread = std::make_shared<Thread>(this->driver);

            // 
            this->pipelineInfo = vkh::VsGraphicsPipelineCreateInfoConstruction();

            // create required buffers
            this->rawBindings = vkt::Vector<VkVertexInputBindingDescription>(this->driver->getAllocator(), vkh::VkBufferCreateInfo{ .size = sizeof(VkVertexInputBindingDescription) * 8u, .usage = {.eTransferSrc = 1, .eUniformBuffer = 1, .eStorageBuffer = 1, .eRayTracing = 1 } }, VMA_MEMORY_USAGE_CPU_TO_GPU);
            this->gpuBindings = vkt::Vector<VkVertexInputBindingDescription>(this->driver->getAllocator(), vkh::VkBufferCreateInfo{ .size = sizeof(VkVertexInputBindingDescription) * 8u, .usage = {.eTransferDst = 1, .eUniformBuffer = 1, .eStorageBuffer = 1, .eRayTracing = 1 } }, VMA_MEMORY_USAGE_GPU_ONLY);
            this->rawAttributes = vkt::Vector<VkVertexInputAttributeDescription>(this->driver->getAllocator(), vkh::VkBufferCreateInfo{ .size = sizeof(VkVertexInputAttributeDescription) * 8u, .usage = {.eTransferSrc = 1, .eUniformBuffer = 1, .eStorageBuffer = 1, .eRayTracing = 1 } }, VMA_MEMORY_USAGE_CPU_TO_GPU);
            this->gpuAttributes = vkt::Vector<VkVertexInputAttributeDescription>(this->driver->getAllocator(), vkh::VkBufferCreateInfo{ .size = sizeof(VkVertexInputAttributeDescription) * 8u, .usage = {.eTransferDst = 1, .eUniformBuffer = 1, .eStorageBuffer = 1, .eRayTracing = 1 } }, VMA_MEMORY_USAGE_GPU_ONLY);

            // ALPHA_TEST
            this->offsetIndirect = vkt::Vector<uint64_t>(this->driver->getAllocator(), vkh::VkBufferCreateInfo{ .size = 16u, .usage = { .eTransferDst = 1, .eUniformBuffer = 1, .eStorageBuffer = 1, .eIndirectBuffer = 1, .eRayTracing = 1, .eTransformFeedbackCounterBuffer = 1 } }, VMA_MEMORY_USAGE_GPU_ONLY);

            /*
            thread->submitOnce([&, this](vk::CommandBuffer& cmd) {
                cmd.fillBuffer(this->offsetIndirect, 0u, 4u, 1u);
                cmd.fillBuffer(this->offsetIndirect, 4u, 12u, 0u);
            });*/

            // 
            //this->gpuMeshInfo = vkt::Vector<MeshInfo>(this->driver->getAllocator(), vkh::VkBufferCreateInfo{ .size = 16u, .usage = {.eTransferDst = 1, .eUniformBuffer = 1, .eStorageBuffer = 1, .eRayTracing = 1 } }, VMA_MEMORY_USAGE_GPU_ONLY);
            this->rawMeshInfo = vkt::Vector<MeshInfo>(this->driver->getAllocator(), vkh::VkBufferCreateInfo{ .size = 16u, .usage = {.eTransferSrc = 1, .eUniformBuffer = 1, .eStorageBuffer = 1, .eRayTracing = 1 } }, VMA_MEMORY_USAGE_CPU_TO_GPU);

            // Internal Instance Map Per Global Node
            this->rawInstanceMap = vkt::Vector<uint32_t>(this->driver->getAllocator(), vkh::VkBufferCreateInfo{ .size = 4u * 64u, .usage = {.eTransferSrc = 1, .eUniformBuffer = 1, .eStorageBuffer = 1, .eRayTracing = 1 } }, VMA_MEMORY_USAGE_CPU_TO_GPU);
            this->gpuInstanceMap = vkt::Vector<uint32_t>(this->driver->getAllocator(), vkh::VkBufferCreateInfo{ .size = 4u * 64u, .usage = {.eTransferDst = 1, .eUniformBuffer = 1, .eStorageBuffer = 1, .eRayTracing = 1 } }, VMA_MEMORY_USAGE_GPU_ONLY);

            // for faster code, pre-initialize
            this->stages = vkt::vector_cast<vkh::VkPipelineShaderStageCreateInfo, vk::PipelineShaderStageCreateInfo>({
                vkt::makePipelineStageInfo(this->driver->getDevice(), vkt::readBinary("./shaders/rtrace/rasterize.vert.spv"), vk::ShaderStageFlagBits::eVertex),
                vkt::makePipelineStageInfo(this->driver->getDevice(), vkt::readBinary("./shaders/rtrace/rasterize.frag.spv"), vk::ShaderStageFlagBits::eFragment)
            });
            this->ctages = vkt::vector_cast<vkh::VkPipelineShaderStageCreateInfo, vk::PipelineShaderStageCreateInfo>({
                vkt::makePipelineStageInfo(this->driver->getDevice(), vkt::readBinary("./shaders/rtrace/covergence.vert.spv"), vk::ShaderStageFlagBits::eVertex),
                vkt::makePipelineStageInfo(this->driver->getDevice(), vkt::readBinary("./shaders/rtrace/covergence.frag.spv"), vk::ShaderStageFlagBits::eFragment)
            });

            { //
                this->indexData = vkt::Vector<uint8_t>(this->driver->getAllocator(), vkh::VkBufferCreateInfo{
                    .size = MaxPrimitiveCount * sizeof(uint32_t) * 3u,
                    .usage = {.eTransferDst = 1, .eStorageTexelBuffer = 1, .eStorageBuffer = 1, .eIndexBuffer = 1, .eSharedDeviceAddress = 1 },
                });
                this->rawMeshInfo[0u].indexType = uint32_t(vk::IndexType::eNoneKHR) + 1u;

                // TODO: other platforms memory handling
                // create OpenGL version of buffers
#ifdef ENABLE_OPENGL_INTEROP
                glCreateBuffers(1u, &this->indexDataOGL.second);
                glCreateMemoryObjectsEXT(1u, &this->indexDataOGL.first);
                glImportMemoryWin32HandleEXT(this->indexDataOGL.first, this->indexData->getAllocationInfo().reqSize, GL_HANDLE_TYPE_OPAQUE_WIN32_EXT, this->indexData->getAllocationInfo().handle);
                glNamedBufferStorageMemEXT(this->indexDataOGL.second, MaxPrimitiveCount * sizeof(uint32_t) * 3u, this->indexDataOGL.first, 0u);
#endif
            };

            // 
            //for (uint32_t i = 0; i < 8; i++) {
            for (uint32_t i = 0; i < 1; i++) {
                this->bindings[i] = vkt::Vector<uint8_t>(this->driver->getAllocator(), vkh::VkBufferCreateInfo{
                    .size = MaxPrimitiveCount * MaxStride * 3u,
                    .usage = {.eTransferDst = 1, .eStorageTexelBuffer = 1, .eStorageBuffer = 1, .eVertexBuffer = 1, .eTransformFeedbackBuffer = 1, .eSharedDeviceAddress = 1 },
                });

                // TODO: other platforms memory handling
                // create OpenGL version of buffers
#ifdef ENABLE_OPENGL_INTEROP
                glCreateBuffers(1u, &this->bindingsOGL[i].second);
                glCreateMemoryObjectsEXT(1u, &this->bindingsOGL[i].first);
                glImportMemoryWin32HandleEXT(this->bindingsOGL[i].first, this->bindings[i]->getAllocationInfo().reqSize, GL_HANDLE_TYPE_OPAQUE_WIN32_EXT, this->bindings[i]->getAllocationInfo().handle);
                glNamedBufferStorageMemEXT(this->bindingsOGL[i].second, MaxPrimitiveCount * 3u * sizeof(uint32_t), this->bindingsOGL[i].first, 0u);
#endif
            };

            // FOR BUILD! 
            this->bdHeadInfo = vkh::VkAccelerationStructureBuildGeometryInfoKHR{};
            this->bdHeadInfo.geometryCount = this->buildGInfo.size();
            this->bdHeadInfo.ppGeometries = reinterpret_cast<vkh::VkAccelerationStructureGeometryKHR**>((this->buildGPtr = this->buildGInfo.data()).ptr());
            this->bdHeadInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;
            this->bdHeadInfo.flags = VK_BUILD_ACCELERATION_STRUCTURE_ALLOW_UPDATE_BIT_KHR | VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR;
            this->bdHeadInfo.geometryArrayOfPointers = false; // MARK TRUE FOR INDIRECT BUILDING!!

            // FOR BUILD! FULL GEOMETRY INFO! // originally, it should to be array (like as old version of LancER)
            this->buildGTemp = vkh::VkAccelerationStructureGeometryKHR{ .flags = {.eOpaque = 1 } };
            this->buildGTemp = vkh::VkAccelerationStructureGeometryTrianglesDataKHR{
                .vertexFormat = VK_FORMAT_R32G32B32_SFLOAT, 
                .vertexStride = MaxStride,
                .indexType = VK_INDEX_TYPE_NONE_KHR,
            };
            this->offsetTemp = vkh::VkAccelerationStructureBuildOffsetInfoKHR{
                .primitiveCount = 1u,
                .primitiveOffset = 0u,
                .transformOffset = 0u
            };

            // FOR CREATE! 
            this->bottomDataCreate[0u].geometryType = this->buildGTemp.geometryType;
            this->bottomDataCreate[0u].maxVertexCount = static_cast<uint32_t>(MaxPrimitiveCount * 3u);
            this->bottomDataCreate[0u].maxPrimitiveCount = static_cast<uint32_t>(MaxPrimitiveCount);
            this->bottomDataCreate[0u].indexType = VK_INDEX_TYPE_NONE_KHR;
            this->bottomDataCreate[0u].vertexFormat = VK_FORMAT_R32G32B32_SFLOAT;
            this->bottomDataCreate[0u].allowsTransforms = true;

            // FOR CREATE! 
            this->bottomCreate.maxGeometryCount = this->bottomDataCreate.size();
            this->bottomCreate.pGeometryInfos = this->bottomDataCreate.data();
            this->bottomCreate.type = this->bdHeadInfo.type;
            this->bottomCreate.flags = this->bdHeadInfo.flags;

            // Generate Default Layout
            this->addBinding(vkh::VkVertexInputBindingDescription{ .stride = static_cast<uint32_t>(MaxStride) });
            this->addAttribute(vkh::VkVertexInputAttributeDescription{ .location = 0u, .format = VK_FORMAT_R32G32B32A32_SFLOAT, .offset = 0u });  // Positions
            this->addAttribute(vkh::VkVertexInputAttributeDescription{ .location = 1u, .format = VK_FORMAT_R32G32B32A32_SFLOAT, .offset = 16u }); // Texcoords
            this->addAttribute(vkh::VkVertexInputAttributeDescription{ .location = 2u, .format = VK_FORMAT_R32G32B32A32_SFLOAT, .offset = 32u }); // Normals
            this->addAttribute(vkh::VkVertexInputAttributeDescription{ .location = 3u, .format = VK_FORMAT_R32G32B32A32_SFLOAT, .offset = 48u }); // Tangents
            this->addAttribute(vkh::VkVertexInputAttributeDescription{ .location = 4u, .format = VK_FORMAT_R32G32B32A32_SFLOAT, .offset = 64u }); // BiNormals

            // 
            return uTHIS;
        };

        // 
        virtual vkt::Vector<uint8_t>& getBindingBuffer(const uintptr_t& i = 0u) {
            return this->bindings[i];
        };

        //
        virtual vkt::Vector<uint8_t>& getIndexBuffer() {
            return this->indexData;
        };

        // 
        virtual const vkt::Vector<uint8_t>& getBindingBuffer(const uintptr_t& i = 0u) const {
            return this->bindings[i];
        };

        //
        virtual const vkt::Vector<uint8_t>& getIndexBuffer() const {
            return this->indexData;
        };

#ifdef ENABLE_OPENGL_INTEROP //
        virtual GLuint& getBindingBufferGL(const uintptr_t& i = 0u) { return this->bindingsOGL[i].second; };
        virtual const GLuint& getBindingBufferGL(const uintptr_t& i = 0u) const { return this->bindingsOGL[i].second; };

        // 
        virtual GLuint& getIndexBufferGL() { return this->indexDataOGL.second; };
        virtual const GLuint& getIndexBufferGL() const { return this->indexDataOGL.second; };
#endif

        // Win32 Only (currently)
        virtual HANDLE& getBindingMemoryHandle(const uintptr_t& i = 0u) {
            return this->bindings[i]->info.handle;
        };

        // Win32 Only (currently)
        virtual HANDLE& getIndexMemoryHandle() {
            return this->indexData->info.handle;
        };

        // Win32 Only (currently)
        virtual const HANDLE& getBindingMemoryHandle(const uintptr_t& i = 0u) const {
            return this->bindings[i]->info.handle;
        };

        // Win32 Only (currently)
        virtual const HANDLE& getIndexMemoryHandle() const {
            return this->indexData->info.handle;
        };

        // 
        virtual uPTR(MeshBinding) setMaterialID(const uint32_t& materialID = 0u) {
            this->rawMeshInfo[0u].materialID = materialID;
            return uTHIS;
        };

        // 
        virtual uPTR(MeshBinding) setThread(const vkt::uni_ptr<Thread>& thread) {
            this->thread = thread;
            return uTHIS;
        };

        // TODO: Add QUADs support for GEN-2.0
        virtual uPTR(MeshBinding) setIndexCount(const uint32_t& count = 65536u * 3u) { this->setPrimitiveCount(std::min(uint32_t(vkt::tiled(count, 3u)), uint32_t(this->MaxPrimitiveCount))); return uTHIS; };
        virtual uPTR(MeshBinding) setPrimitiveCount(const uint32_t& count = 65536u) { this->primitiveCount = this->offsetTemp.primitiveCount = std::min(uint32_t(count), uint32_t(this->MaxPrimitiveCount)); return uTHIS; };

        // 
        virtual uPTR(MeshBinding) setDriver(const vkt::uni_ptr<Driver>& driver = {}){
            this->driver = driver;
            return uTHIS;
        };

        // 
        virtual uPTR(MeshBinding) increaseInstanceCount(const uint32_t& mapID = 0u, const uint32_t& instanceCount = 1u) {
            this->rawInstanceMap[this->instanceCount] = mapID;
            this->instanceCount += instanceCount;
            return uTHIS;
        };

        // 
        virtual uPTR(MeshBinding) setInstanceCount(const uint32_t& instanceCount = 1u) {
            this->instanceCount = instanceCount;
            return uTHIS;
        };

        // MORE useful for instanced data
        virtual uPTR(MeshBinding) setTransformData(const vkh::VkDescriptorBufferInfo& transformData = {}, const uint32_t& stride = sizeof(glm::mat3x4)) {
            this->offsetTemp.transformOffset = transformData.offset;
            this->buildGTemp.geometry.triangles.transformData = transformData.buffer;
            this->transformStride = stride; // used for instanced correction
            this->rawMeshInfo[0u].hasTransform = 1u;
            return uTHIS;
        };

        // 
        template<class T = uint8_t>
        inline uPTR(MeshBinding) addBinding(const vkt::uni_arg<vkh::VkVertexInputBindingDescription>& binding = vkh::VkVertexInputBindingDescription{}) {
            const uintptr_t bindingID = 0u;//this->vertexInputBindingDescriptions.size();
            this->vertexInputBindingDescriptions.resize(bindingID + 1u);
            this->vertexInputBindingDescriptions[bindingID] = binding;
            this->vertexInputBindingDescriptions[bindingID].binding = bindingID;
            this->rawBindings[bindingID] = this->vertexInputBindingDescriptions[bindingID];
            return uTHIS;
        };

        // 
        virtual uPTR(MeshBinding) addAttribute(const vkt::uni_arg<vkh::VkVertexInputAttributeDescription>& attribute = vkh::VkVertexInputAttributeDescription{}, const bool& NotStub = true) {
            //const uintptr_t bindingID = attribute.binding;
            //const uintptr_t locationID = this->locationCounter++;
            const uintptr_t bindingID = this->lastBindID;
            const uintptr_t locationID = attribute->location;
            this->vertexInputAttributeDescriptions.resize(locationID + 1u);
            this->vertexInputAttributeDescriptions[locationID] = attribute;
            this->vertexInputAttributeDescriptions[locationID].binding = static_cast<uint32_t>(bindingID);
            this->vertexInputAttributeDescriptions[locationID].location = static_cast<uint32_t>(locationID);
            this->rawAttributes[locationID] = this->vertexInputAttributeDescriptions[locationID];

            if (locationID == 0u && NotStub) { // 
                const auto& bindingData = this->vertexInputBindingDescriptions[bindingID];
                const auto& bindingBuffer = this->bindings[bindingID];

                // 
                this->offsetTemp.primitiveOffset = bindingBuffer.offset() + attribute->offset; // !!WARNING!!
                this->buildGTemp.geometryType = VK_GEOMETRY_TYPE_TRIANGLES_KHR;
                this->buildGTemp.geometry.triangles.vertexFormat = this->bottomDataCreate[0].vertexFormat = attribute->format;
                this->buildGTemp.geometry.triangles.vertexStride = bindingData.stride;
                this->buildGTemp.geometry.triangles.vertexData = bindingBuffer;
                this->buildGTemp.flags = { .eOpaque = 1 };

                // Fix vec4 formats into vec3, without alpha (but still can be passed by stride value)
                if (attribute->format == VK_FORMAT_R32G32B32A32_SFLOAT) this->buildGTemp.geometry.triangles.vertexFormat = VK_FORMAT_R32G32B32_SFLOAT;
                if (attribute->format == VK_FORMAT_R16G16B16A16_SFLOAT) this->buildGTemp.geometry.triangles.vertexFormat = VK_FORMAT_R16G16B16_SFLOAT;
            };

            return uTHIS;
        };

        // 
        virtual uPTR(MeshBinding) copyBuffers(const vk::CommandBuffer& buildCommand = {}) {
            buildCommand.copyBuffer(this->rawAttributes , this->gpuAttributes , { vk::BufferCopy{ this->rawAttributes .offset(), this->gpuAttributes .offset(), this->gpuAttributes .range() } });
            buildCommand.copyBuffer(this->rawBindings   , this->gpuBindings   , { vk::BufferCopy{ this->rawBindings   .offset(), this->gpuBindings   .offset(), this->gpuBindings   .range() } });
            buildCommand.copyBuffer(this->rawInstanceMap, this->gpuInstanceMap, { vk::BufferCopy{ this->rawInstanceMap.offset(), this->gpuInstanceMap.offset(), this->gpuInstanceMap.range() } });
            return uTHIS;
        };

        // TODO: Add QUADs support for GEN-2.0
        virtual uPTR(MeshBinding) buildGeometry(const vk::CommandBuffer& buildCommand = {}, const glm::uvec4& meshData = glm::uvec4(0u)) { // build geometry data
            if (this->input) {
                this->input->createRasterizePipeline()->buildGeometry(this->bindings[0u], buildCommand, meshData);
                this->setIndexCount(this->input->currentUnitCount);
            };
            return uTHIS;
        };

        // TODO: Add QUADs support for GEN-2.0
        virtual uPTR(MeshBinding) buildAccelerationStructure(const vk::CommandBuffer& buildCommand = {}, const glm::uvec4& meshData = glm::uvec4(0u)) {
            if (!this->accelerationStructure) { this->createAccelerationStructure(); };

            // 
            //this->offsetTemp.primitiveCount = this->primitiveCount;

            // 
            this->offsetInfo[0] = this->offsetTemp;
            this->buildGInfo[0] = this->buildGTemp;

            // 
            this->bdHeadInfo.geometryCount = this->buildGInfo.size();
            this->bdHeadInfo.dstAccelerationStructure = this->accelerationStructure;
            this->bdHeadInfo.ppGeometries = (this->buildGPtr = this->buildGInfo.data()).ptr();
            this->bdHeadInfo.scratchData = this->gpuScratchBuffer;
            this->bdHeadInfo.update = this->needsUpdate;

            // 
            if (buildCommand) {
                vkt::debugLabel(buildCommand, "Begin building bottom acceleration structure...", this->driver->getDispatch());
                buildCommand.buildAccelerationStructureKHR(1u, this->bdHeadInfo, reinterpret_cast<vk::AccelerationStructureBuildOffsetInfoKHR**>((this->offsetPtr = this->offsetInfo.data()).ptr()), this->driver->getDispatch()); this->needsUpdate = true;

                // INDIRECT UNSUPPORTED!!
                buildCommand.updateBuffer<uint64_t>(this->offsetIndirect.buffer(), this->offsetIndirect.offset(), std::array<uint64_t, 1> { this->input->counterData.deviceAddress() }, this->driver->getDispatch());
                buildCommand.buildAccelerationStructureIndirectKHR(this->bdHeadInfo.hpp(), this->offsetIndirect.buffer(), this->offsetIndirect.offset(), this->offsetIndirect.stride(), this->driver->getDispatch());
                vkt::debugLabel(buildCommand, "Ending building bottom acceleration structure...", this->driver->getDispatch());
            }
            else {
                driver->getDevice().buildAccelerationStructureKHR(1u, this->bdHeadInfo, reinterpret_cast<vk::AccelerationStructureBuildOffsetInfoKHR**>((this->offsetPtr = this->offsetInfo.data()).ptr()), this->driver->getDispatch());
            };

            //
            return uTHIS;
        };

        // TODO: Add QUADs support for GEN-2.0
        virtual uPTR(MeshBinding) bindMeshInput(const vkt::uni_ptr<MeshInput>& input = {}) {
            (this->input = input)->rawMeshInfo = this->rawMeshInfo; // Share Memory
            //this->input->linkCounterBuffer(this->offsetIndirect);
            return uTHIS;
        };

        // Create Or Rebuild Acceleration Structure
        virtual uPTR(MeshBinding) createAccelerationStructure() {
            bool Is2ndFormat = // TODO: more correct length detection
                this->buildGTemp.geometry.triangles.vertexFormat == VK_FORMAT_R32G32_SFLOAT ||
                this->buildGTemp.geometry.triangles.vertexFormat == VK_FORMAT_R16G16B16_SFLOAT ||
                this->buildGTemp.geometry.triangles.vertexFormat == VK_FORMAT_R16G16_SFLOAT;

            // 
            if (!this->accelerationStructure) { // create acceleration structure fastly...
                this->accelerationStructure = this->driver->getDevice().createAccelerationStructureKHR(this->bottomCreate, nullptr, this->driver->getDispatch());

                //
                auto requirements = this->driver->getDevice().getAccelerationStructureMemoryRequirementsKHR(vkh::VkAccelerationStructureMemoryRequirementsInfoKHR{
                    .type = VK_ACCELERATION_STRUCTURE_MEMORY_REQUIREMENTS_TYPE_OBJECT_KHR,
                    .buildType = VK_ACCELERATION_STRUCTURE_BUILD_TYPE_DEVICE_KHR,
                    .accelerationStructure = this->accelerationStructure
                }, this->driver->getDispatch());

                // TODO: fix memoryProperties issue
                TempBuffer = vkt::Vector<uint8_t>(this->driver->getAllocator(), vkh::VkBufferCreateInfo{
                    .size = requirements.memoryRequirements.size,
                    .usage = {.eTransferDst = 1, .eStorageTexelBuffer = 1, .eStorageBuffer = 1, .eVertexBuffer = 1, .eSharedDeviceAddress = 1 },
                });

                // 
                this->driver->getDevice().bindAccelerationStructureMemoryKHR(1u,&vkh::VkBindAccelerationStructureMemoryInfoKHR{
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
                this->gpuScratchBuffer = vkt::Vector<uint8_t>(this->driver->getAllocator(), vkh::VkBufferCreateInfo{
                    .size = requirements.memoryRequirements.size,
                    .usage = { .eStorageBuffer = 1, .eRayTracing = 1, .eSharedDeviceAddress = 1 }
                }, VMA_MEMORY_USAGE_GPU_ONLY);
            };

            // 
            return uTHIS;
        };

        // 
        virtual uPTR(MeshBinding) createRasterizePipeline() {
            const auto& viewport = this->context->refViewport();
            const auto& renderArea = this->context->refScissor();

            // TODO: Add to main package
            // Enable Conservative Rasterization For Fix Some Antialiasing Issues
            vk::PipelineRasterizationConservativeStateCreateInfoEXT conserv = {};
            conserv.conservativeRasterizationMode = vk::ConservativeRasterizationModeEXT::eOverestimate;

            // 
            this->pipelineInfo = vkh::VsGraphicsPipelineCreateInfoConstruction();
            this->pipelineInfo.vertexInputAttributeDescriptions = this->vertexInputAttributeDescriptions;
            this->pipelineInfo.vertexInputBindingDescriptions = this->vertexInputBindingDescriptions;
            this->pipelineInfo.stages = this->stages;
            this->pipelineInfo.depthStencilState = vkh::VkPipelineDepthStencilStateCreateInfo{ .depthTestEnable = true, .depthWriteEnable = true };
            this->pipelineInfo.dynamicStates.push_back(VK_DYNAMIC_STATE_VIEWPORT);
            this->pipelineInfo.dynamicStates.push_back(VK_DYNAMIC_STATE_SCISSOR);
            this->pipelineInfo.graphicsPipelineCreateInfo.renderPass = this->context->renderPass;
            this->pipelineInfo.graphicsPipelineCreateInfo.layout = this->context->unifiedPipelineLayout;
            this->pipelineInfo.viewportState.pViewports = &(vkh::VkViewport&)viewport;
            this->pipelineInfo.viewportState.pScissors = &(vkh::VkRect2D&)renderArea;
            //this->pipelineInfo.rasterizationState.pNext = &conserv;

            // 
            for (uint32_t i = 0u; i < 8u; i++) {
                this->pipelineInfo.colorBlendAttachmentStates.push_back(vkh::VkPipelineColorBlendAttachmentState{ .blendEnable = true }); // transparency will generated by ray-tracing
            };

            // 
            this->rasterizationState = driver->getDevice().createGraphicsPipeline(driver->getPipelineCache(), this->pipelineInfo);

            // 
            this->pipelineInfo.rasterizationState.pNext = &conserv;
            this->pipelineInfo.stages = this->ctages;
            this->covergenceState = driver->getDevice().createGraphicsPipeline(driver->getPipelineCache(), this->pipelineInfo);

            // 
            return uTHIS;
        };

        // Create Secondary Command With Pipeline
        virtual uPTR(MeshBinding) createRasterizeCommand(const vk::CommandBuffer& rasterCommand = {}, const glm::uvec4& meshData = glm::uvec4(0u), const bool& conservative = false) { // UNIT ONLY!
            if (this->instanceCount <= 0u) return uTHIS;

            // 
            std::vector<vk::Buffer> buffers = {}; std::vector<vk::DeviceSize> offsets = {};
            buffers.resize(this->bindings.size()); offsets.resize(this->bindings.size()); uintptr_t I = 0u;
            for (auto& B : this->bindings) { if (B.has()) { const uintptr_t i = I++; buffers[i] = B.buffer(); offsets[i] = B.offset(); }; };

            // 
            const auto& viewport = this->context->refViewport();
            const auto& renderArea = this->context->refScissor();
            const auto clearValues = std::vector<vk::ClearValue>{
                vk::ClearColorValue(std::array<float,4>{0.f, 0.f, 0.f, 0.0f}),
                vk::ClearColorValue(std::array<float,4>{0.f, 0.f, 0.f, 0.0f}),
                vk::ClearColorValue(std::array<float,4>{0.f, 0.f, 0.f, 0.0f}),
                vk::ClearColorValue(std::array<float,4>{0.f, 0.f, 0.f, 0.0f}),
                vk::ClearColorValue(std::array<float,4>{0.f, 0.f, 0.f, 0.0f}),
                vk::ClearColorValue(std::array<float,4>{0.f, 0.f, 0.f, 0.0f}),
                vk::ClearColorValue(std::array<float,4>{0.f, 0.f, 0.f, 0.0f}),
                vk::ClearColorValue(std::array<float,4>{0.f, 0.f, 0.f, 0.0f}),
                vk::ClearDepthStencilValue(1.0f, 0)
            };

            // covergence
            vkt::debugLabel(rasterCommand, "Begin rasterization...", this->driver->getDispatch());
            rasterCommand.beginRenderPass(vk::RenderPassBeginInfo(this->context->refRenderPass(), this->context->deferredFramebuffer, renderArea, static_cast<uint32_t>(clearValues.size()), clearValues.data()), vk::SubpassContents::eInline);
            rasterCommand.setViewport(0, { viewport });
            rasterCommand.setScissor(0, { renderArea });
            rasterCommand.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, this->context->unifiedPipelineLayout, 0ull, this->context->descriptorSets, {});
            rasterCommand.bindPipeline(vk::PipelineBindPoint::eGraphics, conservative ? this->covergenceState : this->rasterizationState);
            rasterCommand.bindVertexBuffers(0u, buffers, offsets);
            rasterCommand.pushConstants<glm::uvec4>(this->context->unifiedPipelineLayout, vkh::VkShaderStageFlags{.eVertex = 1, .eGeometry = 1, .eFragment = 1, .eCompute = 1, .eRaygen = 1, .eClosestHit = 1, .eMiss = 1 }.hpp(), 0u, { meshData });
            rasterCommand.draw((this->rawMeshInfo[0u].prmCount = this->primitiveCount) * 3ull, this->instanceCount, this->offsetTemp.firstVertex, 0u);
            rasterCommand.endRenderPass();
            vkt::debugLabel(rasterCommand, "End rasterization...", this->driver->getDispatch());

            // 
            return uTHIS;
        };

    // 
    protected: friend MeshBinding; friend Node; friend Renderer; // GPU Vertex and Attribute Data
        vkt::Vector<uint8_t> indexData = {}; 
#ifdef ENABLE_OPENGL_INTEROP
        std::pair<GLuint, GLuint> indexDataOGL = {};
#endif

        // 
        //vk::IndexType indexType = vk::IndexType::eNoneKHR;
        vk::DeviceSize MaxPrimitiveCount = MAX_PRIM_COUNT, MaxStride = DEFAULT_STRIDE;

        // 
        uint32_t primitiveCount = 0u, instanceCount = 0u;

        // 
        std::array<vkt::Vector<uint8_t>, 1> bindings = {};
#ifdef ENABLE_OPENGL_INTEROP
        std::array<std::pair<GLuint, GLuint>, 1> bindingsOGL = {};
#endif
        std::array<uint32_t, 1> bindRange = {0};

        // 
        std::vector<vkh::VkVertexInputAttributeDescription> vertexInputAttributeDescriptions = {};
        vkt::Vector<vkh::VkVertexInputAttributeDescription> rawAttributes = {};
        vkt::Vector<vkh::VkVertexInputAttributeDescription> gpuAttributes = {};

        // 
        std::vector<vkh::VkVertexInputBindingDescription> vertexInputBindingDescriptions = {};
        vkt::Vector<vkh::VkVertexInputBindingDescription> rawBindings = {};
        vkt::Vector<vkh::VkVertexInputBindingDescription> gpuBindings = {};

        // 
        std::vector<vkh::VkPipelineShaderStageCreateInfo> stages = {};
        std::vector<vkh::VkPipelineShaderStageCreateInfo> ctages = {};

        // accumulated by "Instance" for instanced rendering
        vkt::Vector<glm::mat3x4> gpuTransformData = {};
        uint32_t transformStride = sizeof(glm::mat3x4);
        uint32_t lastBindID = 0u, locationCounter = 0u;
        bool needsUpdate = false;

        // 
        vkh::VsGraphicsPipelineCreateInfoConstruction pipelineInfo = {};


        // FOR CREATE (Acceleration Structure)
        vkh::VkAccelerationStructureCreateInfoKHR                          bottomCreate = {}; // CREATE SINGLE!
        std::vector<vkh::VkAccelerationStructureCreateGeometryTypeInfoKHR> bottomDataCreate = { {} }; // CAN BE MULTIPLE!

        // FOR BUILD! BUT ONLY SINGLE! (Contain Multiple-Instanced)
        vkh::VkAccelerationStructureBuildGeometryInfoKHR                  bdHeadInfo = {};
        //std::vector<vkh::VkAccelerationStructureBuildGeometryInfoKHR>   bdHeadInfo = { {} };
        //vkt::uni_arg<vkh::VkAccelerationStructureBuildGeometryInfoKHR*> bdHeadPtr = {};

        // CAN BE MULTIPLE! (single element of array, array of array[0])
        vkt::Vector<uint64_t>                                         offsetIndirect {};
        std::vector<vkh::VkAccelerationStructureBuildOffsetInfoKHR>   offsetInfo = { {} };
        vkt::uni_arg<vkh::VkAccelerationStructureBuildOffsetInfoKHR*> offsetPtr = {};
        vkh::VkAccelerationStructureBuildOffsetInfoKHR                offsetTemp = {}; // INSTANCE TEMPLATE, CAN'T BE ARRAY! 

        // CAN BE MULTIPLE! (single element of array, array of array[0])
        std::vector<vkh::VkAccelerationStructureGeometryKHR>   buildGInfo = { {} };
        vkt::uni_arg<vkh::VkAccelerationStructureGeometryKHR*> buildGPtr = {};
        vkh::VkAccelerationStructureGeometryKHR                buildGTemp = {}; // INSTANCE TEMPLATE, CAN'T BE ARRAY! 


        // 
        vk::Pipeline rasterizationState = {}; // Vertex Input can changed, so use individual rasterization stages
        vk::Pipeline covergenceState = {};

        // 
        vkt::Vector<uint8_t> TempBuffer = {};
        vkt::Vector<uint8_t> gpuScratchBuffer = {};

        // 
        vkt::Vector<MeshInfo> rawMeshInfo = {};
        vkt::Vector<uint32_t> rawInstanceMap = {};
        vkt::Vector<uint32_t> gpuInstanceMap = {};

        // 
        vk::AccelerationStructureKHR accelerationStructure = {};
        VmaAllocationInfo allocationInfo = {};
        VmaAllocation allocation = {};

        // 
        vkt::uni_ptr<Driver> driver = {};
        vkt::uni_ptr<Thread> thread = {};
        vkt::uni_ptr<Context> context = {};
        vkt::uni_ptr<MeshInput> input = {}; // Currently, Single! 
        //vkt::uni_ptr<Renderer> renderer = {};
    };

};
