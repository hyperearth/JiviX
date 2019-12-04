#pragma once

#include "../lib/core.hpp"
#include "../API/device.hpp"

namespace lancer {

    enum class MemoryType : uint32_t {
        eCustom = 0u, 
        eBuffer = 1u, 
        eImage = 2u, 
    };

    class MemoryAllocation_T : public std::enable_shared_from_this<MemoryAllocation_T> {
        protected: 
            friend MemoryAllocator;
            friend DeviceMaker;
            MemoryAllocator allocator = nullptr;

        public: 
            inline virtual MemoryAllocation free() { return shared_from_this(); };
            inline virtual MemoryAllocation freeImage(const ImageMaker& maker) { return shared_from_this(); };
            inline virtual MemoryAllocation freeBuffer(const BufferMaker& maker) { return shared_from_this(); };
            inline virtual MemoryAllocation smartFree() { return shared_from_this(); };
            inline virtual uintptr_t getPtr() { return 0u; }; // xPEH TB
            inline virtual uintptr_t getCIP() { return 0u; }; // xPEH TB
            inline virtual uint8_t* getMapped() { return nullptr; };
            inline virtual api::DeviceMemory getMemory() { return {}; };
            inline virtual uintptr_t getMemoryOffset() { return 0u; };
            inline virtual size_t getMemorySize() { return 0u; };

            ~MemoryAllocation_T(){  };
             MemoryAllocation_T(const MemoryAllocator& allocator = {}) : allocator(allocator) {};

            inline virtual const DeviceMaker& getDevice() const;
            inline virtual DeviceMaker& getDevice();
            inline virtual const MemoryAllocator& getAllocator() const;
            inline virtual MemoryAllocator& getAllocator();
    };

    class MemoryAllocator_T : public std::enable_shared_from_this<MemoryAllocator_T> {
        protected: 
            DeviceMaker device = {};
            std::vector<std::weak_ptr<MemoryAllocation_T>> allocations = {};
            friend DeviceMaker;

        public: 
            ~MemoryAllocator_T(){};
             MemoryAllocator_T(const DeviceMaker& dvc = {}, const uintptr_t& info = {}) : device(device) {};

            inline virtual void free() {};
            inline virtual MemoryAllocator allocateForBuffer(api::Buffer* buffer, MemoryAllocation& allocation, const api::BufferCreateInfo& bfc = {}, const uintptr_t& ptx = 0u) { return shared_from_this(); };
            inline virtual MemoryAllocator allocateForImage(api::Image* image, MemoryAllocation& allocation, const api::ImageCreateInfo& bfc = {}, const uintptr_t& ptx = 0u) { return shared_from_this(); };
            inline virtual MemoryAllocator allocateForRequirements(MemoryAllocation& allocation, const api::MemoryRequirements2& req = {}, const uintptr_t& ptx = 0u) { return shared_from_this(); };
            inline virtual MemoryAllocator initialize(const uintptr_t& info = 0u) { return shared_from_this(); };
            inline virtual MemoryAllocator linkDevice(const DeviceMaker& device = {}) { return shared_from_this(); };
            inline virtual MemoryAllocation createAllocation(const api::MemoryRequirements2& req = {}, const uintptr_t& info = (uintptr_t)nullptr) { return std::make_shared<MemoryAllocation_T>(shared_from_this()); };
            inline virtual const DeviceMaker& getDevice() const { return device; };
            inline virtual DeviceMaker& getDevice() { return device; };
            inline virtual uintptr_t least() { return 0u; };
    };

    // Define later
    inline const DeviceMaker& MemoryAllocation_T::getDevice() const { return allocator->getDevice(); };
    inline DeviceMaker& MemoryAllocation_T::getDevice() { return allocator->getDevice(); };
    inline const MemoryAllocator& MemoryAllocation_T::getAllocator() const { return allocator; };
    inline MemoryAllocator& MemoryAllocation_T::getAllocator() { return allocator; };

    // 
    inline DeviceMaker Device_T::initialize() {
        if (physicalHelper && device && !(*device)) {
            *device = physicalHelper->least().createDevice(dfc);
        };

        // get VMA allocator for device
        //if (!this->allocator && this->physicalHelper->getAllocator()) { this->allocator = this->physicalHelper->getAllocator(); };
        if (this->allocator) {
            this->allocator->linkDevice(shared_from_this())->initialize();
        };

        // descriptor pool
        if ( this->descriptorPool && !(*this->descriptorPool))
        {
            // pool sizes, and create descriptor pool
            std::vector<api::DescriptorPoolSize> psizes = { };
            psizes.push_back(api::DescriptorPoolSize().setType(api::DescriptorType::eStorageBuffer).setDescriptorCount(128));
            psizes.push_back(api::DescriptorPoolSize().setType(api::DescriptorType::eStorageTexelBuffer).setDescriptorCount(128));
            psizes.push_back(api::DescriptorPoolSize().setType(api::DescriptorType::eInlineUniformBlockEXT).setDescriptorCount(128));
            psizes.push_back(api::DescriptorPoolSize().setType(api::DescriptorType::eUniformBuffer).setDescriptorCount(128));
            psizes.push_back(api::DescriptorPoolSize().setType(api::DescriptorType::eAccelerationStructureNV).setDescriptorCount(128));

            const auto inlineDescPool = api::DescriptorPoolInlineUniformBlockCreateInfoEXT().setMaxInlineUniformBlockBindings(2u);
            *this->descriptorPool = api::Device(*this).createDescriptorPool(api::DescriptorPoolCreateInfo().setPNext(&inlineDescPool).setPPoolSizes(psizes.data()).setPoolSizeCount(psizes.size()).setMaxSets(256).setFlags(api::DescriptorPoolCreateFlagBits::eFreeDescriptorSet | api::DescriptorPoolCreateFlagBits::eUpdateAfterBindEXT));
        };

        // pipeline cache 
        if (!this->pipelineCache) { this->pipelineCache = api::Device(*this).createPipelineCache(api::PipelineCacheCreateInfo()); };

        // 
        return shared_from_this();
    };


    // TODO: Any implementations after device

};
