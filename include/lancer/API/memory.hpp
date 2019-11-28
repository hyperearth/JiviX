#pragma once

#include "../lib/core.hpp"
#include "../API/device.hpp"

namespace lancer {

    class MemoryAllocation_T : public std::enable_shared_from_this<MemoryAllocation_T> {
        protected: 
            friend MemoryAllocator;
            friend DeviceMaker;
            MemoryAllocator allocator = nullptr;

        public: 
            virtual void free() {};
            virtual uintptr_t getPtr() { return 0u; }; // xPEH TB
            virtual uintptr_t getCIP() { return 0u; }; // xPEH TB
            virtual uint8_t* getMapped() {  return nullptr; };

            ~MemoryAllocation_T(){ this->free(); };
             MemoryAllocation_T(const MemoryAllocator& allocator = {}) : allocator(allocator) {};

            inline const DeviceMaker& getDevice() const { return allocator->getDevice(); };
    };

    class MemoryAllocator_T : public std::enable_shared_from_this<MemoryAllocator_T> {
        protected: 
            DeviceMaker device = {};
            std::vector<std::weak_ptr<MemoryAllocation_T>> allocations = {};
            friend DeviceMaker;

        public: 
            ~MemoryAllocator_T(){};
             MemoryAllocator_T(const DeviceMaker& dvc = {}, const uintptr_t& info = {});

            virtual void free() {};
            virtual MemoryAllocator&& allocateForBuffer(api::Buffer* buffer, MemoryAllocation& allocation, const api::BufferCreateInfo& bfc = {}, const uintptr_t& ptx = 0u) {};
            virtual MemoryAllocator&& allocateForImage(api::Image* image, MemoryAllocation& allocation, const api::ImageCreateInfo& bfc = {}, const uintptr_t& ptx = 0u) {};
            virtual MemoryAllocator&& initialize() {};
            virtual MemoryAllocator&& linkDevice(DeviceMaker&& device = {}) {};
            virtual MemoryAllocator&& linkDevice(const DeviceMaker& device = {}) {};
            virtual MemoryAllocation&& createAllocation() { return std::make_shared<MemoryAllocation_T>(*this); };
            inline const DeviceMaker& getDevice() const { return device; };
    };

    // 
    inline DeviceMaker&& Device_T::initialize() {
        if (physicalHelper && device && !(*device)) {
            *device = physicalHelper->least().createDevice(dfc);
        };

        // get VMA allocator for device
        if (!this->allocator && this->physicalHelper->getAllocator()) { this->allocator = this->physicalHelper->getAllocator(); };
        if ( this->allocator) {
             this->allocator->linkDevice(shared_from_this());
             this->allocator->initialize();
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
