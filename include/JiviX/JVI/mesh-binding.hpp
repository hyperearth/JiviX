#pragma once // #

#include "./config.hpp"
#include "./driver.hpp"
#include "./thread.hpp"
#include "./context.hpp"
#include "./mesh-input.hpp"

namespace jvi {

#if defined(ENABLE_OPENGL_INTEROP) && !defined(VKT_USE_GLAD)
    using namespace gl;
#endif

    // WIP Mesh Object
    // Sub-Instances Can Be Supported
    // TODO: Descriptor Sets
    class MeshBinding : public std::enable_shared_from_this<MeshBinding> { public: friend Node; friend Renderer; friend MeshInput;
        MeshBinding() {};
        MeshBinding(const std::shared_ptr<Context>& context, VkDeviceSize MaxPrimitiveCount = MAX_PRIM_COUNT, std::vector<VkDeviceSize> GeometryInitial = {}) : context(context), MaxPrimitiveCount(MaxPrimitiveCount), GeometryInitial(GeometryInitial) { this->construct(); };
        MeshBinding(const std::shared_ptr<Context>& context, VkDeviceSize MaxPrimitiveCount, std::vector<int64_t> GeometryInitial) : context(context), MaxPrimitiveCount(MaxPrimitiveCount), GeometryInitial(vkt::vector_cast<VkDeviceSize>(GeometryInitial)) { this->construct(); };
        MeshBinding(const vkt::uni_ptr<Context>& context, VkDeviceSize MaxPrimitiveCount = MAX_PRIM_COUNT, std::vector<VkDeviceSize> GeometryInitial = {}) : context(context), MaxPrimitiveCount(MaxPrimitiveCount), GeometryInitial(GeometryInitial) { this->construct(); };
        MeshBinding(const vkt::uni_ptr<Context>& context, VkDeviceSize MaxPrimitiveCount, std::vector<int64_t> GeometryInitial) : context(context), MaxPrimitiveCount(MaxPrimitiveCount), GeometryInitial(vkt::vector_cast<VkDeviceSize>(GeometryInitial)) { this->construct(); };
        ~MeshBinding() {};

        // 
        public: virtual vkt::uni_ptr<MeshBinding> sharedPtr() { return shared_from_this(); };
        //virtual vkt::uni_ptr<MeshBinding> sharedPtr() const { return std::shared_ptr<MeshBinding>(shared_from_this()); };

        // 
        protected: virtual uPTR(MeshBinding) construct() {
            this->driver = context->getDriver();
            this->thread = std::make_shared<Thread>(this->driver);

            // 
            if (this->GeometryInitial.size() <= 0ull) { this->GeometryInitial.push_back(MaxPrimitiveCount); };

            // 
            this->pipelineInfo = vkh::VsGraphicsPipelineCreateInfoConstruction();

            // create required buffers
            this->rawBindings = vkt::Vector<vkh::VkVertexInputBindingDescription>(std::make_shared<vkt::VmaBufferAllocation>(this->driver->getAllocator(), vkh::VkBufferCreateInfo{ .size = sizeof(vkh::VkVertexInputBindingDescription) * 8u, .usage = {.eTransferSrc = 1, .eTransferDst = 1, .eUniformBuffer = 1, .eStorageBuffer = 1, .eRayTracing = 1 } }, vkt::VmaMemoryInfo{ .memUsage = VMA_MEMORY_USAGE_CPU_TO_GPU }));
            this->gpuBindings = vkt::Vector<vkh::VkVertexInputBindingDescription>(std::make_shared<vkt::VmaBufferAllocation>(this->driver->getAllocator(), vkh::VkBufferCreateInfo{ .size = sizeof(vkh::VkVertexInputBindingDescription) * 8u, .usage = {.eTransferSrc = 1, .eTransferDst = 1, .eUniformBuffer = 1, .eStorageBuffer = 1, .eRayTracing = 1 } }, vkt::VmaMemoryInfo{ .memUsage = VMA_MEMORY_USAGE_GPU_ONLY }));
            this->rawAttributes = vkt::Vector<vkh::VkVertexInputAttributeDescription>(std::make_shared<vkt::VmaBufferAllocation>(this->driver->getAllocator(), vkh::VkBufferCreateInfo{ .size = sizeof(vkh::VkVertexInputAttributeDescription) * 8u, .usage = {.eTransferSrc = 1, .eUniformBuffer = 1, .eStorageBuffer = 1, .eRayTracing = 1 } }, vkt::VmaMemoryInfo{ .memUsage = VMA_MEMORY_USAGE_CPU_TO_GPU }));
            this->gpuAttributes = vkt::Vector<vkh::VkVertexInputAttributeDescription>(std::make_shared<vkt::VmaBufferAllocation>(this->driver->getAllocator(), vkh::VkBufferCreateInfo{ .size = sizeof(vkh::VkVertexInputAttributeDescription) * 8u, .usage = {.eTransferDst = 1, .eUniformBuffer = 1, .eStorageBuffer = 1, .eRayTracing = 1 } }, vkt::VmaMemoryInfo{ .memUsage = VMA_MEMORY_USAGE_GPU_ONLY }));
            this->rawMaterialIDs = vkt::Vector<uint32_t>(std::make_shared<vkt::VmaBufferAllocation>(this->driver->getAllocator(), vkh::VkBufferCreateInfo{ .size = sizeof(uint32_t) * GeometryInitial.size(), .usage = {.eTransferSrc = 1, .eUniformBuffer = 1, .eStorageBuffer = 1, .eRayTracing = 1 } }, vkt::VmaMemoryInfo{ .memUsage = VMA_MEMORY_USAGE_CPU_TO_GPU }));
            this->gpuMaterialIDs = vkt::Vector<uint32_t>(std::make_shared<vkt::VmaBufferAllocation>(this->driver->getAllocator(), vkh::VkBufferCreateInfo{ .size = sizeof(uint32_t) * GeometryInitial.size(), .usage = {.eTransferDst = 1, .eUniformBuffer = 1, .eStorageBuffer = 1, .eRayTracing = 1 } }, vkt::VmaMemoryInfo{ .memUsage = VMA_MEMORY_USAGE_GPU_ONLY }));

            // 
            this->counterData = vkt::Vector<glm::uvec4>(std::make_shared<vkt::VmaBufferAllocation>(this->driver->getAllocator(), vkh::VkBufferCreateInfo{ .size = sizeof(uint32_t) * 4u, .usage = { .eTransferSrc = 1, .eTransferDst = 1, .eUniformBuffer = 1, .eStorageBuffer = 1, .eIndirectBuffer = 1, .eRayTracing = 1, .eTransformFeedbackBuffer = 1, .eTransformFeedbackCounterBuffer = 1, .eSharedDeviceAddress = 1 } }, vkt::VmaMemoryInfo{ .memUsage = VMA_MEMORY_USAGE_GPU_ONLY }));

            // 
            this->offsetCounterData = vkt::Vector<uint32_t>(std::make_shared<vkt::VmaBufferAllocation>(this->driver->getAllocator(), vkh::VkBufferCreateInfo{ .size = vkt::tiled((std::max(uint64_t(GeometryInitial.size()), uint64_t(64ull))+uint64_t(1u)), uint64_t(2ull)) * 8u, .usage = { .eTransferSrc = 1, .eTransferDst = 1, .eUniformBuffer = 1, .eStorageBuffer = 1, .eIndirectBuffer = 1, .eRayTracing = 1, .eTransformFeedbackBuffer = 1, .eTransformFeedbackCounterBuffer = 1, .eSharedDeviceAddress = 1 } }, vkt::VmaMemoryInfo{ .memUsage = VMA_MEMORY_USAGE_GPU_TO_CPU }));

            // 
            //this->gpuMeshInfo = vkt::Vector<MeshInfo>(this->driver->getAllocator(), vkh::VkBufferCreateInfo{ .size = 16u, .usage = {.eTransferDst = 1, .eUniformBuffer = 1, .eStorageBuffer = 1, .eRayTracing = 1 } }, VMA_MEMORY_USAGE_GPU_ONLY);
            this->rawMeshInfo = vkt::Vector<MeshInfo>(std::make_shared<vkt::VmaBufferAllocation>(this->driver->getAllocator(), vkh::VkBufferCreateInfo{ .size = 16u, .usage = {.eTransferSrc = 1, .eUniformBuffer = 1, .eStorageBuffer = 1, .eRayTracing = 1 } }, vkt::VmaMemoryInfo{ .memUsage = VMA_MEMORY_USAGE_CPU_TO_GPU }));

            // Internal Instance Map Per Global Node
            this->rawInstanceMap = vkt::Vector<uint32_t>(std::make_shared<vkt::VmaBufferAllocation>(this->driver->getAllocator(), vkh::VkBufferCreateInfo{ .size = uint64_t(std::max(static_cast<uint64_t>(GeometryInitial.size()), uint64_t(64ull)) * sizeof(uint32_t)), .usage = {.eTransferSrc = 1, .eUniformBuffer = 1, .eStorageBuffer = 1, .eRayTracing = 1 } }, vkt::VmaMemoryInfo{ .memUsage = VMA_MEMORY_USAGE_CPU_TO_GPU }));
            this->gpuInstanceMap = vkt::Vector<uint32_t>(std::make_shared<vkt::VmaBufferAllocation>(this->driver->getAllocator(), vkh::VkBufferCreateInfo{ .size = uint64_t(std::max(static_cast<uint64_t>(GeometryInitial.size()), uint64_t(64ull)) * sizeof(uint32_t)), .usage = {.eTransferDst = 1, .eUniformBuffer = 1, .eStorageBuffer = 1, .eRayTracing = 1 } }, vkt::VmaMemoryInfo{ .memUsage = VMA_MEMORY_USAGE_GPU_ONLY }));

            // for faster code, pre-initialize
            this->stages = {
                vkt::makePipelineStageInfo(this->driver->getDeviceDispatch(), vkt::readBinary(std::string("./shaders/rtrace/rasterize.vert.spv")), VK_SHADER_STAGE_VERTEX_BIT),
                vkt::makePipelineStageInfo(this->driver->getDeviceDispatch(), vkt::readBinary(std::string("./shaders/rtrace/rasterize.frag.spv")), VK_SHADER_STAGE_FRAGMENT_BIT)
            };

            // 
            this->ctages = {
                vkt::makePipelineStageInfo(this->driver->getDeviceDispatch(), vkt::readBinary(std::string("./shaders/rtrace/covergence.vert.spv")), VK_SHADER_STAGE_VERTEX_BIT),
                vkt::makePipelineStageInfo(this->driver->getDeviceDispatch(), vkt::readBinary(std::string("./shaders/rtrace/covergence.frag.spv")), VK_SHADER_STAGE_FRAGMENT_BIT)
            };

            // 
            this->mtages = {
                vkt::makePipelineStageInfo(this->driver->getDeviceDispatch(), vkt::readBinary(std::string("./shaders/rtrace/mapping.vert.spv")), VK_SHADER_STAGE_VERTEX_BIT),
                vkt::makePipelineStageInfo(this->driver->getDeviceDispatch(), vkt::readBinary(std::string("./shaders/rtrace/mapping.geom.spv")), VK_SHADER_STAGE_GEOMETRY_BIT),
                vkt::makePipelineStageInfo(this->driver->getDeviceDispatch(), vkt::readBinary(std::string("./shaders/rtrace/mapping.frag.spv")), VK_SHADER_STAGE_FRAGMENT_BIT)
            };

            // 
            auto& almac = driver->memoryAllocationInfo();
            almac.memUsage = VMA_MEMORY_USAGE_GPU_ONLY;
            almac.glMemory = almac.glID = 0u;

            // 
            for (uint32_t i = 0; i < this->bindings.size(); i++) {
                this->bindings[i] = vkt::Vector<uint8_t>(std::make_shared<vkt::BufferAllocation>(vkh::VkBufferCreateInfo{
                    .size = (i == 0 ? MaxPrimitiveCount : 1u) * (i == 0 ? MaxStride : sizeof(glm::vec4)) * 3u,
                    .usage = {.eTransferSrc = 1, .eTransferDst = 1, .eStorageTexelBuffer = 1, .eStorageBuffer = 1, .eVertexBuffer = 1, .eTransformFeedbackBuffer = 1, .eSharedDeviceAddress = 1 },
                }, almac));

                // For OpenGL!
#ifdef ENABLE_OPENGL_INTEROP
                this->bindingsOGL[i].first = this->bindings[i].getGLMemory();
                this->bindingsOGL[i].second = this->bindings[i].getGLBuffer();
#endif
            };

            // FOR BUILD!
            auto bdHeadFlags = vkh::VkBuildAccelerationStructureFlagsKHR{ .eAllowUpdate = 1, .ePreferFastTrace = 1 };
            this->bdHeadInfo = vkh::VkAccelerationStructureBuildGeometryInfoKHR{};
            this->bdHeadInfo.geometryCount = this->buildGInfo.size();
            this->bdHeadInfo.ppGeometries = nullptr;//reinterpret_cast<vkh::VkAccelerationStructureGeometryKHR**>((this->buildGPtr = this->buildGInfo.data()).ptr());
            this->bdHeadInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;
            this->bdHeadInfo.flags = bdHeadFlags;
            this->bdHeadInfo.geometryArrayOfPointers = true; // MARK TRUE FOR INDIRECT BUILDING!!

            // FOR BUILD! FULL GEOMETRY INFO! // originally, it should to be array (like as old version of LancER)
            auto buildGeometryFlags = vkh::VkGeometryFlagsKHR{.eOpaque = 1, .eNoDuplicateAnyHitInvocation = 1 };
            this->buildGTemp = vkh::VkAccelerationStructureGeometryKHR{ .flags = buildGeometryFlags }; // Optimize Tracing
            this->buildGTemp = vkh::VkAccelerationStructureGeometryTrianglesDataKHR{
                .vertexFormat = VK_FORMAT_R32G32B32_SFLOAT, 
                .vertexStride = MaxStride,
                .indexType = VK_INDEX_TYPE_NONE_KHR,
            };

            // FOR CREATE! 
            this->bottomDataCreate.resize(GeometryInitial.size()); uintptr_t I = 0ull;
            for (auto& BC : this->bottomDataCreate) {
                BC.geometryType = this->buildGTemp.geometryType;
                BC.maxVertexCount = static_cast<uint32_t>(GeometryInitial[I] * 3u);
                BC.maxPrimitiveCount = static_cast<uint32_t>(GeometryInitial[I]);
                BC.indexType = VK_INDEX_TYPE_NONE_KHR;
                BC.vertexFormat = VK_FORMAT_R32G32B32_SFLOAT;
                BC.allowsTransforms = true; I++;
            };

            // FOR CREATE! 
            this->bottomCreate.maxGeometryCount = this->bottomDataCreate.size();
            this->bottomCreate.pGeometryInfos = this->bottomDataCreate.data();
            this->bottomCreate.type = this->bdHeadInfo.type;
            this->bottomCreate.flags = this->bdHeadInfo.flags;

            // Generate Default Layout
            this->setBinding(vkh::VkVertexInputBindingDescription{ .binding = 0, .stride = static_cast<uint32_t>(MaxStride) });
            this->setAttribute(vkh::VkVertexInputAttributeDescription{ .location = 0u, .format = VK_FORMAT_R32G32B32A32_SFLOAT, .offset = 0u });  // Positions
            this->setAttribute(vkh::VkVertexInputAttributeDescription{ .location = 1u, .format = VK_FORMAT_R32G32B32A32_SFLOAT, .offset = 16u }); // Texcoords
            this->setAttribute(vkh::VkVertexInputAttributeDescription{ .location = 2u, .format = VK_FORMAT_R32G32B32A32_SFLOAT, .offset = 32u }); // Normals
            this->setAttribute(vkh::VkVertexInputAttributeDescription{ .location = 3u, .format = VK_FORMAT_R32G32B32A32_SFLOAT, .offset = 48u }); // Tangents
            this->setAttribute(vkh::VkVertexInputAttributeDescription{ .location = 4u, .format = VK_FORMAT_R32G32B32A32_SFLOAT, .offset = 64u }); // BiNormals

            // FOR QUADS RESERVED!
            //this->setBinding(vkh::VkVertexInputBindingDescription{ .binding = 1, .stride = sizeof(glm::vec4) });
            //this->setAttribute(vkh::VkVertexInputAttributeDescription{ .location = 0u, .binding = 1, .format = VK_FORMAT_R32G32B32A32_SFLOAT, .offset = 0u });  // Positions

            // 
            const glm::uvec4 initialCount = glm::uvec4(0u);
            this->thread->submitOnce([&](const VkCommandBuffer& cmdbuf) {
                this->driver->getDeviceDispatch()->CmdUpdateBuffer(cmdbuf, counterData.buffer(), counterData.offset(), sizeof(glm::uvec4), &initialCount);
            });

            // 
            return uTHIS;
        };

        // 
        public: virtual uPTR(MeshBinding) setThread(const vkt::uni_ptr<Thread>& thread) {
            this->thread = thread;
            return uTHIS;
        };

        // 
        public: virtual uPTR(MeshBinding) setDriver(const vkt::uni_ptr<Driver>& driver = {}) {
            this->driver = driver;
            return uTHIS;
        };

        //
        public: virtual uPTR(MeshBinding) setThread(const std::shared_ptr<Thread>& thread) { return this->setThread(vkt::uni_ptr<Thread>(thread)); };
        public: virtual uPTR(MeshBinding) setDriver(const std::shared_ptr<Driver>& driver) { return this->setDriver(vkt::uni_ptr<Driver>(driver)); };

        // 
        public: virtual vkt::Vector<uint8_t>& getBindingBuffer(const uintptr_t& i = 0u) {
            return this->bindings[i];
        };

        //
        public: virtual vkt::Vector<uint8_t>& getIndexBuffer() {
            return this->indexData;
        };

        // 
        public: virtual const vkt::Vector<uint8_t>& getBindingBuffer(const uintptr_t& i = 0u) const {
            return this->bindings[i];
        };

        //
        public: virtual const vkt::Vector<uint8_t>& getIndexBuffer() const {
            return this->indexData;
        };

#ifdef ENABLE_OPENGL_INTEROP //
        public: virtual GLuint& getBindingBufferGL(const uintptr_t& i = 0u) { return this->bindingsOGL[i].second; };
        public: virtual const GLuint& getBindingBufferGL(const uintptr_t& i = 0u) const { return this->bindingsOGL[i].second; };

        // 
        public: virtual GLuint& getIndexBufferGL() { return this->indexDataOGL.second; };
        public: virtual const GLuint& getIndexBufferGL() const { return this->indexDataOGL.second; };

        // Win32 Only (currently)
        protected: virtual HANDLE& getBindingMemoryHandle(const uintptr_t& i = 0u) {
            return this->bindings[i]->info.handle;
        };

        // Win32 Only (currently)
        protected: virtual HANDLE& getIndexMemoryHandle() {
            return this->indexData->info.handle;
        };

        // Win32 Only (currently)
        protected: virtual const HANDLE& getBindingMemoryHandle(const uintptr_t& i = 0u) const {
            return this->bindings[i]->info.handle;
        };

        // Win32 Only (currently)
        protected: virtual const HANDLE& getIndexMemoryHandle() const {
            return this->indexData->info.handle;
        };

        // OBSOLETE! Needs Instance Based Material ID!
        //virtual uPTR(MeshBinding) setFirstMaterial(const uint32_t& materialID = 0u) {
        //    this->rawMeshInfo[0u].materialID = materialID;
        //    return uTHIS;
        //};
#endif

        // 
        public: virtual uPTR(MeshBinding) manifestIndex(const VkIndexType& type = VK_INDEX_TYPE_NONE_KHR) {
            this->rawMeshInfo[0u].indexType = uint32_t(this->buildGTemp.geometry.triangles.indexType = VkIndexType(type)) + 1u;
            return uTHIS;
        };

        // TODO: Add QUADs support for GEN-2.0
        public: virtual uPTR(MeshBinding) setIndexCount(const VkDeviceSize& count = 65536u * 3u) { this->setPrimitiveCount(std::min(VkDeviceSize(vkt::tiled(uint64_t(count), uint64_t(3ull))), VkDeviceSize(this->MaxPrimitiveCount))); return uTHIS; };
        public: virtual uPTR(MeshBinding) setPrimitiveCount(const VkDeviceSize& count = 65536u) { this->primitiveCount = std::min(VkDeviceSize(count), VkDeviceSize(this->MaxPrimitiveCount)); return uTHIS; };



        // 
        protected: virtual uPTR(MeshBinding) linkWithInstance(const uint32_t& mapID = 0u) {
            if (this->mapCount < this->rawInstanceMap.size() && this->fullGeometryCount > 0u) {
                this->rawInstanceMap[this->mapCount++] = mapID;
            };
            return uTHIS;
        };

        // DEPRECATED
        protected: virtual uPTR(MeshBinding) increaseGeometryCount(const uint32_t& geometryCount = 1u) {
            //this->geometryCount += geometryCount;
            return uTHIS;
        };

        // DEPRECATED
        public: virtual uPTR(MeshBinding) setGeometryCount(const uint32_t& geometryCount = 1u) {
            //this->geometryCount = geometryCount;
            return uTHIS;
        };

        // 
        public: virtual uPTR(MeshBinding) setTransformData(const vkt::uni_arg<vkt::Vector<glm::mat3x4>>& transformData = {}, const uint32_t& stride = sizeof(glm::mat3x4)) {
            this->offsetTemp.transformOffset = transformData->offset(); //!!
            this->buildGTemp.geometry.triangles.transformData = transformData->deviceAddress();
            this->transformStride = stride; // used for instanced correction
            this->rawMeshInfo[0u].hasTransform = 1u;
            return uTHIS;
        };

        // FOR INTERNAL USAGE ONLY!! 
        //template<class T = uint8_t>
        protected: inline uPTR(MeshBinding) setBinding(vkt::uni_arg<vkh::VkVertexInputBindingDescription> binding = vkh::VkVertexInputBindingDescription{}) {
            const uintptr_t bindingID = binding->binding;
            this->vertexInputBindingDescriptions.resize(bindingID + 1u);
            this->vertexInputBindingDescriptions[bindingID] = binding;
            this->vertexInputBindingDescriptions[bindingID].binding = bindingID;
            this->rawBindings[bindingID] = this->vertexInputBindingDescriptions[bindingID];
            return uTHIS;
        };

        // FOR INTERNAL USAGE ONLY!! 
        protected: virtual uPTR(MeshBinding) setAttribute(vkt::uni_arg<vkh::VkVertexInputAttributeDescription> attribute = vkh::VkVertexInputAttributeDescription{}, const bool& NotStub = true) {
            const uintptr_t bindingID = attribute->binding;
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
                auto bflags = vkh::VkGeometryFlagsKHR{ .eOpaque = 1 };
                this->offsetTemp.primitiveOffset = bindingBuffer.offset() + attribute->offset; // !!WARNING!!
                this->buildGTemp.geometryType = VK_GEOMETRY_TYPE_TRIANGLES_KHR;
                this->buildGTemp.geometry.triangles.vertexFormat = attribute->format;
                this->buildGTemp.geometry.triangles.vertexStride = bindingData.stride;
                this->buildGTemp.geometry.triangles.vertexData = bindingBuffer;
                this->buildGTemp.flags = bflags;

                // 
                for (auto& CR : this->bottomDataCreate) {
                    CR.vertexFormat = attribute->format;
                };

                // Fix vec4 formats into vec3, without alpha (but still can be passed by stride value)
                if (attribute->format == VK_FORMAT_R32G32B32A32_SFLOAT) this->buildGTemp.geometry.triangles.vertexFormat = VK_FORMAT_R32G32B32_SFLOAT;
                if (attribute->format == VK_FORMAT_R16G16B16A16_SFLOAT) this->buildGTemp.geometry.triangles.vertexFormat = VK_FORMAT_R16G16B16_SFLOAT;
            };

            return uTHIS;
        };

        // 
        protected: virtual uPTR(MeshBinding) copyBuffers(const vkt::uni_arg<VkCommandBuffer>& buildCommand = {}) {
            const auto vect0 = glm::uvec4(0u);
            this->driver->getDeviceDispatch()->CmdCopyBuffer(buildCommand, this->rawAttributes, this->gpuAttributes, 1u, vkh::VkBufferCopy{ this->rawAttributes.offset(), this->gpuAttributes.offset(), this->gpuAttributes.range() });
            this->driver->getDeviceDispatch()->CmdCopyBuffer(buildCommand, this->rawBindings, this->gpuBindings, 1u, vkh::VkBufferCopy{ this->rawBindings.offset(), this->gpuBindings.offset(), this->gpuBindings.range() });
            this->driver->getDeviceDispatch()->CmdCopyBuffer(buildCommand, this->rawInstanceMap, this->gpuInstanceMap, 1u, vkh::VkBufferCopy{ this->rawInstanceMap.offset(), this->gpuInstanceMap.offset(), this->gpuInstanceMap.range() });
            this->driver->getDeviceDispatch()->CmdCopyBuffer(buildCommand, this->rawMaterialIDs, this->gpuMaterialIDs, 1u, vkh::VkBufferCopy{ this->rawMaterialIDs.offset(), this->gpuMaterialIDs.offset(), this->gpuMaterialIDs.range() });
            this->driver->getDeviceDispatch()->CmdUpdateBuffer(buildCommand, counterData.buffer(), counterData.offset(), sizeof(glm::uvec4), &vect0);
            if (this->inputs.size() > 0) { for (auto& I : this->inputs) if (I.has()) { I->copyMeta(buildCommand); }; };
            return uTHIS;
        };

        //
        /*virtual uPTR(MeshBinding) buildGeometry(const vkt::uni_arg<VkCommandBuffer>& buildCommand = {}, const vkt::uni_arg<glm::uvec4>& meshData = glm::uvec4(0u)){
            return this->buildGeometry(VkCommandBuffer(*buildCommand), meshData);
        };*/

        // 
        protected: virtual uPTR(MeshBinding) buildGeometry(const vkt::uni_arg<VkCommandBuffer>& buildCommand = {}, const vkt::uni_arg<glm::uvec4>& meshData = glm::uvec4(0u)) { // build geometry data
            if (this->fullGeometryCount <= 0u || this->mapCount <= 0u) return uTHIS;
            //if (this->fullGeometryCount <= 0u) return uTHIS;
            this->primitiveCount = 0u;

            // 
            const glm::uvec4 counterValue = glm::uvec4(0u);
            this->driver->getDeviceDispatch()->CmdUpdateBuffer(buildCommand, this->counterData.buffer(), this->counterData.offset(), sizeof(glm::uvec4), &counterValue);
            vkt::commandBarrier(this->driver->getDeviceDispatch(), buildCommand);

            // 
            if (this->fullGeometryCount > this->offsetInfo.size()) {
                this->offsetInfo.resize(this->fullGeometryCount);
                this->offsetPtr.resize(this->fullGeometryCount);
            };


            // We Collect Counter Buffer Results for future usage...
            this->rawMeshInfo[0].geometryCount = this->fullGeometryCount;
            uint32_t f = 0, i = 0, c = 0; for (auto& I : this->inputs) { // Quads needs to format...
                const auto uOffset = this->primitiveCount * 3u;
                if (I.has()) { I->formatQuads(uTHIS, glm::u64vec4(uOffset, uOffset * DEFAULT_STRIDE, this->fullGeometryCount, 0u), buildCommand); };

                // copy as template, use as triangle...
                auto offsetp = this->offsetTemp;
                {
                    //offsetp.primitiveOffset = uOffset * DEFAULT_STRIDE; //+ this->bindings[0u].offset();
                    offsetp.primitiveCount = I.has() ? vkt::tiled(uint64_t(I->getIndexCount()), uint64_t(3ull)) : vkt::tiled(uint64_t(this->ranges[i]), uint64_t(1ull)); // TODO: De-Facto primitive count...
                    offsetp.firstVertex = uOffset;
                    this->driver->getDeviceDispatch()->CmdCopyBuffer(buildCommand, this->counterData.buffer(), this->offsetCounterData.buffer(), 1u, vkh::VkBufferCopy{ .dstOffset = i * sizeof(uint32_t), .size = sizeof(uint32_t) });
                    vkt::commandBarrier(this->driver->getDeviceDispatch(), buildCommand); // TODO: Transform Feedback Counter Barrier In 
                    if (I.has()) { I->buildGeometry(uTHIS, glm::u64vec4(uOffset, uOffset * DEFAULT_STRIDE, this->fullGeometryCount, 0u), buildCommand); };
                    vkt::commandBarrier(this->driver->getDeviceDispatch(), buildCommand); // TODO: Transform Feedback Counter Barrier Out
                };

                // Needs Un-Use Input Geometry Count?!
                //if (I.has()) { 
                    this->primitiveCount += offsetp.primitiveCount; 
                //};

                // 
                for (uint32_t j = 0; j < this->instances[i]; j++) {
                    this->offsetInfo[c] = offsetp;
                    if (this->rawMeshInfo[0u].hasTransform) { // Polcovnic
                        this->offsetInfo[c].transformOffset = this->transformStride * c;
                    };
                    this->offsetPtr[c] = &this->offsetInfo[c]; c++;
                };

                // 
                f += this->instances[i++];
            };

            // 
            if (this->inputs.size() > 0) {
                this->driver->getDeviceDispatch()->CmdCopyBuffer(buildCommand, this->counterData.buffer(), this->offsetCounterData.buffer(), 1u, vkh::VkBufferCopy{ .dstOffset = (this->inputs.size() - 1u) * sizeof(uint32_t), .size = sizeof(uint32_t) });
            }

            // 
            vkt::commandBarrier(this->driver->getDeviceDispatch(), buildCommand);

            // 
            return uTHIS;
        };

        // TODO: Fix Quads support with Indices
        /*virtual uPTR(MeshBinding) buildAccelerationStructure(const VkCommandBuffer& buildCommand = {}, const vkt::uni_arg<glm::uvec4>& meshData = glm::uvec4(0u)){
            return this->buildAccelerationStructure(VkCommandBuffer(buildCommand), meshData);
        };*/

        // 
        protected: virtual uPTR(MeshBinding) buildAccelerationStructure(const VkCommandBuffer& buildCommand = {}, const vkt::uni_arg<glm::uvec4>& meshData = glm::uvec4(0u)) {
            if (this->fullGeometryCount <= 0u || this->mapCount <= 0u) return uTHIS; // Map BROKEN in here!
            //if (this->fullGeometryCount <= 0u) return uTHIS;
            if (!this->accelerationStructure) { this->createAccelerationStructure(); };

            //std::vector<vkh::VkAccelerationStructureGeometryKHR> ptrs = {};
            //ptrs.push_back(reinterpret_cast<vkh::VkAccelerationStructureGeometryKHR&>(this->buildGInfo[i] = this->buildGTemp));

            // 
            if (this->fullGeometryCount > this->buildGInfo.size()) {
                this->buildGInfo.resize(this->fullGeometryCount);
                this->buildGPtr.resize(this->fullGeometryCount);
                this->offsetInfo.resize(this->fullGeometryCount);
                this->offsetPtr.resize(this->fullGeometryCount);
            };

            // 
            uint32_t f = 0, i = 0, c = 0;  for (auto& I : this->inputs) { // Quads needs to format...
                for (uint32_t j = 0; j < this->instances[i]; j++) {
                    this->buildGInfo[c] = this->buildGTemp;
                    this->offsetPtr[c] = &this->offsetInfo[c];
                    this->buildGPtr[c] = &this->buildGInfo[c]; c++;
                }; f += this->instances[i++];
            };

            // 
            this->bdHeadInfo.geometryCount = this->buildGInfo.size();
            this->bdHeadInfo.dstAccelerationStructure = this->accelerationStructure;
            this->bdHeadInfo.ppGeometries = reinterpret_cast<vkh::VkAccelerationStructureGeometryKHR**>(this->buildGPtr.data());
            this->bdHeadInfo.scratchData = this->gpuScratchBuffer;
            this->bdHeadInfo.update = this->needsUpdate;

            // 
            if (buildCommand) {
                //vkt::debugLabel(buildCommand, "Begin building bottom acceleration structure...", this->driver->getDispatch());
                //buildCommand.buildAccelerationStructureKHR(1u, this->bdHeadInfo, reinterpret_cast<VkAccelerationStructureBuildOffsetInfoKHR**>(this->offsetPtr.data()), this->driver->getDispatch()); this->needsUpdate = true;
                driver->getDeviceDispatch()->CmdBuildAccelerationStructureKHR(buildCommand, 1u, this->bdHeadInfo, reinterpret_cast<VkAccelerationStructureBuildOffsetInfoKHR**>(this->offsetPtr.data())); this->needsUpdate = true;
                //vkt::debugLabel(buildCommand, "Ending building bottom acceleration structure...", this->driver->getDispatch()); this->needsUpdate = true;
            } else {
                //driver->getDevice().buildAccelerationStructureKHR(1u, this->bdHeadInfo, reinterpret_cast<VkAccelerationStructureBuildOffsetInfoKHR**>(this->offsetPtr.data()), this->driver->getDispatch());
                vkh::handleVk(driver->getDeviceDispatch()->BuildAccelerationStructureKHR(1u, this->bdHeadInfo, reinterpret_cast<VkAccelerationStructureBuildOffsetInfoKHR**>(this->offsetPtr.data())));
            };
            vkt::commandBarrier(this->driver->getDeviceDispatch(), buildCommand);

            //
            return uTHIS;
        };


        // 
        public: virtual uPTR(MeshBinding) addRangeInput(const VkDeviceSize& primitiveCount, const uint32_t& materialID = 0u, const VkDeviceSize& instanceCount = 1u) {
            uintptr_t ID = this->inputs.size();
            this->inputs.push_back({}); // Correct! 
            this->ranges.push_back(primitiveCount);
            this->instances.push_back(instanceCount);
            for (uint32_t i = 0; i < instanceCount; i++) { this->rawMaterialIDs[this->fullGeometryCount + uintptr_t(i)] = materialID; }; // TODO: Material ID per instance
            this->fullGeometryCount += instanceCount;
            return uTHIS;
        };

        // Instanced, but with vector of materials
        public: virtual uPTR(MeshBinding) addRangeInput(const VkDeviceSize& primitiveCount, const std::vector<uint32_t>& materialIDs) {
            uintptr_t ID = this->inputs.size();
            this->inputs.push_back({}); // Correct! 
            this->ranges.push_back(primitiveCount);
            this->instances.push_back(materialIDs.size());
            for (uint32_t i = 0; i < materialIDs.size(); i++) { this->rawMaterialIDs[this->fullGeometryCount + uintptr_t(i)] = materialIDs[i]; }; // TODO: Material ID per instance
            this->fullGeometryCount += materialIDs.size();
            return uTHIS;
        };

        // 
        public: virtual uPTR(MeshBinding) addRangeInput(const VkDeviceSize& primitiveCount, const std::vector<int32_t>& materialIDs) {
            return this->addRangeInput(primitiveCount, vkt::vector_cast<uint32_t>(materialIDs));
        };


        // 
        public: virtual uPTR(MeshBinding) addMeshInput(const vkt::uni_ptr<MeshInput>& input, const uint32_t& materialID = 0u, const VkDeviceSize& instanceCount = 1u) {
            uintptr_t ID = this->inputs.size();
            this->inputs.push_back(input); // Correct! 
            this->ranges.push_back(vkt::tiled(uint64_t(input->getIndexCount()), uint64_t(3ull)));
            this->instances.push_back(instanceCount);
            for (uint32_t i = 0; i < instanceCount; i++) { this->rawMaterialIDs[this->fullGeometryCount + uintptr_t(i)] = materialID; }; // TODO: Material ID per instance
            this->fullGeometryCount += instanceCount;
            return uTHIS;
        };

        // Instanced, but with vector of materials
        public: virtual uPTR(MeshBinding) addMeshInput(const vkt::uni_ptr<MeshInput>& input, const std::vector<uint32_t>& materialIDs) {
            uintptr_t ID = this->inputs.size();
            this->inputs.push_back(input); // Correct! 
            this->ranges.push_back(vkt::tiled(uint64_t(input->getIndexCount()), uint64_t(3ull)));
            this->instances.push_back(materialIDs.size());
            for (uint32_t i = 0; i < materialIDs.size(); i++) { this->rawMaterialIDs[this->fullGeometryCount + uintptr_t(i)] = materialIDs[i]; }; // TODO: Material ID per instance
            this->fullGeometryCount += materialIDs.size();
            return uTHIS;
        };

        // 
        public: virtual uPTR(MeshBinding) resetInstanceMap(const uint32_t& mapID = 0u) {
            this->mapCount = 0u;
            return uTHIS;
        };

        // Öбнулись! Nullify Rendering! Made for Minecraft... 
        public: virtual uPTR(MeshBinding) resetGeometry() {
            this->fullGeometryCount = 0ull;
            this->needsUpdate = false;
            this->ranges.resize(0u);
            this->inputs.resize(0u);
            this->instances.resize(0u);
            return uTHIS;
        }

        // 
        public: virtual uPTR(MeshBinding) addMeshInput(const vkt::uni_ptr<MeshInput>& input, const std::vector<int32_t>& materialIDs) {
            return this->addMeshInput(input, vkt::vector_cast<uint32_t>(materialIDs));
        };

        // 
        public: virtual uPTR(MeshBinding) addMeshInput(const std::shared_ptr<MeshInput>& input, const uint32_t& materialID = 0u, const VkDeviceSize& instanceCount = 1u) {
            return this->addMeshInput(vkt::uni_ptr<MeshInput>(input), materialID, instanceCount);
        };

        // 
        public: virtual uPTR(MeshBinding) addMeshInput(const std::shared_ptr<MeshInput>& input, const std::vector<uint32_t>& materialIDs) {
            return this->addMeshInput(vkt::uni_ptr<MeshInput>(input), materialIDs);
        };

        // 
        public: virtual uPTR(MeshBinding) addMeshInput(const std::shared_ptr<MeshInput>& input, const std::vector<int32_t>& materialIDs) {
            return this->addMeshInput(vkt::uni_ptr<MeshInput>(input), materialIDs);
        };


        // Create Or Rebuild Acceleration Structure
        protected: virtual uPTR(MeshBinding) createAccelerationStructure() {
            bool Is2ndFormat = // TODO: more correct length detection
                this->buildGTemp.geometry.triangles.vertexFormat == VK_FORMAT_R32G32_SFLOAT ||
                this->buildGTemp.geometry.triangles.vertexFormat == VK_FORMAT_R16G16B16_SFLOAT ||
                this->buildGTemp.geometry.triangles.vertexFormat == VK_FORMAT_R16G16_SFLOAT;

            // 
            if (!this->accelerationStructure) { // create acceleration structure fastly...
                vkh::handleVk(driver->getDeviceDispatch()->CreateAccelerationStructureKHR(this->bottomCreate, nullptr, &this->accelerationStructure));

                //
                vkh::VkMemoryRequirements2 requirements = {};
                driver->getDeviceDispatch()->GetAccelerationStructureMemoryRequirementsKHR(vkh::VkAccelerationStructureMemoryRequirementsInfoKHR{
                    .type = VK_ACCELERATION_STRUCTURE_MEMORY_REQUIREMENTS_TYPE_OBJECT_KHR,
                    .buildType = VK_ACCELERATION_STRUCTURE_BUILD_TYPE_DEVICE_KHR,
                    .accelerationStructure = this->accelerationStructure
                }, requirements);

                // TODO: fix memoryProperties issue
                TempBuffer = vkt::Vector<uint8_t>(std::make_shared<vkt::VmaBufferAllocation>(this->driver->getAllocator(), vkh::VkBufferCreateInfo{
                    .size = requirements.memoryRequirements.size,
                    .usage = {.eTransferDst = 1, .eStorageTexelBuffer = 1, .eStorageBuffer = 1, .eVertexBuffer = 1, .eSharedDeviceAddress = 1 },
                }, vkt::VmaMemoryInfo{ .memUsage = VMA_MEMORY_USAGE_GPU_ONLY }));

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
                this->gpuScratchBuffer = vkt::Vector<uint8_t>(std::make_shared<vkt::VmaBufferAllocation>(driver->getAllocator(), vkh::VkBufferCreateInfo{
                    .size = requirements.memoryRequirements.size,
                    .usage = {.eStorageBuffer = 1, .eRayTracing = 1, .eSharedDeviceAddress = 1 }
                }, vkt::VmaMemoryInfo{ .memUsage = VMA_MEMORY_USAGE_GPU_ONLY }));
            };

            // 
            return uTHIS;
        };

        // 
        protected: virtual uPTR(MeshBinding) createRasterizePipeline() {
            const auto& viewport = this->context->refViewport();
            const auto& renderArea = this->context->refScissor();

            // TODO: Add to main package
            // Enable Conservative Rasterization For Fix Some Antialiasing Issues
            VkPipelineRasterizationConservativeStateCreateInfoEXT conserv = {};
            conserv.conservativeRasterizationMode = VK_CONSERVATIVE_RASTERIZATION_MODE_OVERESTIMATE_EXT;

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

            // float precision BROKEN!
            this->pipelineInfo.colorBlendAttachmentStates[0u].blendEnable = false;
            this->pipelineInfo.colorBlendAttachmentStates[1u].blendEnable = false;

            // 
            vkh::handleVk(this->driver->getDeviceDispatch()->CreateGraphicsPipelines(driver->getPipelineCache(), 1u, this->pipelineInfo, nullptr, &this->rasterizationState));


            // 
            this->pipelineInfo.rasterizationState.pNext = &conserv;
            this->pipelineInfo.stages = this->ctages;
            vkh::handleVk(this->driver->getDeviceDispatch()->CreateGraphicsPipelines(driver->getPipelineCache(), 1u, this->pipelineInfo, nullptr, &this->covergenceState));


            // 
            this->pipelineInfo.colorBlendAttachmentStates.resize(0u);
            this->pipelineInfo.colorBlendAttachmentStates.push_back(vkh::VkPipelineColorBlendAttachmentState{ .blendEnable = false }); // transparency will generated by ray-tracing
            this->pipelineInfo.graphicsPipelineCreateInfo.renderPass = this->context->mapRenderPass;
            this->pipelineInfo.rasterizationState.pNext = &conserv;
            this->pipelineInfo.stages = this->mtages;
            vkh::handleVk(this->driver->getDeviceDispatch()->CreateGraphicsPipelines(driver->getPipelineCache(), 1u, this->pipelineInfo, nullptr, &this->mapState));


            // 
            return uTHIS;
        };


        // Mapping for Shadows, Emission, Voxels, Ray-Tracing...
        protected: virtual uPTR(MeshBinding) createMappingCommand(const VkCommandBuffer& rasterCommand = {}, const glm::uvec4& meshData = glm::uvec4(0u), const vkt::uni_arg<VkFramebuffer>& framebuffer = {}) { // UNIT ONLY!
            if (this->fullGeometryCount <= 0u || this->mapCount <= 0u) return uTHIS;
            //if (this->fullGeometryCount <= 0u) return uTHIS;

            // 
            if (!this->rasterizationState) {
                this->createRasterizePipeline();
            };

            // 
            std::vector<VkBuffer> buffers = {}; std::vector<VkDeviceSize> offsets = {};
            buffers.resize(this->bindings.size()); offsets.resize(this->bindings.size()); uintptr_t I = 0u;
            for (auto& B : this->bindings) { if (B.has()) { const uintptr_t i = I++; buffers[i] = B.buffer(); offsets[i] = B.offset(); }; };

            // 
            const auto mapWidth = 1024u, mapHeight = 1024u;
            const auto renderArea = vkh::VkRect2D{ vkh::VkOffset2D{0, 0}, vkh::VkExtent2D{mapWidth, mapHeight} };
            const auto viewport = vkh::VkViewport{ 0.0f, 0.0f, static_cast<float>(renderArea.extent.width), static_cast<float>(renderArea.extent.height), 0.f, 1.f };
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

            // covergence
            //vkt::debugLabel(rasterCommand, "Begin rasterization...", this->driver->getDispatch());
            this->driver->getDeviceDispatch()->CmdBindPipeline(rasterCommand, VK_PIPELINE_BIND_POINT_GRAPHICS, this->mapState);
            this->driver->getDeviceDispatch()->CmdBindDescriptorSets(rasterCommand, VK_PIPELINE_BIND_POINT_GRAPHICS, this->context->unifiedPipelineLayout, 0u, this->context->descriptorSets.size(), this->context->descriptorSets.data(), 0u, nullptr);
            this->driver->getDeviceDispatch()->CmdSetViewport(rasterCommand, 0u, 1u, viewport);
            this->driver->getDeviceDispatch()->CmdSetScissor(rasterCommand, 0u, 1u, renderArea);
            this->driver->getDeviceDispatch()->CmdBindVertexBuffers(rasterCommand, 0u, buffers.size(), buffers.data(), offsets.data());

            // 
            uint32_t f = 0, i = 0, c = 0;  for (auto& I : this->inputs) { // Quads needs to format...
                for (uint32_t j = 0; j < this->instances[i]; j++) {
                    const auto meta = glm::uvec4(meshData.x, f, meshData.z, 0u);
                    const auto stage = vkh::VkShaderStageFlags{.eVertex = 1, .eGeometry = 1, .eFragment = 1, .eCompute = 1, .eRaygen = 1, .eClosestHit = 1, .eMiss = 1};
                    this->driver->getDeviceDispatch()->CmdPushConstants(rasterCommand, this->context->unifiedPipelineLayout, stage, 0u, sizeof(meta), & meta);
                    this->driver->getDeviceDispatch()->CmdBeginRenderPass(rasterCommand, vkh::VkRenderPassBeginInfo{ .renderPass = this->context->mapRenderPass, .framebuffer = framebuffer, .renderArea = renderArea, .clearValueCount = static_cast<uint32_t>(clearValues.size()), .pClearValues = clearValues.data() }, VK_SUBPASS_CONTENTS_INLINE);
                    this->driver->getDeviceDispatch()->CmdDraw(rasterCommand, this->offsetInfo[c].primitiveCount * 3u, this->instances[i], this->offsetInfo[c].firstVertex, f);
                    this->driver->getDeviceDispatch()->CmdEndRenderPass(rasterCommand);
                    //vkt::commandBarrier(this->driver->getDeviceDispatch(), rasterCommand);

                    // 
                    this->buildGInfo[c] = this->buildGTemp;
                    this->offsetPtr[c] = &this->offsetInfo[c];
                    this->buildGPtr[c] = &this->buildGInfo[c]; c++;
                }; f += this->instances[i++];
            };

            //vkt::debugLabel(rasterCommand, "End rasterization...", this->driver->getDispatch());

            // 
            return uTHIS;
        };


        // Create Secondary Command With Pipeline
        protected: virtual uPTR(MeshBinding) createRasterizeCommand(const VkCommandBuffer& rasterCommand = {}, const glm::uvec4& meshData = glm::uvec4(0u), const bool& conservative = false) { // UNIT ONLY!
            if (this->fullGeometryCount <= 0u || this->mapCount <= 0u) return uTHIS;
            //if (this->fullGeometryCount <= 0u) return uTHIS;

            // 
            if (!this->rasterizationState) {
                this->createRasterizePipeline();
            };

            // 
            std::vector<VkBuffer> buffers = {}; std::vector<VkDeviceSize> offsets = {};
            buffers.resize(this->bindings.size()); offsets.resize(this->bindings.size()); uintptr_t I = 0u;
            for (auto& B : this->bindings) { if (B.has()) { const uintptr_t i = I++; buffers[i] = B.buffer(); offsets[i] = B.offset(); }; };

            // 
            const auto& viewport = reinterpret_cast<vkh::VkViewport&>(this->context->refViewport());
            const auto& renderArea = reinterpret_cast<vkh::VkRect2D&>(this->context->refScissor());
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

            // covergence
            //vkt::debugLabel(rasterCommand, "Begin rasterization...", this->driver->getDispatch());
            this->driver->getDeviceDispatch()->CmdBindPipeline(rasterCommand, VK_PIPELINE_BIND_POINT_GRAPHICS, conservative ? this->covergenceState : this->rasterizationState);
            this->driver->getDeviceDispatch()->CmdBindDescriptorSets(rasterCommand, VK_PIPELINE_BIND_POINT_GRAPHICS, this->context->unifiedPipelineLayout, 0u, this->context->descriptorSets.size(), this->context->descriptorSets.data(), 0u, nullptr);
            this->driver->getDeviceDispatch()->CmdSetViewport(rasterCommand, 0u, 1u, viewport);
            this->driver->getDeviceDispatch()->CmdSetScissor(rasterCommand, 0u, 1u, renderArea);
            this->driver->getDeviceDispatch()->CmdBindVertexBuffers(rasterCommand, 0u, buffers.size(), buffers.data(), offsets.data());

            uint32_t f = 0, i = 0, c = 0;  for (auto& I : this->inputs) { // Quads needs to format...
                for (uint32_t j = 0; j < this->instances[i]; j++) {
                    const auto meta = glm::uvec4(meshData.x, f, meshData.z, 0u);
                    auto stage = vkh::VkShaderStageFlags{.eVertex = 1, .eGeometry = 1, .eFragment = 1, .eCompute = 1, .eRaygen = 1, .eClosestHit = 1, .eMiss = 1};
                    this->driver->getDeviceDispatch()->CmdPushConstants(rasterCommand, this->context->unifiedPipelineLayout, stage, 0u, sizeof(meta), & meta);
                    this->driver->getDeviceDispatch()->CmdBeginRenderPass(rasterCommand, vkh::VkRenderPassBeginInfo{ .renderPass = this->context->refRenderPass(), .framebuffer = this->context->rasteredFramebuffer, .renderArea = renderArea, .clearValueCount = static_cast<uint32_t>(clearValues.size()), .pClearValues = clearValues.data() }, VK_SUBPASS_CONTENTS_INLINE);
                    this->driver->getDeviceDispatch()->CmdDraw(rasterCommand, this->offsetInfo[c].primitiveCount * 3u, this->instances[i], this->offsetInfo[c].firstVertex, f);
                    this->driver->getDeviceDispatch()->CmdEndRenderPass(rasterCommand);
                    //vkt::commandBarrier(this->driver->getDeviceDispatch(), rasterCommand);

                    // 
                    this->buildGInfo[c] = this->buildGTemp;
                    this->offsetPtr[c] = &this->offsetInfo[c];
                    this->buildGPtr[c] = &this->buildGInfo[c]; c++;
                }; f += this->instances[i++];
            };


            //vkt::debugLabel(rasterCommand, "End rasterization...", this->driver->getDispatch());

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
        //VkIndexType indexType = VkIndexType::eNoneKHR;
        VkDeviceSize MaxPrimitiveCount = MAX_PRIM_COUNT, MaxStride = DEFAULT_STRIDE;
        std::vector<VkDeviceSize> GeometryInitial = { MAX_PRIM_COUNT };

        // `primitiveCount` should to be counter!
        uint32_t primitiveCount = 0u, mapCount = 0u, fullGeometryCount = 0u;

        // 
        std::array<vkt::Vector<uint8_t>, 2> bindings = {};
#ifdef ENABLE_OPENGL_INTEROP
        std::array<std::pair<GLuint, GLuint>, 2> bindingsOGL = {};
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
        std::vector<vkh::VkPipelineShaderStageCreateInfo> mtages = {};

        // accumulated by "Instance" for instanced rendering
        vkt::Vector<glm::mat3x4> gpuTransformData = {};
        uint32_t transformStride = sizeof(glm::mat3x4);
        uint32_t lastBindID = 0u, locationCounter = 0u;
        bool needsUpdate = false;
        bool descriptorUpdated = false;

        // 
        vkh::VsGraphicsPipelineCreateInfoConstruction pipelineInfo = {};


        // FOR CREATE (Acceleration Structure)
        vkh::VkAccelerationStructureCreateInfoKHR                          bottomCreate = {}; // CREATE SINGLE!
        std::vector<vkh::VkAccelerationStructureCreateGeometryTypeInfoKHR> bottomDataCreate = { {} }; // CAN BE MULTIPLE!

        // FOR BUILD! BUT ONLY SINGLE! (Contain Multiple-Instanced)
        vkh::VkAccelerationStructureBuildGeometryInfoKHR                  bdHeadInfo = {};
        //std::vector<vkh::VkAccelerationStructureBuildGeometryInfoKHR>   bdHeadInfo = { {} };
        //vkt::uni_arg<vkh::VkAccelerationStructureBuildGeometryInfoKHR*> bdHeadPtr = {};

        // TODO: OFFSET DEPRECATED
        // CAN BE MULTIPLE! (single element of array, array of array[0])
        //vkt::Vector<uint64_t>                                         offsetIndirectPtr {};
        //vkt::Vector<vkh::VkAccelerationStructureBuildOffsetInfoKHR>   offsetIndirect {};
        std::vector<vkh::VkAccelerationStructureBuildOffsetInfoKHR>   offsetInfo = { {} };
        std::vector<vkh::VkAccelerationStructureBuildOffsetInfoKHR*>  offsetPtr = { {} };
        vkh::VkAccelerationStructureBuildOffsetInfoKHR                offsetTemp = {}; // INSTANCE TEMPLATE, CAN'T BE ARRAY! 

        // But used only one, due transform feedback shaders used... 
        // CAN BE MULTIPLE! (single element of array, array of array[0])
        std::vector<vkh::VkAccelerationStructureGeometryKHR>   buildGInfo = { {} };
        std::vector<vkh::VkAccelerationStructureGeometryKHR*>  buildGPtr = { {} };
        vkh::VkAccelerationStructureGeometryKHR                buildGTemp = {}; // INSTANCE TEMPLATE, CAN'T BE ARRAY! 

        // 
        VkPipeline rasterizationState = {}; // Vertex Input can changed, so use individual rasterization stages
        VkPipeline covergenceState = {};
        VkPipeline mapState = {};

        // 
        vkt::Vector<uint8_t> TempBuffer = {};
        vkt::Vector<uint8_t> gpuScratchBuffer = {};

        // 
        vkt::Vector<MeshInfo> rawMeshInfo = {};

        // 
        vkt::Vector<uint32_t> rawInstanceMap = {};
        vkt::Vector<uint32_t> gpuInstanceMap = {};

        // 
        VkAccelerationStructureKHR accelerationStructure = {};
        VmaAllocationInfo allocationInfo = {};
        VmaAllocation allocation = {};

        // 
        vkt::Vector<glm::uvec4> counterData = {};
        vkt::Vector<uint32_t> offsetCounterData = {};
        vkt::Vector<uint32_t> rawMaterialIDs = {};
        vkt::Vector<uint32_t> gpuMaterialIDs = {};

        // 
        std::vector<vkt::uni_ptr<MeshInput>> inputs = {};
        std::vector<VkDeviceSize> ranges = {};
        std::vector<VkDeviceSize> instances = {};

        // 
        vkt::uni_ptr<Driver> driver = {};
        vkt::uni_ptr<Thread> thread = {};
        vkt::uni_ptr<Context> context = {};
        //vkt::uni_ptr<MeshInput> input = {}; // Currently, Single! 
        //vkt::uni_ptr<Renderer> renderer = {};
    };


    // Implemented here due undefined type..
    uPTR(MeshInput) MeshInput::buildGeometry(const vkt::uni_ptr<jvi::MeshBinding>& binding, vkt::uni_arg<glm::u64vec4> offsetHelp, vkt::uni_arg<VkCommandBuffer> buildCommand) { // 
         bool DirectCommand = false, HasCommand = buildCommand.has() && *buildCommand;

         // Initialize Input
         this->createRasterizePipeline()->createDescriptorSet();

         // 
         if (!HasCommand || ignoreIndirection) {
             buildCommand = vkt::createCommandBuffer(this->driver->getDeviceDispatch(), this->thread->getCommandPool()); DirectCommand = true;
         };

         // 
         if (this->indexData) {
             this->meta.indexID = *this->indexData;
             this->meta.indexType = int32_t(this->indexType) + 1;
         };

         // 
         if (this->bvs) {
             this->descriptorSet.resize(2u);
             this->descriptorSet[1u] = this->bvs->getDescriptorSet();
         };

         // 
         if (HasCommand && this->needUpdate) {
             this->needUpdate = false; // 
             std::vector<VkBuffer> buffers = {}; std::vector<VkDeviceSize> offsets = {};
             buffers.resize(this->bindings.size()); offsets.resize(this->bindings.size()); uintptr_t I = 0u;
             for (auto& B : this->bindings) { if (this->bvs->get(B).has()) { const uintptr_t i = I++; buffers[i] = this->bvs->get(B).buffer(); offsets[i] = this->bvs->get(B).offset(); }; };

             // 
             const auto& viewport = reinterpret_cast<vkh::VkViewport&>(this->context->refViewport());
             const auto& renderArea = reinterpret_cast<vkh::VkRect2D&>(this->context->refScissor());
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
             this->meta.primitiveCount = uint32_t(this->currentUnitCount) / 3u;
             this->meta.indexType = int32_t(this->indexType) + 1;

             // 
             //vkt::debugLabel(buildCommand, "Begin building geometry data...", this->driver->getDispatch());
             const glm::uvec4 counterValue = glm::uvec4(0u);
             //this->driver->getDeviceDispatch()->CmdUpdateBuffer(buildCommand, binding->counterData.buffer(), binding->counterData.offset(), sizeof(glm::uvec4), &counterValue);
             vkt::commandBarrier(this->driver->getDeviceDispatch(), buildCommand);

             // 
             const auto gBuffer = binding->getBindingBuffer();
             const VkDeviceSize gOffset = 0u;//offsetHelp->y;
             const VkDeviceSize mOffset = gOffset + gBuffer.offset();
             const VkDeviceSize mSize = std::min(gBuffer.size() - gOffset, this->currentUnitCount * DEFAULT_STRIDE);
             const VkDeviceSize mGeometryCount = offsetHelp->z;

             // 
             this->meta.geometryCount = mGeometryCount;

             //
             auto stage = vkh::VkShaderStageFlags{.eVertex = 1, .eGeometry = 1, .eFragment = 1, .eCompute = 1, .eRaygen = 1, .eClosestHit = 1, .eMiss = 1};
             this->driver->getDeviceDispatch()->CmdBeginRenderPass(buildCommand, vkh::VkRenderPassBeginInfo{ .renderPass = this->context->refRenderPass(), .framebuffer = this->context->smpFlip0Framebuffer, .renderArea = renderArea, .clearValueCount = static_cast<uint32_t>(clearValues.size()), .pClearValues = clearValues.data() }, VK_SUBPASS_CONTENTS_INLINE);
             this->driver->getDeviceDispatch()->CmdSetViewport(buildCommand, 0u, 1u, viewport);
             this->driver->getDeviceDispatch()->CmdSetScissor(buildCommand, 0u, 1u, renderArea);
             this->driver->getDeviceDispatch()->CmdBeginTransformFeedbackEXT(buildCommand, 0u, 1u, &binding->counterData.buffer(), &binding->counterData.offset());
             this->driver->getDeviceDispatch()->CmdBindTransformFeedbackBuffersEXT(buildCommand, 0u, 1u, &gBuffer.buffer(), &mOffset, &mSize);
             this->driver->getDeviceDispatch()->CmdBindPipeline(buildCommand, VK_PIPELINE_BIND_POINT_GRAPHICS, this->transformState);
             this->driver->getDeviceDispatch()->CmdBindDescriptorSets(buildCommand, VK_PIPELINE_BIND_POINT_GRAPHICS, this->transformPipelineLayout, 0u, this->descriptorSet.size(), this->descriptorSet.data(), 0u, nullptr);
             this->driver->getDeviceDispatch()->CmdBindVertexBuffers(buildCommand, 0u, buffers.size(), buffers.data(), offsets.data());
             this->driver->getDeviceDispatch()->CmdPushConstants(buildCommand, this->transformPipelineLayout, stage, 0u, sizeof(meta), & meta);

             // 
             if (this->indexType != VK_INDEX_TYPE_NONE_KHR) {
                 const uintptr_t voffset = 0u;
                 this->driver->getDeviceDispatch()->CmdBindIndexBuffer(buildCommand, this->bvs->get(*this->indexData).buffer(), this->bvs->get(*this->indexData).offset() + this->indexOffset, this->indexType);
                 this->driver->getDeviceDispatch()->CmdDrawIndexed(buildCommand, this->currentUnitCount, 1u, 0u, voffset, 0u);
             } else {
                 this->driver->getDeviceDispatch()->CmdDraw(buildCommand, this->currentUnitCount, 1u, 0u, 0u);
             };

             // 
             this->driver->getDeviceDispatch()->CmdEndTransformFeedbackEXT(buildCommand, 0u, 1u, &binding->counterData.buffer(), &binding->counterData.offset());
             this->driver->getDeviceDispatch()->CmdEndRenderPass(buildCommand);

             //vkt::debugLabel(*buildCommand, "Ending building geometry data...", this->driver->getDispatch());
             vkt::commandBarrier(this->driver->getDeviceDispatch(), buildCommand); // dont transform feedback
             this->descriptorUpdated = false;
         };

         if (DirectCommand) {
             vkt::submitCmd(this->driver->getDeviceDispatch(), this->thread->getQueue(), { buildCommand });
             this->driver->getDeviceDispatch()->FreeCommandBuffers(this->thread->getCommandPool(), 1u, buildCommand);
             //this->thread->getDevice().freeCommandBuffers(this->thread->getCommandPool(), { buildCommand });
         };

         return uTHIS;
    };


};
