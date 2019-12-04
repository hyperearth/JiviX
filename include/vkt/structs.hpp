#pragma once

#include "./utils.hpp"

namespace vkt {
    using namespace lancer;

    // application surface format information structure
    //struct SurfaceFormat : public std::enable_shared_from_this<SurfaceFormat> {
    struct SurfaceFormat {
        api::Format colorFormat = {};
        api::Format depthFormat = {};
        api::Format stencilFormat = {};
        api::ColorSpaceKHR colorSpace = {};
        api::FormatProperties colorFormatProperties = {};
    };

    // framebuffer with command buffer and fence
    struct Framebuffer : public std::enable_shared_from_this<Framebuffer> {
        api::Framebuffer frameBuffer = {};
        api::CommandBuffer commandBuffer = {}; // terminal command (barrier)
        api::Fence waitFence = {};
        api::Semaphore semaphore = {};
    };

    // vertex layout (OUTDATED?!)
    // GraphicsPipelineMaker include that...
    //struct VertexLayout : public std::enable_shared_from_this<VertexLayout> {
    //    std::vector<api::VertexInputBindingDescription> inputBindings = {};
    //    std::vector<api::VertexInputAttributeDescription> inputAttributes = {};
    //};

#pragma pack(push, 1)
    struct GeometryInstance {
        //float transform[12];
        glm::mat3x4 transform;
        uint32_t instanceId : 24;
        uint32_t mask : 8;
        uint32_t instanceOffset : 24;
        uint32_t flags : 8;
        uint64_t accelerationStructureHandle;
    };
#pragma pack(pop)

    // UPDATE 04.12.2019 Dedicate Buffer Class
    template<class It = uint8_t>
    class Buffer {
    protected:
        DeviceMaker device = {};
        friend Buffer<It>;

    public:
        // 
        Buffer<It>(const DeviceMaker& device = {}, api::BufferUsageFlags usage = api::BufferUsageFlagBits::eRayTracingNV | api::BufferUsageFlagBits::eTransferDst, const size_t& maxSize = 1024u) : device(device), rBuffer(api::DescriptorBufferInfo{}) {
            if (!!device) {
                mBuffer = device->createBufferMaker(api::BufferCreateInfo().setUsage(usage), &rBuffer.buffer)->createRegion<It>(&rBuffer, 0u, maxSize);
            };
        };

        // Copy From Another Unit
        Buffer<It>(const Buffer<It>& buffer) {
            this->device = buffer.device;
            this->rBuffer = buffer.rBuffer;
            this->vBuffer = buffer.vBuffer;
            this->mBuffer = std::make_shared<lancer::BufferRegion_T<It>>(buffer.mBuffer, this->rBuffer); // make region independent
        };

        // 
        std::shared_ptr<lancer::BufferRegion_T<It>> mBuffer = {};
        api::DescriptorBufferInfo rBuffer = {};
        VmaAllocationCreateInfo vBuffer = {};

        // Region Anyways Will Imported from pointer
        Buffer<It>& setSize(const size_t& size = 1u) { rBuffer.range = mBuffer->least()->getCreateInfo().size = size * sizeof(It); return *this; };
        Buffer<It>& setRegion(const uintptr_t offset = 0u, const size_t& size = VK_WHOLE_SIZE) { rBuffer.range = size; rBuffer.offset = offset; return *this; };

        // 


        // 
        operator std::shared_ptr<lancer::BufferRegion_T<It>>& () { return mBuffer; };
        operator const std::shared_ptr<lancer::BufferRegion_T<It>>& () const { return mBuffer; };

        // 
        operator BufferMaker&() { return mBuffer->least(); };
        operator const BufferMaker& () const { return mBuffer->least(); };

        // 
        operator api::DescriptorBufferInfo& () { return rBuffer; };
        operator const api::DescriptorBufferInfo& () const { return rBuffer; };

        // 
        operator api::BufferCreateInfo& () { return mBuffer->least()->getCreateInfo(); };
        operator const api::BufferCreateInfo& () const { return mBuffer->least()->getCreateInfo(); };

        // 
        BufferMaker& getBufferMaker() { return mBuffer->least(); };
        const BufferMaker& getBufferMaker() const { return mBuffer->least(); };

        // 
        Buffer<It>& allocate() { mBuffer->least()->allocate(uintptr_t(&vBuffer)); return *this; };
    };


    // TODO: Add Uploading Support 
    template<class It = uint32_t, class Vt = glm::vec4>
    class GeometryBuffer {
    protected:
        DeviceMaker device = {};
        friend GeometryBuffer<It, Vt>;

    public:
        GeometryBuffer(const DeviceMaker& device = {}, const size_t& maxSize = 1024u) : device(device) {
            if (!!device) {
                mVertices = Buffer<Vt>(device, api::BufferUsageFlagBits::eRayTracingNV | api::BufferUsageFlagBits::eTransferDst | api::BufferUsageFlagBits::eVertexBuffer);
                mIndices = Buffer<It>(device, api::BufferUsageFlagBits::eRayTracingNV | api::BufferUsageFlagBits::eTransferDst | api::BufferUsageFlagBits::eIndexBuffer);
            };
        };

        GeometryBuffer(const GeometryBuffer<It, Vt>& buffer) {
            this->device = buffer.device;
            this->mIndices = buffer.mIndices;
            this->mVertices = buffer.mVertices;
        };

        // Setters (for later using)
        GeometryBuffer& setDevice(const DeviceMaker& device) { this->device = device; return *this; };
        GeometryBuffer& setIndices(const Buffer<It>& indices) { this->mIndices = indices; return *this; };
        GeometryBuffer& setVertices(const Buffer<Vt>& vertices) { this->mVertices = vertices; return *this; };

        // Convert into buffer range object
        operator std::shared_ptr<lancer::BufferRegion_T<It>>& () { return mIndices; };
        operator std::shared_ptr<lancer::BufferRegion_T<Vt>>& () { return mVertices; };
        operator const std::shared_ptr<lancer::BufferRegion_T<It>>& () const { return mIndices; };
        operator const std::shared_ptr<lancer::BufferRegion_T<Vt>>& () const { return mVertices; };

        // Convert into original buffer object 
        operator Buffer<It>& () { return mIndices; };
        operator Buffer<Vt>& () { return mVertices; };
        operator const Buffer<It>& () const { return mIndices; };
        operator const Buffer<Vt>& () const { return mVertices; };

        // 
        Buffer<Vt>& getVertices() { return mVertices; };
        Buffer<It>& getIndices() { return mIndices; };
        const Buffer<Vt>& getVertices() const { return mVertices; };
        const Buffer<It>& getIndices() const { return mIndices; };

        // Buffers Helpers
        Buffer<Vt> mVertices = {}, mVerticesUpload = {};
        Buffer<It> mIndices = {}, mIndicesUpload = {};
        Buffer<glm::mat3x4> mTransform = {}, mTransformUpload = {};

        // Push Into Acceleration
        GeometryBuffer& pushIntoAcceleration(lancer::GeometryAcceleration& acceleration) {
            if (this->mVertices) {
                acceleration->beginTriangles()->setVertex(this->mVertices);
                if (this->mIndices) acceleration->setIndices(this->mIndices);
                if (this->mTransform) acceleration->setTransform3x4(this->mTransform);
            };
        };
    };


    // 
    class AccelerationGeometry {
    protected:
        DeviceMaker device = {};
        friend AccelerationGeometry;

    public:
        AccelerationGeometry(const DeviceMaker& device = {}, const api::AccelerationStructureCreateInfoNV& info = {}, const size_t& maxSize = 1024u) {
            if (!!device) {
                lowLevel = device->createGeometryAcceleration(info, &structure);
                lowLevel->linkScratch(mtscratch = device->createBufferMaker(api::BufferCreateInfo().setUsage(api::BufferUsageFlagBits::eRayTracingNV), &rtscratch.buffer)->createRegion<uint8_t>(&rtscratch, 0u, maxSize));
            };
        };

        AccelerationGeometry(const AccelerationGeometry& accel) {
            this->device = accel.device;
            this->lowLevel = accel.lowLevel;
            this->structure = accel.structure;
            this->rtscratch = accel.rtscratch;
            this->mtscratch = accel.mtscratch;
        };

        // Geometry Level 
        lancer::GeometryAcceleration lowLevel = {};
        api::AccelerationStructureNV structure = {};
        api::DescriptorBufferInfo rtscratch = {}; // BUFFER WILL SELF-ASSIGN!
        std::shared_ptr<lancer::BufferRegion_T<uint8_t>> mtscratch = {};

        // 
        template<class It = uint32_t, class Vt = glm::vec4>
        AccelerationGeometry& pGeometry(const GeometryBuffer<It, Vt>& buffer = {}) { buffer.pushIntoAcceleration(lowLevel); return *this; };
        AccelerationGeometry& updateCmd(api::CommandBuffer& cmdbuf) { return *this; };
        AccelerationGeometry& setDevice(const DeviceMaker& device) { this->device = device; return *this; };

        // Compatible With (lancer::InstancedAcceleration)
        lancer::GeometryAcceleration_T* operator->() { return &(*lowLevel); };
        operator lancer::GeometryAcceleration& () { return lowLevel; };
        operator const lancer::GeometryAcceleration& () const { return lowLevel; };

    };


    // TODO: Add Geometry Acceleration Pushing Support
    class AccelerationInstanced {
    protected:
        DeviceMaker device = {};
        friend AccelerationInstanced;

    public:
        AccelerationInstanced(const AccelerationInstanced& accel) {
            this->device = accel.device;
            this->topLevel = accel.topLevel;
            this->structure = accel.structure;
            this->mScratch = accel.mScratch;
            this->mUpload = accel.mUpload;
            this->mCache = accel.mCache;
        };

        AccelerationInstanced(const DeviceMaker& device = {}, const api::AccelerationStructureCreateInfoNV& info = {}, const size_t& maxSize = 1024u) : device(device) {
            if (!!device) {
                topLevel = device->createInstancedAcceleration(info, &structure);
                topLevel->linkGPURegion(mCache = Buffer<lancer::GeometryInstance>(device, api::BufferUsageFlagBits::eRayTracingNV | api::BufferUsageFlagBits::eTransferDst, maxSize));
                topLevel->linkCacheRegion(mUpload = Buffer<lancer::GeometryInstance>(device, api::BufferUsageFlagBits::eRayTracingNV | api::BufferUsageFlagBits::eTransferSrc, maxSize));
                topLevel->linkScratch(mScratch = Buffer<uint8_t>(device, api::BufferUsageFlagBits::eRayTracingNV | api::BufferUsageFlagBits::eTransferDst, maxSize));

                // Create GPU Cache Buffer 
                mCache.vBuffer.usage = VMA_MEMORY_USAGE_GPU_ONLY;
                mCache.setSize(maxSize).allocate();

                // Scratch Buffer 
                mScratch.vBuffer.usage = VMA_MEMORY_USAGE_CPU_TO_GPU;
                mScratch.vBuffer.flags = VMA_ALLOCATION_CREATE_MAPPED_BIT;
                mScratch.setSize(maxSize * 8u);//.allocate();

                // Create From Host Buffer
                mUpload.vBuffer.usage = VMA_MEMORY_USAGE_GPU_ONLY;
                mUpload.setSize(maxSize).allocate();
            };
        };

        // Instance Level 
        lancer::InstancedAcceleration topLevel = {};
        api::AccelerationStructureNV structure = {};

        // Buffers
        Buffer<uint8_t> mScratch = {};
        Buffer<lancer::GeometryInstance> mUpload = {};
        Buffer<lancer::GeometryInstance> mCache = {};

        // 
        AccelerationInstanced& setDevice(const DeviceMaker& device) { this->device = device; return *this; };
        AccelerationInstanced& updateCmd(api::CommandBuffer& cmdbuf, const bool& updateOnly = false) {
            topLevel->create();
            topLevel->uploadCmd(cmdbuf);
            lancer::commandBarrier(cmdbuf); // barrier util require for transfer
            topLevel->createCmd(cmdbuf, updateOnly);
            lancer::commandBarrier(cmdbuf);
            return *this;
        };

        // Compatible With (lancer::InstancedAcceleration)
        lancer::InstancedAcceleration_T* operator->() { return &(*topLevel); };
        operator lancer::InstancedAcceleration& () { return topLevel; };
        operator const lancer::InstancedAcceleration& () const { return topLevel; };
    };


};
