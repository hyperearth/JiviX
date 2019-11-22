#pragma once

#include "../lib/core.hpp"
#include "../API/device.hpp"

namespace lancer {

    class Allocation_T : public std::enable_shared_from_this<Allocation_T> {
        protected: 
            friend Allocator;
            friend Device;

            Allocator allocator = nullptr;
            api::MemoryHeap memory = {};
            uint8_t* mapped = nullptr;

        public: 
            virtual void free() {};
            virtual uintptr_t getPtr() { return 0u; }; // xPEH TB
            virtual uintptr_t getCIP() { return 0u; }; // xPEH TB
            virtual uint8_t* getMapped() {  return nullptr; };

            ~Allocation_T(){ this->free(); };
             Allocation_T(const Allocator& allocator = {}) : allocator(allocator) {};

            inline const Device& getDevice() const { return allocator->getDevice(); };
    };

    class Allocator_T : public std::enable_shared_from_this<Allocator_T> {
        protected: 
            Device device = {};
            std::vector<std::weak_ptr<Allocation_T>> allocations = {};
            friend Device;

        public: 
            ~Allocator_T(){};
             Allocator_T(){};
             Allocator_T(const Device& dvc){};

            virtual void free() {};
            virtual void allocateForBuffer(api::Buffer* buffer, Allocation& allocation, const api::BufferCreateInfo& bfc = {}, const uintptr_t& ptx = 0u);
            virtual void allocateForImage(api::Image* image, Allocation& allocation, const api::ImageCreateInfo& bfc = {}, const uintptr_t& ptx = 0u);
            virtual void initialize(const Device& device = {});
            virtual Allocation&& createAllocation() { return std::make_shared<Allocation_T>(*this); };
            inline const Device& getDevice() const { return device; };
    };

    // 
    inline Device&& Device_T::initialize() {
        if (physicalHelper && device && !(*device)) {
            *device = ((api::PhysicalDevice&)(*physicalHelper)).createDevice(dfc);
        };

        // get VMA allocator for device
        if (!this->allocator && this->physicalHelper->getAllocator()) { this->allocator = this->physicalHelper->getAllocator(); };
        if ( this->allocator) { this->allocator->initialize(shared_from_this()); };

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
