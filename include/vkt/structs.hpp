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


    // TODO: Needs To Complete GeometryBuffer
    class GeometryBuffer {
    protected:
        DeviceMaker device = {};
        friend GeometryBuffer;

    public:
        GeometryBuffer(const DeviceMaker& device = {}) {

        };



    };


    class AccelerationInstanced {
    protected:
        DeviceMaker device = {};
        friend AccelerationInstanced;

    public:
        AccelerationInstanced(const AccelerationInstanced& accel) {
            this->device = accel.device;
            this->topLevel = accel.topLevel;
            this->structure = accel.structure;
            this->rtscratch = accel.rtscratch;
            this->rtupload = accel.rtupload;
            this->rtgcache = accel.rtgcache;
            this->mtscratch = accel.mtscratch;
            this->mtupload = accel.mtupload;
            this->mtgcache = accel.mtgcache;
        };

        AccelerationInstanced(const DeviceMaker& device = {}, const api::AccelerationStructureCreateInfoNV& info = {}, const size_t& maxSize = 1024u) {
            if (!!device) {
                topLevel = device->createInstancedAcceleration(info, &structure);
                topLevel->linkGPURegion(mtgcache = device->createBufferMaker(api::BufferCreateInfo().setUsage(api::BufferUsageFlagBits::eRayTracingNV | api::BufferUsageFlagBits::eTransferDst), &rtgcache.buffer)->createRegion<lancer::GeometryInstance>(&rtgcache, 0u, maxSize));
                topLevel->linkCacheRegion(mtupload = device->createBufferMaker(api::BufferCreateInfo().setUsage(api::BufferUsageFlagBits::eRayTracingNV | api::BufferUsageFlagBits::eTransferSrc), &rtupload.buffer)->createRegion<lancer::GeometryInstance>(&rtupload, 0u, maxSize));
                topLevel->linkScratch(mtscratch = device->createBufferMaker(api::BufferCreateInfo().setUsage(api::BufferUsageFlagBits::eRayTracingNV), &rtscratch.buffer)->createRegion<uint8_t>(&rtscratch, 0u, maxSize * 8u));

                // Will Allocated Later 
                //mtscratch->least()->getCreateInfo().size = maxSize * 8u;
                //atscratch.usage = VMA_MEMORY_USAGE_GPU_ONLY;
                //mtscratch->least()->allocate(uintptr_t(&atscratch));

                // Create GPU Cache Buffer 
                mtgcache->least()->getCreateInfo().size = maxSize * sizeof(lancer::GeometryInstance);
                atgcache.usage = VMA_MEMORY_USAGE_GPU_ONLY;
                mtgcache->least()->allocate(uintptr_t(&atgcache));

                // Create From Host Buffer
                mtupload->least()->getCreateInfo().size = maxSize * sizeof(lancer::GeometryInstance);
                atupload.usage = VMA_MEMORY_USAGE_CPU_TO_GPU;
                atupload.flags = VMA_ALLOCATION_CREATE_MAPPED_BIT;
                mtupload->least()->allocate(uintptr_t(&atupload));

                // 
                //topLevel->create();
            };
        };

        // Instance Level 
        lancer::InstancedAcceleration topLevel = {};
        api::AccelerationStructureNV structure = {};

        // 
        VmaAllocationCreateInfo atscratch = {};
        VmaAllocationCreateInfo atupload = {};
        VmaAllocationCreateInfo atgcache = {};

        // 
        api::DescriptorBufferInfo rtscratch = {}; // BUFFER WILL SELF-ASSIGN!
        api::DescriptorBufferInfo rtupload = {}; // BUFFER WILL SELF-ASSIGN!
        api::DescriptorBufferInfo rtgcache = {}; // BUFFER WILL SELF-ASSIGN!

        // 
        std::shared_ptr<lancer::BufferRegion_T<uint8_t>> mtscratch = {};
        std::shared_ptr<lancer::BufferRegion_T<lancer::GeometryInstance>> mtupload = {};
        std::shared_ptr<lancer::BufferRegion_T<lancer::GeometryInstance>> mtgcache = {};

        // 
        AccelerationInstanced& updateCmd(api::CommandBuffer& cmdbuf) { return *this; };
        AccelerationInstanced& setDevice(const DeviceMaker& device) { this->device = device; return *this; };

        // Compatible With (lancer::InstancedAcceleration)
        lancer::InstancedAcceleration_T* operator->() { return &(*topLevel); };
        operator lancer::InstancedAcceleration&() { return topLevel; };
        operator const lancer::InstancedAcceleration&() const { return topLevel; };
    };


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
        AccelerationGeometry& updateCmd(api::CommandBuffer& cmdbuf) { return *this; };
        AccelerationGeometry& setDevice(const DeviceMaker& device) { this->device = device; return *this; };

        // Compatible With (lancer::InstancedAcceleration)
        lancer::GeometryAcceleration_T* operator->() { return &(*lowLevel); };
        operator lancer::GeometryAcceleration& () { return lowLevel; };
        operator const lancer::GeometryAcceleration& () const { return lowLevel; };

    };


};
