#pragma once // #

#include "./config.hpp"
#include "./driver.hpp"
#include "./thread.hpp"
#include "./context.hpp"

namespace jvi {

    struct MeshInfo {
        uint32_t materialID = 0u;
        uint32_t indexType = 0u;
        uint32_t prmCount = 0u;
        //uint32_t flags = 0u;
        uint32_t
            hasTransform : 1,
            hasNormal : 1,
            hasTexcoord : 1,
            hasTangent : 1;
    };

    // WIP Mesh Object
    // Sub-Instances Can Be Supported
    // TODO: Descriptor Sets
    class Mesh : public std::enable_shared_from_this<Mesh> { public: friend Node; friend Renderer;
        Mesh() {};
        Mesh(const vkt::uni_ptr<Context>& context, vk::DeviceSize AllocationUnitCount = 32768) : context(context), AllocationUnitCount(AllocationUnitCount) { this->construct(); };
        //Mesh(Context* context, vk::DeviceSize AllocationUnitCount = 32768) : AllocationUnitCount(AllocationUnitCount) { this->context = vkt::uni_ptr<Context>(context); this->construct(); };
        ~Mesh() {};

        // 
        virtual vkt::uni_ptr<Mesh> sharedPtr() { return shared_from_this(); };
        virtual vkt::uni_ptr<const Mesh> sharedPtr() const { return shared_from_this(); };

        // 
        virtual uPTR(Mesh) construct() {
            this->driver = context->getDriver();
            this->thread = std::make_shared<Thread>(this->driver);

            // 
            this->pipelineInfo = vkh::VsGraphicsPipelineCreateInfoConstruction();

            // create required buffers
            this->rawBindings = vkt::Vector<VkVertexInputBindingDescription>(this->driver->getAllocator(), vkh::VkBufferCreateInfo{ .size = sizeof(VkVertexInputBindingDescription) * 8u, .usage = {.eTransferSrc = 1, .eUniformBuffer = 1, .eStorageBuffer = 1, .eRayTracing = 1 } }, VMA_MEMORY_USAGE_CPU_TO_GPU);
            this->gpuBindings = vkt::Vector<VkVertexInputBindingDescription>(this->driver->getAllocator(), vkh::VkBufferCreateInfo{ .size = sizeof(VkVertexInputBindingDescription) * 8u, .usage = {.eTransferDst = 1, .eUniformBuffer = 1, .eStorageBuffer = 1, .eRayTracing = 1 } }, VMA_MEMORY_USAGE_GPU_ONLY);
            this->rawAttributes = vkt::Vector<VkVertexInputAttributeDescription>(this->driver->getAllocator(), vkh::VkBufferCreateInfo{ .size = sizeof(VkVertexInputAttributeDescription) * 8u, .usage = {.eTransferSrc = 1, .eUniformBuffer = 1, .eStorageBuffer = 1, .eRayTracing = 1 } }, VMA_MEMORY_USAGE_CPU_TO_GPU);
            this->gpuAttributes = vkt::Vector<VkVertexInputAttributeDescription>(this->driver->getAllocator(), vkh::VkBufferCreateInfo{ .size = sizeof(VkVertexInputAttributeDescription) * 8u, .usage = {.eTransferDst = 1, .eUniformBuffer = 1, .eStorageBuffer = 1, .eRayTracing = 1 } }, VMA_MEMORY_USAGE_GPU_ONLY);

            // 
            //this->gpuMeshInfo = vkt::Vector<MeshInfo>(this->driver->getAllocator(), vkh::VkBufferCreateInfo{ .size = 16u, .usage = {.eTransferDst = 1, .eUniformBuffer = 1, .eStorageBuffer = 1, .eRayTracing = 1 } }, VMA_MEMORY_USAGE_GPU_ONLY);
            this->rawMeshInfo = vkt::Vector<MeshInfo>(this->driver->getAllocator(), vkh::VkBufferCreateInfo{ .size = 16u, .usage = {.eTransferSrc = 1, .eUniformBuffer = 1, .eStorageBuffer = 1, .eRayTracing = 1 } }, VMA_MEMORY_USAGE_CPU_TO_GPU);

            // Internal Instance Map Per Global Node
            this->rawInstanceMap = vkt::Vector<uint32_t>(this->driver->getAllocator(), vkh::VkBufferCreateInfo{ .size = 4u * 64u, .usage = {.eTransferSrc = 1, .eUniformBuffer = 1, .eStorageBuffer = 1, .eRayTracing = 1 } }, VMA_MEMORY_USAGE_CPU_TO_GPU);
            this->gpuInstanceMap = vkt::Vector<uint32_t>(this->driver->getAllocator(), vkh::VkBufferCreateInfo{ .size = 4u * 64u, .usage = {.eTransferDst = 1, .eUniformBuffer = 1, .eStorageBuffer = 1, .eRayTracing = 1 } }, VMA_MEMORY_USAGE_GPU_ONLY);

            // for faster code, pre-initialize
            this->stages = vkt::vector_cast<vkh::VkPipelineShaderStageCreateInfo, vk::PipelineShaderStageCreateInfo>({
                vkt::makePipelineStageInfo(this->driver->getDevice(), vkt::readBinary("./shaders/rtrace/rasterize.vert.spv"), vk::ShaderStageFlagBits::eVertex),
                vkt::makePipelineStageInfo(this->driver->getDevice(), vkt::readBinary("./shaders/rtrace/rasterize.geom.spv"), vk::ShaderStageFlagBits::eGeometry),
                vkt::makePipelineStageInfo(this->driver->getDevice(), vkt::readBinary("./shaders/rtrace/rasterize.frag.spv"), vk::ShaderStageFlagBits::eFragment)
            });
            this->ctages = vkt::vector_cast<vkh::VkPipelineShaderStageCreateInfo, vk::PipelineShaderStageCreateInfo>({
                vkt::makePipelineStageInfo(this->driver->getDevice(), vkt::readBinary("./shaders/rtrace/covergence.vert.spv"), vk::ShaderStageFlagBits::eVertex),
                vkt::makePipelineStageInfo(this->driver->getDevice(), vkt::readBinary("./shaders/rtrace/covergence.geom.spv"), vk::ShaderStageFlagBits::eGeometry),
                vkt::makePipelineStageInfo(this->driver->getDevice(), vkt::readBinary("./shaders/rtrace/covergence.frag.spv"), vk::ShaderStageFlagBits::eFragment)
            });

            // 
            this->quadStage = vkt::makePipelineStageInfo(this->driver->getDevice(), vkt::readBinary("./shaders/rtrace/quad.comp.spv"), vk::ShaderStageFlagBits::eCompute);
            
            // 
            //this->quadGenerator = vkt::createCompute(this->driver->getDevice(), std::string("./shaders/rtrace/quad.comp.spv"), this->context->unifiedPipelineLayout, this->driver->pipelineCache);

            // 
            auto allocInfo = vkt::MemoryAllocationInfo{};
            allocInfo.device = *driver;
            allocInfo.memoryProperties = driver->getMemoryProperties().memoryProperties;
            allocInfo.dispatch = driver->getDispatch();

            { //
                this->indexType = vk::IndexType::eNoneKHR;
                this->indexData = vkt::Vector<uint8_t>(allocInfo, vkh::VkBufferCreateInfo{
                    .size = AllocationUnitCount * sizeof(uint32_t) * 3u,
                    .usage = {.eTransferDst = 1, .eStorageTexelBuffer = 1, .eStorageBuffer = 1, .eIndexBuffer = 1, .eSharedDeviceAddress = 1 },
                });
                this->rawMeshInfo[0u].indexType = uint32_t(vk::IndexType::eNoneKHR) + 1u;

                // TODO: other platforms memory handling
                // create OpenGL version of buffers
#ifdef ENABLE_OPENGL_INTEROP
                glCreateBuffers(1u, &this->indexDataOGL.second);
                glCreateMemoryObjectsEXT(1u, &this->indexDataOGL.first);
                glImportMemoryWin32HandleEXT(this->indexDataOGL.first, this->indexData->getAllocationInfo().reqSize, GL_HANDLE_TYPE_OPAQUE_WIN32_EXT, this->indexData->getAllocationInfo().handle);
                glNamedBufferStorageMemEXT(this->indexDataOGL.second, AllocationUnitCount * 2u * sizeof(uint32_t), this->indexDataOGL.first, 0u);
#endif
            };

            // 
            for (uint32_t i = 0; i < 8; i++) {
                this->buildGInfo[0].geometry.triangles.vertexStride = sizeof(glm::vec4);
                this->buildGInfo[0].geometry.triangles.vertexFormat = VK_FORMAT_R32G32B32_SFLOAT;
                this->bindings[i] = vkt::Vector<uint8_t>(allocInfo, vkh::VkBufferCreateInfo{
                    .size = AllocationUnitCount * sizeof(glm::vec4) * 12u,
                    .usage = {.eTransferDst = 1, .eStorageTexelBuffer = 1, .eStorageBuffer = 1, .eVertexBuffer = 1, .eSharedDeviceAddress = 1 },
                });

                // TODO: other platforms memory handling
                // create OpenGL version of buffers
#ifdef ENABLE_OPENGL_INTEROP
                glCreateBuffers(1u, &this->bindingsOGL[i].second);
                glCreateMemoryObjectsEXT(1u, &this->bindingsOGL[i].first);
                glImportMemoryWin32HandleEXT(this->bindingsOGL[i].first, this->bindings[i]->getAllocationInfo().reqSize, GL_HANDLE_TYPE_OPAQUE_WIN32_EXT, this->bindings[i]->getAllocationInfo().handle);
                glNamedBufferStorageMemEXT(this->bindingsOGL[i].second, AllocationUnitCount * 6u * sizeof(uint32_t), this->bindingsOGL[i].first, 0u);
#endif
            };

            // FOR BUILD! 
            this->bdHeadInfo.geometryCount = this->buildGInfo.size();
            this->bdHeadInfo.ppGeometries = reinterpret_cast<vkh::VkAccelerationStructureGeometryKHR**>((this->buildGPtr = this->buildGInfo.data()).ptr());
            this->bdHeadInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;
            this->bdHeadInfo.flags = VK_BUILD_ACCELERATION_STRUCTURE_ALLOW_UPDATE_BIT_KHR | VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR;
            this->bdHeadInfo.geometryArrayOfPointers = false;

            // FOR BUILD! FULL GEOMETRY INFO! // originally, it should to be array (like as old version of LancER)
            this->buildGInfo[0u].geometry = vkh::VkAccelerationStructureGeometryTrianglesDataKHR{};
            this->buildGInfo[0u].geometry.triangles.indexType = VK_INDEX_TYPE_NONE_KHR;
            this->buildGInfo[0u].geometry.triangles.vertexFormat = VK_FORMAT_R32G32B32_SFLOAT;
            this->buildGInfo[0u].geometry.triangles.vertexStride = sizeof(glm::vec4);
            this->offsetInfo[0u].firstVertex = 0u; // FOR INDICED INCLUDED! 
            this->offsetInfo[0u].primitiveCount = 0u;
            this->offsetInfo[0u].primitiveOffset = 0u;
            this->offsetInfo[0u].transformOffset = 0u;

            // FOR CREATE! 
            this->bottomDataCreate[0u].geometryType = this->buildGInfo[0u].geometryType;
            this->bottomDataCreate[0u].maxVertexCount = static_cast<uint32_t>(AllocationUnitCount * 3u);
            this->bottomDataCreate[0u].maxPrimitiveCount = static_cast<uint32_t>(AllocationUnitCount);
            this->bottomDataCreate[0u].indexType = VK_INDEX_TYPE_NONE_KHR;
            this->bottomDataCreate[0u].vertexFormat = VK_FORMAT_R32G32B32_SFLOAT;
            this->bottomDataCreate[0u].allowsTransforms = true;

            // FOR CREATE! 
            this->bottomCreate.maxGeometryCount = this->bottomDataCreate.size();
            this->bottomCreate.pGeometryInfos = this->bottomDataCreate.data();
            this->bottomCreate.type = this->bdHeadInfo.type;
            this->bottomCreate.flags = this->bdHeadInfo.flags;

            // 
            return uTHIS;
        }

        // 
        virtual vkt::Vector<uint8_t>& getBindingBuffer(const uintptr_t& i) {
            return this->bindings[i];
        };

        // 
        virtual vkt::Vector<uint8_t>& getBindingBuffer() {
            return this->bindings[this->lastBindID];
        };

        //
        virtual vkt::Vector<uint8_t>& getIndexBuffer() {
            return this->indexData;
        };

        // 
        virtual const vkt::Vector<uint8_t>& getBindingBuffer(const uintptr_t& i) const {
            return this->bindings[i];
        };

        // 
        virtual const vkt::Vector<uint8_t>& getBindingBuffer() const {
            return this->bindings[this->lastBindID];
        };

        //
        virtual const vkt::Vector<uint8_t>& getIndexBuffer() const {
            return this->indexData;
        };

#ifdef ENABLE_OPENGL_INTEROP //
        virtual GLuint& getBindingBufferGL(const uintptr_t& i) {
            return this->bindingsOGL[i].second;
        };

        // 
        virtual GLuint& getBindingBufferGL() {
            return this->bindingsOGL[this->lastBindID].second;
        };

        // 
        virtual GLuint& getIndexBufferGL() {
            return this->indexDataOGL.second;
        };

        virtual const GLuint& getBindingBufferGL(const uintptr_t& i) const {
            return this->bindingsOGL[i].second;
        };

        // 
        virtual const GLuint& getBindingBufferGL() const {
            return this->bindingsOGL[this->lastBindID].second;
        };

        // 
        virtual const GLuint& getIndexBufferGL() const {
            return this->indexDataOGL.second;
        };
#endif

        // Win32 Only (currently)
        virtual HANDLE& getBindingMemoryHandle(const uintptr_t& i = 0) {
            return this->bindings[i]->info.handle;
        };

        // Win32 Only (currently)
        virtual HANDLE& getBindingMemoryHandle() {
            return this->bindings[this->lastBindID]->info.handle;
        };

        // Win32 Only (currently)
        virtual HANDLE& getIndexMemoryHandle() {
            return this->indexData->info.handle;
        };

        // Win32 Only (currently)
        virtual const HANDLE& getBindingMemoryHandle(const uintptr_t& i = 0) const {
            return this->bindings[i]->info.handle;
        };

        // Win32 Only (currently)
        virtual const HANDLE& getBindingMemoryHandle() const {
            return this->bindings[this->lastBindID]->info.handle;
        };

        // Win32 Only (currently)
        virtual const HANDLE& getIndexMemoryHandle() const {
            return this->indexData->info.handle;
        };

        // 
        virtual uPTR(Mesh) setMaterialID(const uint32_t& materialID = 0u) {
            this->rawMeshInfo[0u].materialID = materialID;
            return uTHIS;
        };

        // 
        virtual uPTR(Mesh) setThread(const vkt::uni_ptr<Thread>& thread) {
            this->thread = thread;
            return uTHIS;
        };


#ifdef ENABLE_OPENGL_INTEROP
        // WARNING: OpenGL `rawData` SHOULD TO BE  `CPU_TO_GPU`  BUFFER FOR HIGHER PERFORMANCE!!!
        virtual uPTR(Mesh) addBinding(const GLuint& rawData, const vkt::uni_arg<vkh::VkVertexInputBindingDescription>& binding = vkh::VkVertexInputBindingDescription{}) {
            const uintptr_t bindingID = this->vertexInputBindingDescriptions.size();
            this->vertexInputBindingDescriptions.resize(bindingID + 1u);
            this->vertexInputBindingDescriptions[bindingID] = binding;
            this->vertexInputBindingDescriptions[bindingID].binding = bindingID;
            this->rawBindings[bindingID] = this->vertexInputBindingDescriptions[bindingID];

            // 
            GLint glsize = 0u; glGetNamedBufferParameteriv(rawData, GL_BUFFER_SIZE, &glsize);
            const auto& range = this->bindRange[this->lastBindID = bindingID] = glsize;
            if (glsize > 0) {
                glCopyNamedBufferSubData(rawData, this->bindingsOGL[bindingID].second, 0u, this->bindings[bindingID].offset(), range);
            };
            return uTHIS;
        };

        // For JavaCPP compatibility (from LWJGL3 Pointer)
        //virtual uPTR(Mesh) addBinding(const GLuint& rawData, const vkh::VkVertexInputBindingDescription* binding) { return this->addBinding(rawData, *binding); };
        //virtual uPTR(Mesh) addBinding(const GLuint& rawData, const void* binding) { return this->addBinding(rawData, *(vkh::VkVertexInputBindingDescription*)binding); };
#endif

        // 
        template<class T = uint8_t>
        inline uPTR(Mesh) addBinding(const std::vector<T>& rawData, const vkt::uni_arg<vkh::VkVertexInputBindingDescription>& binding = vkh::VkVertexInputBindingDescription{}) {
            const uintptr_t bindingID = this->vertexInputBindingDescriptions.size();
            this->vertexInputBindingDescriptions.resize(bindingID + 1u);
            this->vertexInputBindingDescriptions[bindingID] = binding;
            this->vertexInputBindingDescriptions[bindingID].binding = bindingID;
            this->rawBindings[bindingID] = this->vertexInputBindingDescriptions[bindingID];
            const uint8_t* ptr = reinterpret_cast<const uint8_t*>(&rawData[0]);
            const auto& range = this->bindRange[this->lastBindID = bindingID] = rawData.size() * sizeof(T);
            if (rawData.data() && rawData.size() > 0) {
                this->thread->submitOnce([&, this](vk::CommandBuffer& cmd) {
                    for (vk::DeviceSize u = 0; u < range; u += 65536u) {
                        cmd.updateBuffer(this->bindings[bindingID], this->bindings[bindingID].offset() + u, std::min(65536ull, range - u), &ptr[u]);
                    };
                });
            };
            return uTHIS;
        };

        // For JavaCPP compatibility (from LWJGL3 Pointer)
        //template<class T = uint8_t> inline uPTR(Mesh) addBinding(const std::vector<T>& rawData, const vkh::VkVertexInputBindingDescription* binding) { return this->addBinding(rawData, *binding); };
        //template<class T = uint8_t> inline uPTR(Mesh) addBinding(const std::vector<T>& rawData, const void* binding) { return this->addBinding(rawData, *(vkh::VkVertexInputBindingDescription*)binding); };

        // 
        template<class T = uint8_t>
        inline uPTR(Mesh) addBinding(const vkt::Vector<T>& rawData, const vkt::uni_arg<vkh::VkVertexInputBindingDescription>& binding = vkh::VkVertexInputBindingDescription{}){
            const uintptr_t bindingID = this->vertexInputBindingDescriptions.size();
            this->vertexInputBindingDescriptions.resize(bindingID+1u);
            this->vertexInputBindingDescriptions[bindingID] = binding;
            this->vertexInputBindingDescriptions[bindingID].binding = static_cast<uint32_t>(bindingID);
            this->rawBindings[bindingID] = this->vertexInputBindingDescriptions[bindingID];
            this->bindRange[this->lastBindID = static_cast<uint32_t>(bindingID)] = rawData.range();
            if (rawData.has() && rawData.range() > 0) {
                this->thread->submitOnce([=, this](vk::CommandBuffer& cmd) {
                    cmd.copyBuffer(rawData, this->bindings[bindingID], { vk::BufferCopy{ rawData.offset(), this->bindings[bindingID].offset(), std::min(vk::DeviceSize(this->bindings[bindingID].range()), vk::DeviceSize(rawData.range())) } });
                });
            };
            return uTHIS;
        };

        // 
        template<class T = uint8_t>
        inline uPTR(Mesh) addBinding(const vkt::uni_arg<vkh::VkDescriptorBufferInfo>& rawData, const vkt::uni_arg<vkh::VkVertexInputBindingDescription>& binding = vkh::VkVertexInputBindingDescription{}) {
            const uintptr_t bindingID = this->vertexInputBindingDescriptions.size();
            this->vertexInputBindingDescriptions.resize(bindingID + 1u);
            this->vertexInputBindingDescriptions[bindingID] = binding;
            this->vertexInputBindingDescriptions[bindingID].binding = bindingID;
            this->rawBindings[bindingID] = this->vertexInputBindingDescriptions[bindingID];
            this->bindRange[this->lastBindID = bindingID] = rawData->range;
            if (rawData->buffer && rawData->range > 0) {
                this->thread->submitOnce([&, this](vk::CommandBuffer& cmd) {
                    cmd.copyBuffer(rawData->buffer, this->bindings[bindingID], { vk::BufferCopy{ rawData->offset, this->bindings[bindingID].offset(), std::min(this->bindings[bindingID].range(), rawData->range) } });
                });
            };
            return uTHIS;
        };

        // 
        virtual uPTR(Mesh) genQuads(const vk::DeviceSize& primitiveCount = 0u) {
            this->buildGInfo[0].geometry.triangles.indexType = this->bottomDataCreate[0].indexType = VkIndexType(this->indexType = vk::IndexType::eUint32);
            this->buildGInfo[0].geometry.triangles.indexData = this->indexData; // Force Use Index Data
            this->currentUnitCount = (this->primitiveCount = this->offsetInfo[0].primitiveCount = primitiveCount)*6u; this->needsQuads = true;
            return uTHIS;
        };

        // For JavaCPP compatibility (from LWJGL3 Pointer)
        //template<class T = uint8_t> inline uPTR(Mesh) addBinding(const vkt::Vector<T>& rawData, const vkh::VkVertexInputBindingDescription* binding) { return this->addBinding(rawData, *binding); };
        //template<class T = uint8_t> inline uPTR(Mesh) addBinding(const vkt::Vector<T>& rawData, const void* binding) { return this->addBinding(rawData, *(vkh::VkVertexInputBindingDescription*)binding); };

        // 
        virtual uPTR(Mesh) addAttribute(const vkt::uni_arg<vkh::VkVertexInputAttributeDescription>& attribute = vkh::VkVertexInputAttributeDescription{}, const bool& NotStub = true) {
            //const uintptr_t bindingID = attribute.binding;
            //const uintptr_t locationID = this->locationCounter++;
            const uintptr_t bindingID = this->lastBindID;
            const uintptr_t locationID = attribute->location;
            this->vertexInputAttributeDescriptions.resize(locationID+1u);
            this->vertexInputAttributeDescriptions[locationID] = attribute;
            this->vertexInputAttributeDescriptions[locationID].binding = static_cast<uint32_t>(bindingID);
            this->vertexInputAttributeDescriptions[locationID].location = static_cast<uint32_t>(locationID);
            this->rawAttributes[locationID] = this->vertexInputAttributeDescriptions[locationID];

            if (locationID == 0 && NotStub) { // 
                const auto& binding = this->vertexInputBindingDescriptions[bindingID];
                if (this->indexType == vk::IndexType::eNoneKHR) {
                    this->currentUnitCount = (this->bindRange[bindingID] / binding.stride);
                };

                // 
                this->buildGInfo[0].flags = { .eOpaque = 1 };
                this->offsetInfo[0].primitiveOffset = attribute->offset + this->bindings[bindingID].offset(); // !!WARNING!! Also, unknown about needing `.offset()`... 
                this->buildGInfo[0].geometryType = VK_GEOMETRY_TYPE_TRIANGLES_KHR;
                this->buildGInfo[0].geometry.triangles.vertexFormat = this->bottomDataCreate[0].vertexFormat = attribute->format;
                this->buildGInfo[0].geometry.triangles.vertexStride = binding.stride;
                this->buildGInfo[0].geometry.triangles.vertexData = this->bindings[bindingID];

                // Fix vec4 formats into vec3, without alpha (but still can be passed by stride value)
                if (attribute->format == VK_FORMAT_R32G32B32A32_SFLOAT) this->buildGInfo[0].geometry.triangles.vertexFormat = VK_FORMAT_R32G32B32_SFLOAT;
                if (attribute->format == VK_FORMAT_R16G16B16A16_SFLOAT) this->buildGInfo[0].geometry.triangles.vertexFormat = VK_FORMAT_R16G16B16_SFLOAT;
            };

            if (locationID == 1u && NotStub) { rawMeshInfo[0u].hasTexcoord = 1; };
            if (locationID == 2u && NotStub) { rawMeshInfo[0u].hasNormal = 1; };
            if (locationID == 3u && NotStub) { rawMeshInfo[0u].hasTangent = 1; };

            if (this->indexType == vk::IndexType::eNoneKHR) {
                this->primitiveCount = this->currentUnitCount / (this->needsQuads ? 4u : 3u);
            };

            return uTHIS;
        };

        // For JavaCPP compatibility (from LWJGL3 Pointer)
        //virtual uPTR(Mesh) addAttribute(const vkh::VkVertexInputAttributeDescription* attribute, const bool& NotStub = true) { return this->addAttribute(*attribute, NotStub); };
        //virtual uPTR(Mesh) addAttribute(const void* attribute, const bool& NotStub = true) { return this->addAttribute(*(vkh::VkVertexInputAttributeDescription*)attribute, NotStub); };

        // 
        virtual uPTR(Mesh) setIndexCount(const uint32_t& count = 32768u) {
            this->primitiveCount = this->offsetInfo[0].primitiveCount = (this->currentUnitCount = count) / (this->needsQuads ? 4u : 3u); // Mul*3u
            return uTHIS;
        };

        // 
        virtual uPTR(Mesh) setPrimitiveCount(const uint32_t& count = 32768u) {
            this->currentUnitCount = (this->primitiveCount = this->offsetInfo[0].primitiveCount = count) * (this->needsQuads ? 4u : 3u); // Mul*3u
            return uTHIS;
        };

        // 
        virtual uPTR(Mesh) manifestIndex(const vk::IndexType& type, const vk::DeviceSize& primitiveCount = 0) {
            this->indexType = type;
            if (primitiveCount) {
                this->currentUnitCount = (this->primitiveCount = this->offsetInfo[0].primitiveCount = static_cast<uint32_t>(primitiveCount)) * 3u;
                this->rawMeshInfo[0u].indexType = uint32_t(type) + 1u;
            };
            return uTHIS;
        };

        // 
        //template<class T = uint8_t>
        //inline uPTR(Mesh) setIndexData(const vkt::Vector<T>& rawIndices, const vk::IndexType& type) {
        inline uPTR(Mesh) setIndexData(const vkt::uni_arg<vkh::VkDescriptorBufferInfo>& rawIndices, const vk::IndexType& type) {
            vk::DeviceSize count = 0u;
            uint32_t stride = 1u;
            if (rawIndices.has()) {
                switch (type) { // 
                    case vk::IndexType::eUint32:   count = rawIndices->range / (stride = 4u); break;
                    case vk::IndexType::eUint16:   count = rawIndices->range / (stride = 2u); break;
                    case vk::IndexType::eUint8EXT: count = rawIndices->range / (stride = 1u); break;
                    default: count = 0u;
                };

                // 
                this->thread->submitOnce([&, this](vk::CommandBuffer& cmd) {
                    cmd.copyBuffer(rawIndices->buffer, this->indexData, { vk::BufferCopy{ rawIndices->offset, this->indexData.offset(), std::min(this->indexData.range(), rawIndices->range) } });
                });
            };

            // 
            this->indexType = (rawIndices.has() && type != vk::IndexType::eNoneKHR) ? type : vk::IndexType::eNoneKHR;
            this->rawMeshInfo[0].indexType = uint32_t(this->buildGInfo[0].geometry.triangles.indexType = this->bottomDataCreate[0].indexType = VkIndexType(this->indexType)) + 1u;
            this->buildGInfo[0u].geometry.triangles.indexData = this->indexData;
            this->offsetInfo[0u].firstVertex = this->indexData.offset() / stride; // Trying these scheme (change indices to next)
            this->offsetInfo[0u].primitiveCount = (this->primitiveCount = (this->currentUnitCount = count) / (this->needsQuads ? 4u : 3u));

            // 
            return uTHIS;
        };

        // 
        virtual uPTR(Mesh) setIndexData(const vkt::Vector<uint32_t>& rawIndices) { return this->setIndexData(rawIndices.getDescriptor(),vk::IndexType::eUint32); };
        virtual uPTR(Mesh) setIndexData(const vkt::Vector<uint16_t>& rawIndices) { return this->setIndexData(rawIndices.getDescriptor(),vk::IndexType::eUint16); };
        virtual uPTR(Mesh) setIndexData(const vkt::Vector<uint8_t >& rawIndices) { return this->setIndexData(rawIndices.getDescriptor(),vk::IndexType::eUint8EXT); };
        virtual uPTR(Mesh) setIndexData() { return this->setIndexData({},vk::IndexType::eNoneKHR); };

        // some type dependent
        template<class T = uint8_t>
        inline uPTR(Mesh) setIndexData(const vkt::Vector<T>& rawIndices = {}) { return this->setIndexData(rawIndices); };

        // 
        virtual uPTR(Mesh) setDriver(const vkt::uni_ptr<Driver>& driver = {}){
            this->driver = driver;
            return uTHIS;
        };

        // 
        virtual uPTR(Mesh) increaseInstanceCount(const uint32_t& mapID = 0u, const uint32_t& instanceCount = 1u) {
            this->rawInstanceMap[this->instanceCount] = mapID;
            this->instanceCount += instanceCount;
            return uTHIS;
        };

        // 
        virtual uPTR(Mesh) setInstanceCount(const uint32_t& instanceCount = 1u) {
            this->instanceCount = instanceCount;
            return uTHIS;
        };

        // MORE useful for instanced data
        virtual uPTR(Mesh) setTransformData(const vkh::VkDescriptorBufferInfo& transformData = {}, const uint32_t& stride = sizeof(glm::vec4)) {
            this->offsetInfo[0].transformOffset = transformData.offset;
            this->buildGInfo[0].geometry.triangles.transformData = transformData.buffer;
            this->transformStride = stride; // used for instanced correction
            this->rawMeshInfo[0u].hasTransform = 1u;
            return uTHIS;
        };

        // 
        virtual uPTR(Mesh) copyBuffers(const vk::CommandBuffer& buildCommand = {}) {
            buildCommand.copyBuffer(this->rawAttributes , this->gpuAttributes , { vk::BufferCopy{ this->rawAttributes .offset(), this->gpuAttributes .offset(), this->gpuAttributes .range() } });
            buildCommand.copyBuffer(this->rawBindings   , this->gpuBindings   , { vk::BufferCopy{ this->rawBindings   .offset(), this->gpuBindings   .offset(), this->gpuBindings   .range() } });
            buildCommand.copyBuffer(this->rawInstanceMap, this->gpuInstanceMap, { vk::BufferCopy{ this->rawInstanceMap.offset(), this->gpuInstanceMap.offset(), this->gpuInstanceMap.range() } });
            //vkt::commandBarrier(buildCommand);
            return uTHIS;
        };

        // 
        virtual uPTR(Mesh) buildAccelerationStructure(const vk::CommandBuffer& buildCommand = {}, const glm::uvec4& meshData = glm::uvec4(0u)) {
            if (this->accelerationStructure) { this->updateGeometry(); }
            else { this->createAccelerationStructure(); };

            // 
            if (this->needsQuads && buildCommand) { //
                this->quadInfo.layout = this->context->unifiedPipelineLayout;
                this->quadInfo.stage = this->quadStage;
                this->quadGenerator = vkt::createCompute(driver->getDevice(), vkt::FixConstruction(this->quadStage), vk::PipelineLayout(this->quadInfo.layout), driver->getPipelineCache());

                buildCommand.bindDescriptorSets(vk::PipelineBindPoint::eCompute, this->context->unifiedPipelineLayout, 0ull, this->context->descriptorSets, {});
                buildCommand.bindPipeline(vk::PipelineBindPoint::eCompute, this->quadGenerator);
                buildCommand.pushConstants<glm::uvec4>(this->context->unifiedPipelineLayout, vkh::VkShaderStageFlags{.eVertex = 1, .eGeometry = 1, .eFragment = 1, .eCompute = 1, .eRaygen = 1, .eClosestHit = 1, .eMiss = 1 }.hpp(), 0u, { meshData });
                buildCommand.dispatch(vkt::tiled(this->primitiveCount, 256u) * 256u, 1u, 1u);
                this->offsetInfo[0u].primitiveCount = (this->primitiveCount <<= 1);
                this->needsQuads = false;
            };

            // 
            this->bdHeadInfo.geometryCount = this->buildGInfo.size();
            this->bdHeadInfo.dstAccelerationStructure = this->accelerationStructure;
            this->bdHeadInfo.ppGeometries = (this->buildGPtr = this->buildGInfo.data()).ptr();
            this->bdHeadInfo.scratchData = this->gpuScratchBuffer;
            this->bdHeadInfo.update = this->needsUpdate;

            // 
            if (buildCommand) {
                buildCommand.buildAccelerationStructureKHR(1u, this->bdHeadInfo, reinterpret_cast<vk::AccelerationStructureBuildOffsetInfoKHR**>((this->offsetPtr = this->offsetInfo.data()).ptr()), this->driver->getDispatch());
                this->needsUpdate = true;
            } else {
                driver->getDevice().buildAccelerationStructureKHR(1u, this->bdHeadInfo, reinterpret_cast<vk::AccelerationStructureBuildOffsetInfoKHR**>((this->offsetPtr = this->offsetInfo.data()).ptr()), this->driver->getDispatch());
            }
            return uTHIS;
        };

        //
        virtual uPTR(Mesh) updateGeometry() { // TODO: Reserved For FUTURE!

            return uTHIS;
        };

        // Create Or Rebuild Acceleration Structure
        virtual uPTR(Mesh) createAccelerationStructure() {
            bool Is2ndFormat = // TODO: more correct length detection
                this->buildGInfo[0].geometry.triangles.vertexFormat == VK_FORMAT_R32G32_SFLOAT ||
                this->buildGInfo[0].geometry.triangles.vertexFormat == VK_FORMAT_R16G16B16_SFLOAT ||
                this->buildGInfo[0].geometry.triangles.vertexFormat == VK_FORMAT_R16G16_SFLOAT;

            // 
            if (!this->accelerationStructure) { // create acceleration structure fastly...
                this->accelerationStructure = this->driver->getDevice().createAccelerationStructureKHR(this->bottomCreate, nullptr, this->driver->getDispatch());

                //
                auto requirements = this->driver->getDevice().getAccelerationStructureMemoryRequirementsKHR(vkh::VkAccelerationStructureMemoryRequirementsInfoKHR{
                    .type = VK_ACCELERATION_STRUCTURE_MEMORY_REQUIREMENTS_TYPE_OBJECT_KHR,
                    .buildType = VK_ACCELERATION_STRUCTURE_BUILD_TYPE_DEVICE_KHR,
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
                this->driver->getDevice().bindAccelerationStructureMemoryKHR(1u,&vkh::VkBindAccelerationStructureMemoryInfoKHR{
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
        virtual uPTR(Mesh) createRasterizePipeline() {
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
        virtual uPTR(Mesh) createCovergenceCommand(const vk::CommandBuffer& rasterCommand = {}, const glm::uvec4& meshData = glm::uvec4(0u)) { // UNIT ONLY!
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
            rasterCommand.beginRenderPass(vk::RenderPassBeginInfo(this->context->refRenderPass(), this->context->deferredFramebuffer, renderArea, static_cast<uint32_t>(clearValues.size()), clearValues.data()), vk::SubpassContents::eInline);
            rasterCommand.setViewport(0, { viewport });
            rasterCommand.setScissor(0, { renderArea });
            rasterCommand.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, this->context->unifiedPipelineLayout, 0ull, this->context->descriptorSets, {});
            rasterCommand.bindPipeline(vk::PipelineBindPoint::eGraphics, this->covergenceState);
            rasterCommand.bindVertexBuffers(0u, buffers, offsets);
            rasterCommand.pushConstants<glm::uvec4>(this->context->unifiedPipelineLayout, vkh::VkShaderStageFlags{.eVertex = 1, .eGeometry = 1, .eFragment = 1, .eCompute = 1, .eRaygen = 1, .eClosestHit = 1, .eMiss = 1 }.hpp(), 0u, { meshData });

            // Make Draw Instanced
            this->rawMeshInfo[0u].prmCount = this->primitiveCount;
            if (this->indexType != vk::IndexType::eNoneKHR) { // PLC Mode
                const uintptr_t voffset = this->bindings[this->vertexInputAttributeDescriptions[0u].binding].offset(); // !!WARNING!!
                rasterCommand.bindIndexBuffer(this->indexData, this->indexData.offset(), this->indexType);
                rasterCommand.drawIndexed(this->currentUnitCount, this->instanceCount, this->offsetInfo[0u].firstVertex, voffset, 0u);
            }
            else { // VAL Mode
                rasterCommand.draw(this->currentUnitCount, this->instanceCount, this->offsetInfo[0u].firstVertex, 0u);
            };
            rasterCommand.endRenderPass();
            //vkt::commandBarrier(rasterCommand);

            // 
            return uTHIS;
        };

        // Create Secondary Command With Pipeline
        virtual uPTR(Mesh) createRasterizeCommand(const vk::CommandBuffer& rasterCommand = {}, const glm::uvec4& meshData = glm::uvec4(0u)) { // UNIT ONLY!
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

            // samples itself
            rasterCommand.beginRenderPass(vk::RenderPassBeginInfo(this->context->refRenderPass(), this->context->deferredFramebuffer, renderArea, static_cast<uint32_t>(clearValues.size()), clearValues.data()), vk::SubpassContents::eInline);
            rasterCommand.setViewport(0, { viewport });
            rasterCommand.setScissor(0, { renderArea });
            rasterCommand.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, this->context->unifiedPipelineLayout, 0ull, this->context->descriptorSets, {});
            rasterCommand.bindPipeline(vk::PipelineBindPoint::eGraphics, this->rasterizationState);
            rasterCommand.bindVertexBuffers(0u, buffers, offsets);
            rasterCommand.pushConstants<glm::uvec4>(this->context->unifiedPipelineLayout, vkh::VkShaderStageFlags{.eVertex = 1, .eGeometry = 1, .eFragment = 1, .eCompute = 1, .eRaygen = 1, .eClosestHit = 1, .eMiss = 1 }.hpp(), 0u, { meshData });

            // Make Draw Instanced
            this->rawMeshInfo[0u].prmCount = this->primitiveCount;
            if (this->indexType != vk::IndexType::eNoneKHR) { // PLC Mode
                const uintptr_t voffset = this->bindings[this->vertexInputAttributeDescriptions[0u].binding].offset(); // !!WARNING!!
                rasterCommand.bindIndexBuffer(this->indexData, this->indexData.offset(), this->indexType);
                rasterCommand.drawIndexed(this->currentUnitCount, this->instanceCount, this->offsetInfo[0u].firstVertex, voffset, 0u);
            }
            else { // VAL Mode
                rasterCommand.draw(this->currentUnitCount, this->instanceCount, this->offsetInfo[0u].firstVertex, 0u);
            };
            rasterCommand.endRenderPass();
            //vkt::commandBarrier(rasterCommand);

            // 
            return uTHIS;
        };

    // 
    protected: friend Mesh; friend Node; friend Renderer; // GPU Vertex and Attribute Data
        vkt::Vector<uint8_t> indexData = {}; 
#ifdef ENABLE_OPENGL_INTEROP
        std::pair<GLuint, GLuint> indexDataOGL = {};
#endif

        vk::DeviceSize currentUnitCount = 0u;
        vk::IndexType indexType = vk::IndexType::eNoneKHR;
        vk::DeviceSize AllocationUnitCount = 32768;

        // 
        //uint32_t indexCount = 0u, vertexCount = 0u, instanceCount = 0u;
        uint32_t primitiveCount = 0u, instanceCount = 0u;
        bool needsUpdate = false;
        bool needsQuads = false;

        // 
        std::array<vkt::Vector<uint8_t>, 8> bindings = {};
#ifdef ENABLE_OPENGL_INTEROP
        std::array<std::pair<GLuint, GLuint>, 8> bindingsOGL = {};
#endif
        std::array<uint32_t, 8> bindRange = {0,0,0,0,0,0,0,0};

        // 
        //std::vector<vkt::Vector<uint8_t>> bindings = {};
        std::vector<vkh::VkVertexInputBindingDescription> vertexInputBindingDescriptions = {};
        std::vector<vkh::VkVertexInputAttributeDescription> vertexInputAttributeDescriptions = {};
        std::vector<vkh::VkPipelineShaderStageCreateInfo> stages = {};
        std::vector<vkh::VkPipelineShaderStageCreateInfo> ctages = {};

        vkh::VkComputePipelineCreateInfo quadInfo = {};
        vkh::VkPipelineShaderStageCreateInfo quadStage = {};
        vk::Pipeline quadGenerator = {};

        // accumulated by "Instance" for instanced rendering
        vkt::Vector<glm::vec4> gpuTransformData = {};
        uint32_t transformStride = sizeof(glm::vec4);
        uint32_t lastBindID = 0u, locationCounter = 0u;

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
        std::vector<vkh::VkAccelerationStructureBuildOffsetInfoKHR>   offsetInfo = { {} };
        vkt::uni_arg<vkh::VkAccelerationStructureBuildOffsetInfoKHR*> offsetPtr = {};

        // CAN BE MULTIPLE! (single element of array, array of array[0])
        std::vector<vkh::VkAccelerationStructureGeometryKHR>   buildGInfo = { {} };
        vkt::uni_arg<vkh::VkAccelerationStructureGeometryKHR*> buildGPtr = {};


        // 
        vk::Pipeline rasterizationState = {}; // Vertex Input can changed, so use individual rasterization stages
        vk::Pipeline covergenceState = {};

        // WIP buffer bindings
        vkt::Vector<vkh::VkVertexInputAttributeDescription> rawAttributes = {};
        vkt::Vector<vkh::VkVertexInputAttributeDescription> gpuAttributes = {};
        vkt::Vector<vkh::VkVertexInputBindingDescription> rawBindings = {};
        vkt::Vector<vkh::VkVertexInputBindingDescription> gpuBindings = {};

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
        //vkt::uni_ptr<Renderer> renderer = {};
    };

};
