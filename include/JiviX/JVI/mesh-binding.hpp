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
    class MeshBinding : public std::enable_shared_from_this<MeshBinding> { public: friend Node; friend Renderer; friend MeshInput;
        MeshBinding() {};
        MeshBinding(const std::shared_ptr<Context>& context, vk::DeviceSize MaxPrimitiveCount = MAX_PRIM_COUNT, std::vector<vk::DeviceSize> GeometryInitial = {}) : context(context), MaxPrimitiveCount(MaxPrimitiveCount), GeometryInitial(GeometryInitial) { this->construct(); };
        MeshBinding(const std::shared_ptr<Context>& context, vk::DeviceSize MaxPrimitiveCount, std::vector<int64_t> GeometryInitial) : context(context), MaxPrimitiveCount(MaxPrimitiveCount), GeometryInitial(vkt::vector_cast<vk::DeviceSize>(GeometryInitial)) { this->construct(); };
        MeshBinding(const vkt::uni_ptr<Context>& context, vk::DeviceSize MaxPrimitiveCount = MAX_PRIM_COUNT, std::vector<vk::DeviceSize> GeometryInitial = {}) : context(context), MaxPrimitiveCount(MaxPrimitiveCount), GeometryInitial(GeometryInitial) { this->construct(); };
        MeshBinding(const vkt::uni_ptr<Context>& context, vk::DeviceSize MaxPrimitiveCount, std::vector<int64_t> GeometryInitial) : context(context), MaxPrimitiveCount(MaxPrimitiveCount), GeometryInitial(vkt::vector_cast<vk::DeviceSize>(GeometryInitial)) { this->construct(); };
        ~MeshBinding() {};

        // 
        virtual vkt::uni_ptr<MeshBinding> sharedPtr() { return shared_from_this(); };
        //virtual vkt::uni_ptr<MeshBinding> sharedPtr() const { return std::shared_ptr<MeshBinding>(shared_from_this()); };

        // 
        virtual uPTR(MeshBinding) construct() {
            this->driver = context->getDriver();
            this->thread = std::make_shared<Thread>(this->driver);

            // 
            if (this->GeometryInitial.size() <= 0ull) { this->GeometryInitial.push_back(MaxPrimitiveCount); };

            // 
            this->pipelineInfo = vkh::VsGraphicsPipelineCreateInfoConstruction();

            // create required buffers
            this->rawBindings = vkt::Vector<VkVertexInputBindingDescription>(std::make_shared<vkt::VmaBufferAllocation>(this->driver->getAllocator(), vkh::VkBufferCreateInfo{ .size = sizeof(VkVertexInputBindingDescription) * 8u, .usage = {.eTransferSrc = 1, .eTransferDst = 1, .eUniformBuffer = 1, .eStorageBuffer = 1, .eRayTracing = 1 } }, VMA_MEMORY_USAGE_CPU_TO_GPU));
            this->gpuBindings = vkt::Vector<VkVertexInputBindingDescription>(std::make_shared<vkt::VmaBufferAllocation>(this->driver->getAllocator(), vkh::VkBufferCreateInfo{ .size = sizeof(VkVertexInputBindingDescription) * 8u, .usage = {.eTransferSrc = 1, .eTransferDst = 1, .eUniformBuffer = 1, .eStorageBuffer = 1, .eRayTracing = 1 } }, VMA_MEMORY_USAGE_GPU_ONLY));
            this->rawAttributes = vkt::Vector<VkVertexInputAttributeDescription>(std::make_shared<vkt::VmaBufferAllocation>(this->driver->getAllocator(), vkh::VkBufferCreateInfo{ .size = sizeof(VkVertexInputAttributeDescription) * 8u, .usage = {.eTransferSrc = 1, .eUniformBuffer = 1, .eStorageBuffer = 1, .eRayTracing = 1 } }, VMA_MEMORY_USAGE_CPU_TO_GPU));
            this->gpuAttributes = vkt::Vector<VkVertexInputAttributeDescription>(std::make_shared<vkt::VmaBufferAllocation>(this->driver->getAllocator(), vkh::VkBufferCreateInfo{ .size = sizeof(VkVertexInputAttributeDescription) * 8u, .usage = {.eTransferDst = 1, .eUniformBuffer = 1, .eStorageBuffer = 1, .eRayTracing = 1 } }, VMA_MEMORY_USAGE_GPU_ONLY));
            this->rawMaterialIDs = vkt::Vector<uint32_t>(std::make_shared<vkt::VmaBufferAllocation>(this->driver->getAllocator(), vkh::VkBufferCreateInfo{ .size = sizeof(uint32_t) * GeometryInitial.size(), .usage = {.eTransferSrc = 1, .eUniformBuffer = 1, .eStorageBuffer = 1, .eRayTracing = 1 } }, VMA_MEMORY_USAGE_CPU_TO_GPU));
            this->gpuMaterialIDs = vkt::Vector<uint32_t>(std::make_shared<vkt::VmaBufferAllocation>(this->driver->getAllocator(), vkh::VkBufferCreateInfo{ .size = sizeof(uint32_t) * GeometryInitial.size(), .usage = {.eTransferDst = 1, .eUniformBuffer = 1, .eStorageBuffer = 1, .eRayTracing = 1 } }, VMA_MEMORY_USAGE_GPU_ONLY));

            // 
            this->counterData = vkt::Vector<uint32_t>(std::make_shared<vkt::VmaBufferAllocation>(this->driver->getAllocator(), vkh::VkBufferCreateInfo{ .size = sizeof(uint32_t) * 4u, .usage = {.eTransferSrc = 1, .eTransferDst = 1, .eUniformBuffer = 1, .eStorageBuffer = 1, .eIndirectBuffer = 1, .eRayTracing = 1, .eTransformFeedbackBuffer = 1, .eTransformFeedbackCounterBuffer = 1, .eSharedDeviceAddress = 1 } }, VMA_MEMORY_USAGE_GPU_TO_CPU));

            // ALPHA_TEST
            //this->offsetIndirectPtr = vkt::Vector<uint64_t>(std::make_shared<vkt::VmaBufferAllocation>(this->driver->getAllocator(), vkh::VkBufferCreateInfo{ .size = 16u, .usage = { .eTransferDst = 1, .eUniformBuffer = 1, .eStorageBuffer = 1, .eIndirectBuffer = 1, .eRayTracing = 1, .eTransformFeedbackCounterBuffer = 1 } }, VMA_MEMORY_USAGE_GPU_ONLY));
            //this->offsetIndirect = vkt::Vector<vkh::VkAccelerationStructureBuildOffsetInfoKHR>(std::make_shared<vkt::VmaBufferAllocation>(this->driver->getAllocator(), vkh::VkBufferCreateInfo{ .size = sizeof(vkh::VkAccelerationStructureBuildOffsetInfoKHR), .usage = {.eTransferDst = 1, .eUniformBuffer = 1, .eStorageBuffer = 1, .eIndirectBuffer = 1, .eRayTracing = 1, .eTransformFeedbackCounterBuffer = 1 } }, VMA_MEMORY_USAGE_GPU_ONLY));

            /*
            thread->submitOnce([&, this](vk::CommandBuffer& cmd) {
                cmd.fillBuffer(this->offsetIndirect, 0u, 4u, 1u);
                cmd.fillBuffer(this->offsetIndirect, 4u, 12u, 0u);
            });*/

            // 
            //this->gpuMeshInfo = vkt::Vector<MeshInfo>(this->driver->getAllocator(), vkh::VkBufferCreateInfo{ .size = 16u, .usage = {.eTransferDst = 1, .eUniformBuffer = 1, .eStorageBuffer = 1, .eRayTracing = 1 } }, VMA_MEMORY_USAGE_GPU_ONLY);
            this->rawMeshInfo = vkt::Vector<MeshInfo>(std::make_shared<vkt::VmaBufferAllocation>(this->driver->getAllocator(), vkh::VkBufferCreateInfo{ .size = 16u, .usage = {.eTransferSrc = 1, .eUniformBuffer = 1, .eStorageBuffer = 1, .eRayTracing = 1 } }, VMA_MEMORY_USAGE_CPU_TO_GPU));

            // Internal Instance Map Per Global Node
            this->rawInstanceMap = vkt::Vector<uint32_t>(std::make_shared<vkt::VmaBufferAllocation>(this->driver->getAllocator(), vkh::VkBufferCreateInfo{ .size = GeometryInitial.size() * sizeof(uint32_t), .usage = {.eTransferSrc = 1, .eUniformBuffer = 1, .eStorageBuffer = 1, .eRayTracing = 1 } }, VMA_MEMORY_USAGE_CPU_TO_GPU));
            this->gpuInstanceMap = vkt::Vector<uint32_t>(std::make_shared<vkt::VmaBufferAllocation>(this->driver->getAllocator(), vkh::VkBufferCreateInfo{ .size = GeometryInitial.size() * sizeof(uint32_t), .usage = {.eTransferDst = 1, .eUniformBuffer = 1, .eStorageBuffer = 1, .eRayTracing = 1 } }, VMA_MEMORY_USAGE_GPU_ONLY));

            // for faster code, pre-initialize
            this->stages = vkt::vector_cast<vkh::VkPipelineShaderStageCreateInfo, vk::PipelineShaderStageCreateInfo>({
                vkt::makePipelineStageInfo(this->driver->getDevice(), vkt::readBinary("./shaders/rtrace/rasterize.vert.spv"), vk::ShaderStageFlagBits::eVertex),
                vkt::makePipelineStageInfo(this->driver->getDevice(), vkt::readBinary("./shaders/rtrace/rasterize.frag.spv"), vk::ShaderStageFlagBits::eFragment)
            });

            // 
            this->ctages = vkt::vector_cast<vkh::VkPipelineShaderStageCreateInfo, vk::PipelineShaderStageCreateInfo>({
                vkt::makePipelineStageInfo(this->driver->getDevice(), vkt::readBinary("./shaders/rtrace/covergence.vert.spv"), vk::ShaderStageFlagBits::eVertex),
                vkt::makePipelineStageInfo(this->driver->getDevice(), vkt::readBinary("./shaders/rtrace/covergence.frag.spv"), vk::ShaderStageFlagBits::eFragment)
            });

            /*{ //
                this->indexData = vkt::Vector<uint8_t>(std::make_shared<vkt::VmaBufferAllocation>(this->driver->getAllocator(), vkh::VkBufferCreateInfo{
                    .size = sizeof(uint32_t) * 3u,
                    .usage = {.eTransferDst = 1, .eStorageTexelBuffer = 1, .eStorageBuffer = 1, .eIndexBuffer = 1, .eSharedDeviceAddress = 1 },
                }));
                this->rawMeshInfo[0u].indexType = uint32_t(vk::IndexType::eNoneKHR) + 1u;

                // TODO: other platforms memory handling
                // create OpenGL version of buffers
#ifdef ENABLE_OPENGL_INTEROP
                glCreateBuffers(1u, &this->indexDataOGL.second);
                glCreateMemoryObjectsEXT(1u, &this->indexDataOGL.first);
                glImportMemoryWin32HandleEXT(this->indexDataOGL.first, this->indexData->getAllocationInfo().reqSize, GL_HANDLE_TYPE_OPAQUE_WIN32_EXT, this->indexData->getAllocationInfo().handle);
                glNamedBufferStorageMemEXT(this->indexDataOGL.second, MaxPrimitiveCount * sizeof(uint32_t) * 3u, this->indexDataOGL.first, 0u);
#endif
            };*/

            // 
            vkt::MemoryAllocationInfo almac = {};
            almac.device = this->driver->getDevice();
            almac.dispatch = this->driver->getDispatch();
            almac.memoryProperties = this->driver->getMemoryProperties().memoryProperties;
            almac.memUsage = VMA_MEMORY_USAGE_GPU_ONLY;
            almac.glMemory = almac.glID = 0u;

            // 
            for (uint32_t i = 0; i < this->bindings.size(); i++) {
                this->bindings[i] = vkt::Vector<uint8_t>(std::make_shared<vkt::BufferAllocation>(almac, vkh::VkBufferCreateInfo{
                    .size = (i == 0 ? MaxPrimitiveCount : 1u) * (i == 0 ? MaxStride : sizeof(glm::vec4)) * 3u,
                    .usage = {.eTransferDst = 1, .eStorageTexelBuffer = 1, .eStorageBuffer = 1, .eVertexBuffer = 1, .eTransformFeedbackBuffer = 1, .eSharedDeviceAddress = 1 },
                }));

                // TODO: other platforms memory handling
                // create OpenGL version of buffers
#ifdef ENABLE_OPENGL_INTEROP
                gl::glCreateBuffers(1u, &this->bindingsOGL[i].second);
                gl::glCreateMemoryObjectsEXT(1u, &this->bindingsOGL[i].first);
                gl::glImportMemoryWin32HandleEXT(this->bindingsOGL[i].first, this->bindings[i]->getAllocationInfo().reqSize, gl::GL_HANDLE_TYPE_OPAQUE_WIN32_EXT, this->bindings[i]->getAllocationInfo().handle);
                gl::glNamedBufferStorageMemEXT(this->bindingsOGL[i].second, MaxPrimitiveCount * 3u * 80u, this->bindingsOGL[i].first, 0u);
#endif
            };

            // FOR BUILD! 
            this->bdHeadInfo = vkh::VkAccelerationStructureBuildGeometryInfoKHR{};
            this->bdHeadInfo.geometryCount = this->buildGInfo.size();
            this->bdHeadInfo.ppGeometries = nullptr;//reinterpret_cast<vkh::VkAccelerationStructureGeometryKHR**>((this->buildGPtr = this->buildGInfo.data()).ptr());
            this->bdHeadInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;
            this->bdHeadInfo.flags = VK_BUILD_ACCELERATION_STRUCTURE_ALLOW_UPDATE_BIT_KHR | VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR;
            this->bdHeadInfo.geometryArrayOfPointers = true; // MARK TRUE FOR INDIRECT BUILDING!!

            // FOR BUILD! FULL GEOMETRY INFO! // originally, it should to be array (like as old version of LancER)
            this->buildGTemp = vkh::VkAccelerationStructureGeometryKHR{ .flags = {.eOpaque = 1 } };
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
            this->setBinding(vkh::VkVertexInputBindingDescription{ .binding = 1, .stride = sizeof(glm::vec4) });
            //this->setAttribute(vkh::VkVertexInputAttributeDescription{ .location = 0u, .binding = 1, .format = VK_FORMAT_R32G32B32A32_SFLOAT, .offset = 0u });  // Positions

            // 
            const glm::uvec4 initialCount = glm::uvec4(0u);
            this->thread->submitOnce([&](const vk::CommandBuffer& cmdbuf) {
                cmdbuf.updateBuffer(counterData.buffer(), counterData.offset(), sizeof(glm::uvec4), &initialCount);
            });

            // 
            return uTHIS;
        };

        // 
        virtual uPTR(MeshBinding) setThread(const vkt::uni_ptr<Thread>& thread) {
            this->thread = thread;
            return uTHIS;
        };

        // 
        virtual uPTR(MeshBinding) setDriver(const vkt::uni_ptr<Driver>& driver = {}) {
            this->driver = driver;
            return uTHIS;
        };

        //
        virtual uPTR(MeshBinding) setThread(const std::shared_ptr<Thread>& thread) { return this->setThread(vkt::uni_ptr<Thread>(thread)); };
        virtual uPTR(MeshBinding) setDriver(const std::shared_ptr<Driver>& driver) { return this->setDriver(vkt::uni_ptr<Driver>(driver)); };

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
        virtual gl::GLuint& getBindingBufferGL(const uintptr_t& i = 0u) { return this->bindingsOGL[i].second; };
        virtual const gl::GLuint& getBindingBufferGL(const uintptr_t& i = 0u) const { return this->bindingsOGL[i].second; };

        // 
        virtual gl::GLuint& getIndexBufferGL() { return this->indexDataOGL.second; };
        virtual const gl::GLuint& getIndexBufferGL() const { return this->indexDataOGL.second; };
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

        // OBSOLETE! Needs Instance Based Material ID!
        //virtual uPTR(MeshBinding) setFirstMaterial(const uint32_t& materialID = 0u) {
        //    this->rawMeshInfo[0u].materialID = materialID;
        //    return uTHIS;
        //};

        // 
        virtual uPTR(MeshBinding) manifestIndex(const vk::IndexType& type = vk::IndexType::eNoneKHR) {
            this->rawMeshInfo[0u].indexType = uint32_t(this->buildGTemp.geometry.triangles.indexType = VkIndexType(type)) + 1u;
            return uTHIS;
        };

        // TODO: Add QUADs support for GEN-2.0
        virtual uPTR(MeshBinding) setIndexCount(const vk::DeviceSize& count = 65536u * 3u) { this->setPrimitiveCount(std::min(vk::DeviceSize(vkt::tiled(count, 3ull)), vk::DeviceSize(this->MaxPrimitiveCount))); return uTHIS; };
        virtual uPTR(MeshBinding) setPrimitiveCount(const vk::DeviceSize& count = 65536u) { this->primitiveCount = std::min(vk::DeviceSize(count), vk::DeviceSize(this->MaxPrimitiveCount)); return uTHIS; };

        // 
        virtual uPTR(MeshBinding) linkWithInstance(const uint32_t& mapID = 0u) {
            this->rawInstanceMap[this->mapCount++] = mapID;
            return uTHIS;
        };

        // DEPRECATED
        virtual uPTR(MeshBinding) increaseGeometryCount(const uint32_t& geometryCount = 1u) {
            //this->geometryCount += geometryCount;
            return uTHIS;
        };

        // 
        virtual uPTR(MeshBinding) setGeometryCount(const uint32_t& geometryCount = 1u) {
            //this->geometryCount = geometryCount;
            return uTHIS;
        };

        // 
        virtual uPTR(MeshBinding) setTransformData(const vkt::uni_arg<vkt::Vector<glm::mat3x4>>& transformData = {}, const uint32_t& stride = sizeof(glm::mat3x4)) {
            //this->offsetTemp.transformOffset = transformData.offset(); //!!
            this->buildGTemp.geometry.triangles.transformData = transformData->deviceAddress();
            this->transformStride = stride; // used for instanced correction
            this->rawMeshInfo[0u].hasTransform = 1u;
            return uTHIS;
        };

        // 
        //template<class T = uint8_t>
        inline uPTR(MeshBinding) setBinding(vkt::uni_arg<vkh::VkVertexInputBindingDescription> binding = vkh::VkVertexInputBindingDescription{}) {
            const uintptr_t bindingID = binding->binding;
            this->vertexInputBindingDescriptions.resize(bindingID + 1u);
            this->vertexInputBindingDescriptions[bindingID] = binding;
            this->vertexInputBindingDescriptions[bindingID].binding = bindingID;
            this->rawBindings[bindingID] = this->vertexInputBindingDescriptions[bindingID];
            return uTHIS;
        };

        // DEPRECATED!
        virtual uPTR(MeshBinding) setAttribute(vkt::uni_arg<vkh::VkVertexInputAttributeDescription> attribute = vkh::VkVertexInputAttributeDescription{}, const bool& NotStub = true) {
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
                //this->offsetTemp.primitiveOffset = bindingBuffer.offset() + attribute->offset; // !!WARNING!!
                this->buildGTemp.geometryType = VK_GEOMETRY_TYPE_TRIANGLES_KHR;
                this->buildGTemp.geometry.triangles.vertexFormat = attribute->format;
                this->buildGTemp.geometry.triangles.vertexStride = bindingData.stride;
                this->buildGTemp.geometry.triangles.vertexData = bindingBuffer;
                this->buildGTemp.flags = { .eOpaque = 1 };

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
        virtual uPTR(MeshBinding) copyBuffers(const vkt::uni_arg<VkCommandBuffer>& buildCommand = {}){
            return this->copyBuffers(vk::CommandBuffer(*buildCommand));
        };

        // 
        virtual uPTR(MeshBinding) copyBuffers(const vkt::uni_arg<vk::CommandBuffer>& buildCommand = {}) {
            buildCommand->copyBuffer(this->rawAttributes, this->gpuAttributes, { vk::BufferCopy{ this->rawAttributes.offset(), this->gpuAttributes.offset(), this->gpuAttributes.range() } });
            buildCommand->copyBuffer(this->rawBindings, this->gpuBindings, { vk::BufferCopy{ this->rawBindings.offset(), this->gpuBindings.offset(), this->gpuBindings.range() } });
            buildCommand->copyBuffer(this->rawInstanceMap, this->gpuInstanceMap, { vk::BufferCopy{ this->rawInstanceMap.offset(), this->gpuInstanceMap.offset(), this->gpuInstanceMap.range() } });
            buildCommand->copyBuffer(this->rawMaterialIDs, this->gpuMaterialIDs, { vk::BufferCopy{ this->rawMaterialIDs.offset(), this->gpuMaterialIDs.offset(), this->gpuMaterialIDs.range() } });
            buildCommand->updateBuffer<glm::uvec4>(counterData.buffer(), counterData.offset(), { glm::uvec4(0u) }); // Nullify Counters
            if (this->inputs.size() > 0) { for (auto& I : this->inputs) if (I.has()) { I->copyMeta(buildCommand); }; };
            return uTHIS;
        };

        //
        virtual uPTR(MeshBinding) buildGeometry(const vkt::uni_arg<VkCommandBuffer>& buildCommand = {}, const vkt::uni_arg<glm::uvec4>& meshData = glm::uvec4(0u)){
            return this->buildGeometry(vk::CommandBuffer(*buildCommand), meshData);
        };

        // 
        virtual uPTR(MeshBinding) buildGeometry(const vkt::uni_arg<vk::CommandBuffer>& buildCommand = {}, const vkt::uni_arg<glm::uvec4>& meshData = glm::uvec4(0u)) { // build geometry data
            if (this->fullGeometryCount <= 0u || this->mapCount <= 0u) return uTHIS; this->primitiveCount = 0u;

            // 
            if (this->fullGeometryCount > this->offsetInfo.size()) {
                this->offsetInfo.resize(this->fullGeometryCount);
                this->offsetPtr.resize(this->fullGeometryCount);
            };

            // this->fullGeometryCount
            uint32_t f = 0, i = 0, c = 0; for (auto& I : this->inputs) { // Quads needs to format...
                const auto uOffset = this->primitiveCount * 3u;
                if (I.has()) { // WARNING! Out Of Pool Memory!
                    I->createRasterizePipeline()->createDescriptorSet()->formatQuads(uTHIS, glm::u64vec4(uOffset, 0u, 0u, 0u), buildCommand);
                };

                // copy as template, use as triangle...
                auto offsetp = this->offsetTemp;
                {
                    //offsetp.primitiveOffset = uOffset * 80u; //+ this->bindings[0u].offset();
                    offsetp.primitiveCount = I.has() ? vkt::tiled(I->getIndexCount(), 3ull) : vkt::tiled(this->ranges[i], 1ull); // TODO: De-Facto primitive count...
                    offsetp.firstVertex = uOffset;
                    if (I.has()) { I->buildGeometry(uTHIS, glm::u64vec4(uOffset, uOffset * 80u, 0u, 0u), buildCommand); };
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
            return uTHIS;
        };

        // TODO: Fix Quads support with Indices
        virtual uPTR(MeshBinding) buildAccelerationStructure(const VkCommandBuffer& buildCommand = {}, const vkt::uni_arg<glm::uvec4>& meshData = glm::uvec4(0u)){
            return this->buildAccelerationStructure(vk::CommandBuffer(buildCommand), meshData);
        };

        //
        virtual uPTR(MeshBinding) buildAccelerationStructure(const vk::CommandBuffer& buildCommand = {}, const vkt::uni_arg<glm::uvec4>& meshData = glm::uvec4(0u)) {
            if (this->fullGeometryCount <= 0u || this->mapCount <= 0u) return uTHIS;
            if (!this->accelerationStructure) { this->createAccelerationStructure(); };

            //std::vector<vkh::VkAccelerationStructureGeometryKHR> ptrs = {};
            //ptrs.push_back(reinterpret_cast<vkh::VkAccelerationStructureGeometryKHR&>(this->buildGInfo[i] = this->buildGTemp));

            // 
            if (this->fullGeometryCount > this->buildGInfo.size()) {
                this->buildGInfo.resize(this->fullGeometryCount);
                this->buildGPtr.resize(this->fullGeometryCount);
            };

            // 
            uint32_t f = 0, i = 0, c = 0;  for (auto& I : this->inputs) { // Quads needs to format...
                for (uint32_t j = 0; j < this->instances[i]; j++) {
                    this->buildGInfo[c] = this->buildGTemp;
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
                vkt::debugLabel(buildCommand, "Begin building bottom acceleration structure...", this->driver->getDispatch());
                buildCommand.buildAccelerationStructureKHR(1u, this->bdHeadInfo, reinterpret_cast<vk::AccelerationStructureBuildOffsetInfoKHR**>(this->offsetPtr.data()), this->driver->getDispatch()); this->needsUpdate = true;
                vkt::debugLabel(buildCommand, "Ending building bottom acceleration structure...", this->driver->getDispatch()); this->needsUpdate = true;
            } else {
                driver->getDevice().buildAccelerationStructureKHR(1u, this->bdHeadInfo, reinterpret_cast<vk::AccelerationStructureBuildOffsetInfoKHR**>(this->offsetPtr.data()), this->driver->getDispatch());
            };

            //
            return uTHIS;
        };


        // 
        virtual uPTR(MeshBinding) addRangeInput(const vk::DeviceSize& primitiveCount, const uint32_t& materialID = 0u, const vk::DeviceSize& instanceCount = 1u) {
            uintptr_t ID = this->inputs.size();
            this->inputs.push_back({}); // Correct! 
            this->ranges.push_back(primitiveCount);
            this->instances.push_back(instanceCount);
            for (uint32_t i = 0; i < instanceCount; i++) { this->rawMaterialIDs[this->fullGeometryCount + i] = materialID; }; // TODO: Material ID per instance
            this->fullGeometryCount += instanceCount;
            return uTHIS;
        };

        // Instanced, but with vector of materials
        virtual uPTR(MeshBinding) addRangeInput(const vk::DeviceSize& primitiveCount, const std::vector<uint32_t>& materialIDs) {
            uintptr_t ID = this->inputs.size();
            this->inputs.push_back({}); // Correct! 
            this->ranges.push_back(primitiveCount);
            this->instances.push_back(materialIDs.size());
            for (uint32_t i = 0; i < materialIDs.size(); i++) { this->rawMaterialIDs[this->fullGeometryCount + i] = materialIDs[i]; }; // TODO: Material ID per instance
            this->fullGeometryCount += materialIDs.size();
            return uTHIS;
        };

        // 
        virtual uPTR(MeshBinding) addRangeInput(const vk::DeviceSize& primitiveCount, const std::vector<int32_t>& materialIDs) {
            return this->addRangeInput(primitiveCount, vkt::vector_cast<uint32_t>(materialIDs));
        };


        // 
        virtual uPTR(MeshBinding) addMeshInput(const vkt::uni_ptr<MeshInput>& input, const uint32_t& materialID = 0u, const vk::DeviceSize& instanceCount = 1u) {
            uintptr_t ID = this->inputs.size();
            this->inputs.push_back(input); // Correct! 
            this->ranges.push_back(vkt::tiled(input->getIndexCount(), 3ull));
            this->instances.push_back(instanceCount);
            for (uint32_t i = 0; i < instanceCount; i++) { this->rawMaterialIDs[this->fullGeometryCount + i] = materialID; }; // TODO: Material ID per instance
            this->fullGeometryCount += instanceCount;
            return uTHIS;
        };

        // Instanced, but with vector of materials
        virtual uPTR(MeshBinding) addMeshInput(const vkt::uni_ptr<MeshInput>& input, const std::vector<uint32_t>& materialIDs) {
            uintptr_t ID = this->inputs.size();
            this->inputs.push_back(input); // Correct! 
            this->ranges.push_back(vkt::tiled(input->getIndexCount(), 3ull));
            this->instances.push_back(materialIDs.size());
            for (uint32_t i = 0; i < materialIDs.size(); i++) { this->rawMaterialIDs[this->fullGeometryCount + i] = materialIDs[i]; }; // TODO: Material ID per instance
            this->fullGeometryCount += materialIDs.size();
            return uTHIS;
        };


        // 
        virtual uPTR(MeshBinding) addMeshInput(const vkt::uni_ptr<MeshInput>& input, const std::vector<int32_t>& materialIDs) {
            return this->addMeshInput(input, vkt::vector_cast<uint32_t>(materialIDs));
        };

        // 
        virtual uPTR(MeshBinding) addMeshInput(const std::shared_ptr<MeshInput>& input, const uint32_t& materialID = 0u, const vk::DeviceSize& instanceCount = 1u) {
            return this->addMeshInput(vkt::uni_ptr<MeshInput>(input), materialID, instanceCount);
        };

        // 
        virtual uPTR(MeshBinding) addMeshInput(const std::shared_ptr<MeshInput>& input, const std::vector<uint32_t>& materialIDs) {
            return this->addMeshInput(vkt::uni_ptr<MeshInput>(input), materialIDs);
        };

        // 
        virtual uPTR(MeshBinding) addMeshInput(const std::shared_ptr<MeshInput>& input, const std::vector<int32_t>& materialIDs) {
            return this->addMeshInput(vkt::uni_ptr<MeshInput>(input), materialIDs);
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
                TempBuffer = vkt::Vector<uint8_t>(std::make_shared<vkt::VmaBufferAllocation>(this->driver->getAllocator(), vkh::VkBufferCreateInfo{
                    .size = requirements.memoryRequirements.size,
                    .usage = {.eTransferDst = 1, .eStorageTexelBuffer = 1, .eStorageBuffer = 1, .eVertexBuffer = 1, .eSharedDeviceAddress = 1 },
                }));

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
                this->gpuScratchBuffer = vkt::Vector<uint8_t>(std::make_shared<vkt::VmaBufferAllocation>(this->driver->getAllocator(), vkh::VkBufferCreateInfo{
                    .size = requirements.memoryRequirements.size,
                    .usage = { .eStorageBuffer = 1, .eRayTracing = 1, .eSharedDeviceAddress = 1 }
                }, VMA_MEMORY_USAGE_GPU_ONLY));
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
            this->rasterizationState = vkt::handleHpp(driver->getDevice().createGraphicsPipeline(driver->getPipelineCache(), this->pipelineInfo));

            // 
            this->pipelineInfo.rasterizationState.pNext = &conserv;
            this->pipelineInfo.stages = this->ctages;
            this->covergenceState = vkt::handleHpp(driver->getDevice().createGraphicsPipeline(driver->getPipelineCache(), this->pipelineInfo));

            // 
            return uTHIS;
        };

        // Create Secondary Command With Pipeline
        virtual uPTR(MeshBinding) createRasterizeCommand(const vk::CommandBuffer& rasterCommand = {}, const glm::uvec4& meshData = glm::uvec4(0u), const bool& conservative = false) { // UNIT ONLY!
            if (this->fullGeometryCount <= 0u || this->mapCount <= 0u) return uTHIS;

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
            rasterCommand.endRenderPass();
            vkt::debugLabel(rasterCommand, "End rasterization...", this->driver->getDispatch());

            // 
            return uTHIS;
        };

    // 
    protected: friend MeshBinding; friend Node; friend Renderer; // GPU Vertex and Attribute Data
        vkt::Vector<uint8_t> indexData = {}; 
#ifdef ENABLE_OPENGL_INTEROP
        std::pair<gl::GLuint, gl::GLuint> indexDataOGL = {};
#endif

        // 
        //vk::IndexType indexType = vk::IndexType::eNoneKHR;
        vk::DeviceSize MaxPrimitiveCount = MAX_PRIM_COUNT, MaxStride = DEFAULT_STRIDE;
        std::vector<vk::DeviceSize> GeometryInitial = { MAX_PRIM_COUNT };

        // `primitiveCount` should to be counter!
        uint32_t primitiveCount = 0u, mapCount = 0u, fullGeometryCount = 0u;

        // 
        std::array<vkt::Vector<uint8_t>, 2> bindings = {};
#ifdef ENABLE_OPENGL_INTEROP
        std::array<std::pair<gl::GLuint, gl::GLuint>, 2> bindingsOGL = {};
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
        vk::Pipeline rasterizationState = {}; // Vertex Input can changed, so use individual rasterization stages
        vk::Pipeline covergenceState = {};

        // 
        vkt::Vector<uint8_t> TempBuffer = {};
        vkt::Vector<uint8_t> gpuScratchBuffer = {};

        // 
        vkt::Vector<MeshInfo> rawMeshInfo = {};

        // 
        vkt::Vector<uint32_t> rawInstanceMap = {};
        vkt::Vector<uint32_t> gpuInstanceMap = {};

        // 
        vk::AccelerationStructureKHR accelerationStructure = {};
        VmaAllocationInfo allocationInfo = {};
        VmaAllocation allocation = {};

        // 
        vkt::Vector<glm::uvec4> counterData = {};
        vkt::Vector<uint32_t> rawMaterialIDs = {};
        vkt::Vector<uint32_t> gpuMaterialIDs = {};

        // 
        std::vector<vkt::uni_ptr<MeshInput>> inputs = {};
        std::vector<vk::DeviceSize> ranges = {};
        std::vector<vk::DeviceSize> instances = {};

        // 
        vkt::uni_ptr<Driver> driver = {};
        vkt::uni_ptr<Thread> thread = {};
        vkt::uni_ptr<Context> context = {};
        //vkt::uni_ptr<MeshInput> input = {}; // Currently, Single! 
        //vkt::uni_ptr<Renderer> renderer = {};
    };


    // Implemented here due undefined type..
    uPTR(MeshInput) MeshInput::buildGeometry(const vkt::uni_ptr<jvi::MeshBinding>& binding, vkt::uni_arg<glm::u64vec4> offsetHelp, vkt::uni_arg<vk::CommandBuffer> buildCommand) { // 
         bool DirectCommand = false, HasCommand = buildCommand.has() && buildCommand && *buildCommand;

         // 
         if (!HasCommand || ignoreIndirection) {
             buildCommand = vkt::createCommandBuffer(this->thread->getDevice(), this->thread->getCommandPool()); DirectCommand = true;
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
             std::vector<vk::Buffer> buffers = {}; std::vector<vk::DeviceSize> offsets = {};
             buffers.resize(this->bindings.size()); offsets.resize(this->bindings.size()); uintptr_t I = 0u;
             for (auto& B : this->bindings) { if (this->bvs->get(B).has()) { const uintptr_t i = I++; buffers[i] = this->bvs->get(B).buffer(); offsets[i] = this->bvs->get(B).offset(); }; };

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

             // TODO: Fix Vertex Count for Quads
             this->meta.primitiveCount = uint32_t(this->currentUnitCount) / 3u;
             this->meta.indexType = int32_t(this->indexType) + 1;

             // 
             vkt::debugLabel(buildCommand, "Begin building geometry data...", this->driver->getDispatch());
             buildCommand->updateBuffer<glm::uvec4>(binding->counterData.buffer(), binding->counterData.offset(), { glm::uvec4(0u) }); // Nullify Counters
             vkt::commandBarrier(buildCommand);

             // 
             const auto& gOffset = offsetHelp->y;
             const auto& gBuffer = binding->getBindingBuffer();
             buildCommand->beginRenderPass(vk::RenderPassBeginInfo(this->context->refRenderPass(), this->context->deferredFramebuffer, renderArea, static_cast<uint32_t>(clearValues.size()), clearValues.data()), vk::SubpassContents::eInline);
             buildCommand->setViewport(0, { viewport });
             buildCommand->setScissor(0, { renderArea });
             buildCommand->beginTransformFeedbackEXT(0u, { binding->counterData.buffer() }, { binding->counterData.offset() }, this->driver->getDispatch()); //!!WARNING!!
             buildCommand->bindTransformFeedbackBuffersEXT(0u, { gBuffer.buffer() }, { gBuffer.offset() + gOffset }, { gBuffer.range() - gOffset }, this->driver->getDispatch()); //!!WARNING!!
             buildCommand->bindDescriptorSets(vk::PipelineBindPoint::eGraphics, this->transformPipelineLayout, 0ull, this->descriptorSet, {});
             buildCommand->bindPipeline(vk::PipelineBindPoint::eGraphics, this->transformState);
             buildCommand->bindVertexBuffers(0u, buffers, offsets);
             buildCommand->pushConstants<jvi::MeshInfo>(this->transformPipelineLayout, vkh::VkShaderStageFlags{.eVertex = 1, .eGeometry = 1, .eFragment = 1, .eCompute = 1, .eRaygen = 1, .eClosestHit = 1, .eMiss = 1 }.hpp(), 0u, { meta });

             // 
             if (this->indexType != vk::IndexType::eNoneKHR) { // PLC Mode
                 const uintptr_t voffset = 0u;//this->bindings[this->vertexInputAttributeDescriptions[0u].binding].offset(); // !!WARNING!!
                 buildCommand->bindIndexBuffer(this->bvs->get(*this->indexData).buffer(), this->bvs->get(*this->indexData).offset() + this->indexOffset, this->indexType);
                 buildCommand->drawIndexed(this->currentUnitCount, 1u, 0u, voffset, 0u);
             }
             else { // VAL Mode
                 buildCommand->draw(this->currentUnitCount, 1u, 0u, 0u);
             };

             // 
             buildCommand->endTransformFeedbackEXT(0u, { binding->counterData.buffer() }, { binding->counterData.offset() }, this->driver->getDispatch()); //!!WARNING!!
             buildCommand->endRenderPass();
             vkt::debugLabel(*buildCommand, "Ending building geometry data...", this->driver->getDispatch());
             vkt::commandBarrier(buildCommand); // dont transform feedback
         };

         if (DirectCommand) {
             vkt::submitCmd(this->thread->getDevice(), this->thread->getQueue(), { buildCommand });
             this->thread->getDevice().freeCommandBuffers(this->thread->getCommandPool(), { buildCommand });
         };

         return uTHIS;
    };


};
