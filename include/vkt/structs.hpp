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

    public: // 
        inline Buffer<It>(const DeviceMaker& device = {}, api::BufferUsageFlags usage = api::BufferUsageFlagBits::eRayTracingNV | api::BufferUsageFlagBits::eTransferDst, const size_t& maxSize = 1024u) : device(device), rBuffer(api::DescriptorBufferInfo{}) {
            rBuffer.offset = 0u, rBuffer.range = maxSize * sizeof(It);
            if (!!device) {
                mBuffer = device->createBufferMaker(api::BufferCreateInfo().setUsage(usage).setSize(maxSize*sizeof(It)), &(rBuffer.buffer))->createRegion<It>(&rBuffer, 0u, maxSize);
            };
        };

        // Copy From Another Unit
        inline Buffer<It>(const Buffer<It>& buffer) {
            this->device = buffer.device;
            this->vBuffer = buffer.vBuffer;
            this->rBuffer = buffer.rBuffer;
            this->rBuffer.buffer = buffer.mBuffer->handle();
            this->mBuffer = std::make_shared<lancer::BufferRegion_T<It>>(buffer.mBuffer, &this->rBuffer); // make region independent
            this->mBuffer->least()->link(&this->rBuffer.buffer); // Fix Wrong Pointer Issue 
        };

        inline Buffer<It>& operator=(const Buffer<It>& buffer) {
            this->device = buffer.device;
            this->vBuffer = buffer.vBuffer;
            this->rBuffer = buffer.rBuffer;
            this->rBuffer.buffer = buffer.mBuffer->handle();
            this->mBuffer = std::make_shared<lancer::BufferRegion_T<It>>(buffer.mBuffer, &this->rBuffer); // make region independent
            this->mBuffer->least()->link(&this->rBuffer.buffer); // Fix Wrong Pointer Issue 
            return *this;
        };


        // Region Anyways Will Imported from pointer
        inline Buffer<It>& setSize(const size_t& size = 1u) { rBuffer.range = mBuffer->least()->getCreateInfo().size = size * sizeof(It); return *this; };
        inline Buffer<It>& setRegion(const uintptr_t offset = 0u, const size_t& size = VK_WHOLE_SIZE) { rBuffer.range = size * sizeof(It); rBuffer.offset = offset; return *this; };

        // 
        //inline size_t& getSize() { return std::min(rBuffer.range, mBuffer->least()->getCreateInfo().size) / sizeof(It); };
        inline const size_t getSize() const { return std::min(rBuffer.range, mBuffer->least()->getCreateInfo().size) / sizeof(It); };

        // 
        inline operator std::shared_ptr<lancer::BufferRegion_T<It>>& () { return mBuffer; };
        inline operator const std::shared_ptr<lancer::BufferRegion_T<It>>& () const { return mBuffer; };

        // 
        inline operator BufferMaker&() { return mBuffer->least(); };
        inline operator const BufferMaker& () const { return mBuffer->least(); };

        // 
        inline operator api::DescriptorBufferInfo& () { return rBuffer; };
        inline operator const api::DescriptorBufferInfo& () const { return rBuffer; };

        // 
        inline operator api::BufferCreateInfo& () { return mBuffer->least()->getCreateInfo(); };
        inline operator const api::BufferCreateInfo& () const { return mBuffer->least()->getCreateInfo(); };

        // 
        inline BufferMaker& getBufferMaker() { return mBuffer->least(); };
        inline const BufferMaker& getBufferMaker() const { return mBuffer->least(); };

        // 
        inline Buffer<It>& allocate() { 
            mBuffer->least()->allocate(uintptr_t(&vBuffer)); 
            return *this;
        };


    public: // 
        std::shared_ptr<lancer::BufferRegion_T<It>> mBuffer = {};
        api::DescriptorBufferInfo rBuffer = {};
        VmaAllocationCreateInfo vBuffer = {};
    };


    template<class It = uint8_t>
    class BufferUploader {
    protected:
        DeviceMaker device = {};
        friend BufferUploader<It>;

    public: // 
        inline BufferUploader<It>(const DeviceMaker& device = {}, Buffer<It>* pGPU = nullptr, std::vector<It>* pCPU = nullptr, const size_t& minSize = 1024u) : device(device), pCPU(pCPU), pGPU(pGPU) {
            if (!!device) {
                mCPU = Buffer<It>(device, api::BufferUsageFlagBits::eRayTracingNV | api::BufferUsageFlagBits::eTransferSrc, std::min(std::max(minSize, pGPU->getSize()), pCPU->size()));
                mCPU.vBuffer.usage = VMA_MEMORY_USAGE_CPU_TO_GPU;
                mCPU.vBuffer.flags = VMA_ALLOCATION_CREATE_MAPPED_BIT;
                mCPU.allocate();
            };
        };

        inline BufferUploader<It>(const BufferUploader<It>& another) {
            this->device = another.device;
            this->pGPU = another.pGPU;
            this->pCPU = another.pCPU;
            this->mCPU = another.mCPU;
        };

        inline BufferUploader<It>& operator=(const BufferUploader<It>& another) {
            this->device = another.device;
            this->pGPU = another.pGPU;
            this->pCPU = another.pCPU;
            this->mCPU = another.mCPU;
            return *this;
        };

        inline BufferUploader<It>& uploadCmd(api::CommandBuffer& cmdbuf) {
            std::memcpy(mCPU.mBuffer->map(), pCPU->data(), pCPU->size() * sizeof(It));
            cmdbuf.copyBuffer(mCPU.mBuffer->handle(), pGPU->mBuffer->handle(), { vk::BufferCopy(0u,pGPU->mBuffer->offset(),mCPU.getSize()) });
            lancer::commandBarrier(cmdbuf);
            return *this;
        };

    public: // 
        std::vector<It>* pCPU = nullptr;
        Buffer<It>* pGPU = nullptr;
        Buffer<It> mCPU = {};
    };



    // TODO: Add Uploading Support 
    template<class It = uint32_t, class Vt = glm::vec3>
    class GeometryBuffer {
    protected:
        DeviceMaker device = {};
        friend GeometryBuffer<It, Vt>;

    public:
        // 
        inline GeometryBuffer(const DeviceMaker& device = {}, const size_t& maxSize = 1024u) : device(device) {
            if (!!device) {
                mVertices = Buffer<Vt>(device, api::BufferUsageFlagBits::eRayTracingNV | api::BufferUsageFlagBits::eTransferDst | api::BufferUsageFlagBits::eVertexBuffer | api::BufferUsageFlagBits::eStorageBuffer | api::BufferUsageFlagBits::eUniformBuffer);
                mIndices = Buffer<It>(device, api::BufferUsageFlagBits::eRayTracingNV | api::BufferUsageFlagBits::eTransferDst | api::BufferUsageFlagBits::eIndexBuffer | api::BufferUsageFlagBits::eStorageBuffer | api::BufferUsageFlagBits::eUniformBuffer);
                mTransform = Buffer<glm::mat3x4>(device, api::BufferUsageFlagBits::eRayTracingNV | api::BufferUsageFlagBits::eTransferDst | api::BufferUsageFlagBits::eStorageBuffer | api::BufferUsageFlagBits::eUniformBuffer);

                mTransform.allocate();
                mVertices.allocate();
                mIndices.allocate();
            };
        };

        // 
        inline GeometryBuffer(const GeometryBuffer<It, Vt>& buffer) {
            this->device = buffer.device;
            this->mIndices = buffer.mIndices;
            this->mVertices = buffer.mVertices;
            this->mTransform = buffer.mTransform;
        };

        // 
        inline GeometryBuffer& operator=(const GeometryBuffer<It,Vt>& buffer) {
            this->device = buffer.device;
            this->mIndices = buffer.mIndices;
            this->mVertices = buffer.mVertices;
            this->mTransform = buffer.mTransform;
            return *this;
        };

        // Setters (for later using)
        inline GeometryBuffer& allocate() { 
            mTransform.allocate();
            mVertices.allocate();
            mIndices.allocate();
            return *this;
        };
        inline GeometryBuffer& setDevice(const DeviceMaker& device) { this->device = device; return *this; };
        inline GeometryBuffer& setIndices(const Buffer<It>& indices) { this->mIndices = indices; return *this; };
        inline GeometryBuffer& setVertices(const Buffer<Vt>& vertices) { this->mVertices = vertices; return *this; };
        inline GeometryBuffer& setTransform(const Buffer<glm::mat3x4>& transform) { this->mTransform = transform; return *this; };

        // Convert into buffer range object
        inline operator std::shared_ptr<lancer::BufferRegion_T<It>>& () { return mIndices; };
        inline operator std::shared_ptr<lancer::BufferRegion_T<Vt>>& () { return mVertices; };
        inline operator std::shared_ptr<lancer::BufferRegion_T<glm::mat3x4>>& () { return mTransform; };
        inline operator const std::shared_ptr<lancer::BufferRegion_T<It>>& () const { return mIndices; };
        inline operator const std::shared_ptr<lancer::BufferRegion_T<Vt>>& () const { return mVertices; };
        inline operator const std::shared_ptr<lancer::BufferRegion_T<glm::mat3x4>>& () const { return mTransform; };

        // Convert into original buffer object 
        inline operator Buffer<It>& () { return mIndices; };
        inline operator Buffer<Vt>& () { return mVertices; };
        inline operator Buffer<glm::mat3x4>& () { return mTransform; };
        inline operator const Buffer<It>& () const { return mIndices; };
        inline operator const Buffer<Vt>& () const { return mVertices; };
        inline operator const Buffer<glm::mat3x4>& () const { return mTransform; };

        // 
        inline Buffer<Vt>& getVertices() { return mVertices; };
        inline Buffer<It>& getIndices() { return mIndices; };
        inline Buffer<glm::mat3x4>& getTransform() { return mTransform; };
        inline const Buffer<Vt>& getVertices() const { return mVertices; };
        inline const Buffer<It>& getIndices() const { return mIndices; };
        inline const Buffer<glm::mat3x4>& getTransform() const { return mTransform; };

        // Push Into Acceleration
        inline GeometryBuffer<It,Vt>& pushIntoAcceleration(lancer::GeometryAcceleration& acceleration) {
            if (this->mVertices.rBuffer.buffer) {
                acceleration->beginTriangles()->setVertex(this->mVertices);
                if (this->mIndices.rBuffer.buffer) acceleration->setIndices(this->mIndices);
                if (this->mTransform.rBuffer.buffer) acceleration->setTransform3x4(this->mTransform);
            };
            return *this;
        };

    public:
        // Buffers Helpers
        Buffer<Vt> mVertices = {};//, mVerticesUpload = {};
        Buffer<It> mIndices = {};//, mIndicesUpload = {};
        Buffer<glm::mat3x4> mTransform = {}; //, mTransformUpload = {};
    };


    // 
    class AccelerationGeometry {
    protected:
        DeviceMaker device = {};
        friend AccelerationGeometry;

    public:
        inline AccelerationGeometry(const DeviceMaker& device = {}, const api::AccelerationStructureCreateInfoNV& info = {}, const size_t& maxSize = 1024u) {
            if (!!device) {
                lowLevel = device->createGeometryAcceleration(info, &structure);
                lowLevel->linkScratch(mtscratch = device->createBufferMaker(api::BufferCreateInfo().setUsage(api::BufferUsageFlagBits::eRayTracingNV), &rtscratch.buffer)->createRegion<uint8_t>(&rtscratch, 0u, maxSize));
            };
        };

        inline AccelerationGeometry(const AccelerationGeometry& accel) {
            this->device = accel.device;
            this->lowLevel = accel.lowLevel;
            this->structure = accel.structure;
            this->rtscratch = accel.rtscratch;
            this->mtscratch = accel.mtscratch;
        };

        inline AccelerationGeometry& operator=(const AccelerationGeometry& accel) {
            this->device = accel.device;
            this->lowLevel = accel.lowLevel;
            this->structure = accel.structure;
            this->rtscratch = accel.rtscratch;
            this->mtscratch = accel.mtscratch;
            return *this;
        };

        // Setters (for later using)
        inline AccelerationGeometry& allocate() {
            VmaAllocationCreateInfo allocation = {};
            allocation.usage = VMA_MEMORY_USAGE_GPU_ONLY;
            this->lowLevel->create()->allocate(uintptr_t(&allocation));
            return *this;
        };

        // 
        template<class It = uint32_t, class Vt = glm::vec4>
        inline AccelerationGeometry& pGeometry(GeometryBuffer<It, Vt>& buffer = {}) { buffer.pushIntoAcceleration(lowLevel); return *this; };
        inline AccelerationGeometry& updateCmd(api::CommandBuffer& cmdbuf) { lowLevel->createCmd(cmdbuf); lancer::commandBarrier(cmdbuf); return *this; };
        inline AccelerationGeometry& setDevice(const DeviceMaker& device) { this->device = device; return *this; };
        inline AccelerationGeometry& writeForInstance(lancer::GeometryInstance& instance) { lowLevel->writeForInstance(instance); return *this; };

        // 
        inline const AccelerationGeometry& writeForInstance(lancer::GeometryInstance& instance) const { lowLevel->writeForInstance(instance); return *this; };

        // Compatible With (lancer::InstancedAcceleration)
        inline lancer::GeometryAcceleration_T* operator->() { return &(*lowLevel); };
        inline operator lancer::GeometryAcceleration& () { return lowLevel; };
        inline operator const lancer::GeometryAcceleration& () const { return lowLevel; };



    public: // Geometry Level 
        lancer::GeometryAcceleration lowLevel = {};
        api::AccelerationStructureNV structure = {};
        api::DescriptorBufferInfo rtscratch = {}; // BUFFER WILL SELF-ASSIGN!
        std::shared_ptr<lancer::BufferRegion_T<uint8_t>> mtscratch = {};
    };


    // TODO: Add Geometry Acceleration Pushing Support
    class AccelerationInstanced {
    protected:
        DeviceMaker device = {};
        friend AccelerationInstanced;

    public:
        // 
        inline AccelerationInstanced(const AccelerationInstanced& accel) {
            this->device = accel.device;
            this->topLevel = accel.topLevel;
            this->structure = accel.structure;
            this->mScratch = accel.mScratch;
            this->mUpload = accel.mUpload;
            this->mCache = accel.mCache;
        };

        inline AccelerationInstanced& operator=(const AccelerationInstanced& accel) {
            this->device = accel.device;
            this->topLevel = accel.topLevel;
            this->structure = accel.structure;
            this->mScratch = accel.mScratch;
            this->mUpload = accel.mUpload;
            this->mCache = accel.mCache;
            return *this;
        };

        // 
        inline AccelerationInstanced(const DeviceMaker& device = {}, const api::AccelerationStructureCreateInfoNV& info = {}, const size_t& maxSize = 1024u) : device(device) {
            if (!!device) {
                topLevel = device->createInstancedAcceleration(info, &structure);
                topLevel->linkScratch(mScratch = Buffer<uint8_t>(device, api::BufferUsageFlagBits::eRayTracingNV | api::BufferUsageFlagBits::eTransferDst, maxSize));
                topLevel->linkGPURegion(mCache = Buffer<lancer::GeometryInstance>(device, api::BufferUsageFlagBits::eRayTracingNV | api::BufferUsageFlagBits::eTransferDst, maxSize));
                topLevel->linkCacheRegion(mUpload = Buffer<lancer::GeometryInstance>(device, api::BufferUsageFlagBits::eRayTracingNV | api::BufferUsageFlagBits::eTransferSrc, maxSize));

                // Scratch Buffer 
                mScratch.vBuffer.usage = VMA_MEMORY_USAGE_GPU_ONLY;
                mScratch.setSize(maxSize * 8u);//.allocate();

                // Create GPU Cache Buffer 
                mCache.vBuffer.usage = VMA_MEMORY_USAGE_GPU_ONLY;
                mCache.setSize(maxSize).allocate();

                // Create From Host Buffer
                mUpload.vBuffer.usage = VMA_MEMORY_USAGE_CPU_TO_GPU;
                mUpload.vBuffer.flags = VMA_ALLOCATION_CREATE_MAPPED_BIT;
                mUpload.setSize(maxSize).allocate();
            };
        };

        // Setters (for later using)
        inline AccelerationInstanced& allocate() {
            VmaAllocationCreateInfo allocation = {};
            allocation.usage = VMA_MEMORY_USAGE_GPU_ONLY;
            this->topLevel->create()->allocate(uintptr_t(&allocation));
            return *this;
        };

        // Write For Descriptor
        inline const AccelerationInstanced& writeForDescription(api::WriteDescriptorSetAccelerationStructureNV* AS = nullptr) const { topLevel->writeForDescription(AS); return *this; };

        // 
        inline AccelerationInstanced& writeForDescription(api::WriteDescriptorSetAccelerationStructureNV* AS = nullptr) { topLevel->writeForDescription(AS); return *this; };
        inline AccelerationInstanced& pushGeometry(const AccelerationGeometry& geometry, const lancer::GeometryInstance& instance = {}) { geometry.writeForInstance(topLevel->pushInstance(instance)->getInstance()); return *this; };
        inline AccelerationInstanced& setDevice(const DeviceMaker& device) { this->device = device; return *this; };
        inline AccelerationInstanced& updateCmd(api::CommandBuffer& cmdbuf, const bool& updateOnly = false) {
            topLevel->createCmd(cmdbuf);
            topLevel->uploadCmd(cmdbuf);
            lancer::commandBarrier(cmdbuf); // barrier util require for transfer
            topLevel->createCmd(cmdbuf, updateOnly);
            lancer::commandBarrier(cmdbuf);
            return *this;
        };

        // Compatible With (lancer::InstancedAcceleration)
        inline lancer::InstancedAcceleration_T* operator->() { return &(*topLevel); };
        inline operator lancer::InstancedAcceleration& () { return topLevel; };
        inline operator const lancer::InstancedAcceleration& () const { return topLevel; };

    public: // Instance Level 
        lancer::InstancedAcceleration topLevel = {};
        api::AccelerationStructureNV structure = {};

        // Buffers
        Buffer<uint8_t> mScratch = {};
        Buffer<lancer::GeometryInstance> mUpload = {};
        Buffer<lancer::GeometryInstance> mCache = {};
    };


};
