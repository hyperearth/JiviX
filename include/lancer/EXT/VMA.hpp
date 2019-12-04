#pragma once

#include "../lib/core.hpp"
#include "../API/memory.hpp"
#include "../API/device.hpp"
#include "../API/buffer.hpp"
#include "../API/image.hpp"
#include <VMA/vk_mem_alloc.h>

#ifndef EXTENSION_VMA
#define EXTENSION_VMA
#endif

namespace lancer {

    // 
    class VMAllocator_T;
    using VMAllocator = std::shared_ptr<VMAllocator_T>;
    class VMAllocation_T;
    using VMAllocation = std::shared_ptr<VMAllocation_T>;

    struct VMAllocation_B {
        VmaAllocationInfo alcmc = {}; // least registered allocation, not necessary
        VmaAllocation alloc = {};
    };

    class VMAllocation_T : public MemoryAllocation_T {
        protected: 
            VMAllocator allocator = {};
            VMAllocation_B allocation;
            friend VMAllocator;
            friend MemoryAllocator;
            friend MemoryAllocation;
            bool sDF = false;

        public: 
            // unique constructor 
             VMAllocation_T(const VMAllocator& allocator = {}, const VMAllocation_B& allocation = {}) : allocator(allocator), allocation(allocation) {};
            inline ~VMAllocation_T();
            inline virtual MemoryAllocation freeImage(const ImageMaker& maker) override;
            inline virtual MemoryAllocation freeBuffer(const BufferMaker& maker) override;
            inline virtual MemoryAllocation free() override;
            inline virtual MemoryAllocation smartFree() override { sDF = true; return shared_from_this(); };
            inline virtual uintptr_t getCIP() override { return (uintptr_t)(&allocation.alcmc); };
            inline virtual uintptr_t getPtr() override { return (uintptr_t)(&allocation.alloc); };
            inline virtual uintptr_t getMemoryOffset() override { return allocation.alcmc.offset; };
            inline virtual size_t getMemorySize() override { return allocation.alcmc.size; };
            inline virtual uint8_t* getMapped() override { return (uint8_t*)allocation.alcmc.pMappedData; };
            inline virtual api::DeviceMemory getMemory() override { return allocation.alcmc.deviceMemory; };
            inline virtual const DeviceMaker& getDevice() const;
            inline virtual DeviceMaker& getDevice();
            inline virtual const MemoryAllocator& getAllocator() const;
            inline virtual MemoryAllocator& getAllocator();

            // TODO: smart de-allocate memory 
            //~VMAllocation(){
            //    this->Free(); // when object forcely removed (may be crashed)
            //}
    };

    class VMAllocator_T : public MemoryAllocator_T {
        protected:
            DeviceMaker device = {};
            VmaAllocatorCreateInfo  amc = {}; // Template
            VmaAllocator vma = {};

            friend MemoryAllocator;
            friend DeviceMaker;

        public: 
            ~VMAllocator_T() { vmaDestroyAllocator(vma); };
             VMAllocator_T(const DeviceMaker& dvc = {}, const uintptr_t& info = 0u) : device(dvc) {
                if (!!info) { amc = *((const VmaAllocatorCreateInfo*)info); }; // Re-Assign From Pointer
             };

            inline virtual MemoryAllocator allocateForRequirements(MemoryAllocation& allocation, const api::MemoryRequirements2& req = {}, const uintptr_t& ptx = 0u) override { 
                 vmaAllocateMemory(vma,(VkMemoryRequirements*)&req.memoryRequirements, (VmaAllocationCreateInfo*)ptx, (VmaAllocation*)allocation->getPtr(), (VmaAllocationInfo*)allocation->getCIP());
                 return shared_from_this(); };

            inline virtual MemoryAllocator allocateForBuffer(api::Buffer* buffer, MemoryAllocation& allocation, const api::BufferCreateInfo& bfc = {}, const uintptr_t& ptx = 0u) override {
                 VkBuffer buf = VK_NULL_HANDLE;
                 vmaCreateBuffer(vma, (VkBufferCreateInfo*)&bfc, (VmaAllocationCreateInfo*)ptx, &buf, (VmaAllocation*)allocation->getPtr(), (VmaAllocationInfo*)allocation->getCIP());
                 *buffer = buf;
                 return shared_from_this(); };

            inline virtual MemoryAllocator allocateForImage(api::Image* image, MemoryAllocation& allocation, const api::ImageCreateInfo& imc = {}, const uintptr_t& ptx = 0u) override {
                VkImage img = VK_NULL_HANDLE;
                vmaCreateImage(vma, (VkImageCreateInfo*)&imc, (VmaAllocationCreateInfo*)ptx, &img, (VmaAllocation*)allocation->getPtr(), (VmaAllocationInfo*)allocation->getCIP());
                *image = img;
                return shared_from_this(); };

            // Sometimes required special allocation
            inline virtual MemoryAllocation createAllocation(const uintptr_t& info = (uintptr_t)nullptr, const api::MemoryRequirements2& req = {}) override {
                //auto vma_info = (VmaAllocationCreateInfo*)info;
                //vmaAllocateMemory(vma,(VkMemoryRequirements*)&req.memoryRequirements,vma_info,&allocation.alloc,&allocation.alcmc);
                auto VmaAllocator = std::dynamic_pointer_cast<VMAllocator_T>(shared_from_this());
                return std::dynamic_pointer_cast<MemoryAllocation_T>(std::make_shared<VMAllocation_T>(VmaAllocator, VMAllocation_B()));
            };

            inline virtual uintptr_t least() override { return (uintptr_t)(&vma); };

            //
            inline virtual const DeviceMaker& getDevice() const override { return device; };
            inline virtual DeviceMaker& getDevice() override { return device; };

            //
            inline virtual MemoryAllocator linkDevice(const DeviceMaker& device = {}) override { this->device = device; return std::dynamic_pointer_cast<MemoryAllocator_T>(shared_from_this()); };
            inline virtual MemoryAllocator initialize(const uintptr_t& info = 0u) override {
                if (!!info) {
                    amc = *((const VmaAllocatorCreateInfo*)info); // Re-Assign From Pointer
                };
                amc.pVulkanFunctions = nullptr;

#ifdef VOLK_H_
                // load API calls for context
                //volkLoadDevice(device->least());

                // create VMA memory allocator (with Volk support)
                VolkDeviceTable vktable = {};
                volkLoadDeviceTable(&vktable, device->least());

                // VMA functions with Volk compatibility
                VmaVulkanFunctions vfuncs = {};
                vfuncs.vkAllocateMemory = vktable.vkAllocateMemory;
                vfuncs.vkBindBufferMemory = vktable.vkBindBufferMemory;
                vfuncs.vkBindImageMemory = vktable.vkBindImageMemory;
                vfuncs.vkCreateBuffer = vktable.vkCreateBuffer;
                vfuncs.vkCreateImage = vktable.vkCreateImage;
                vfuncs.vkDestroyBuffer = vktable.vkDestroyBuffer;
                vfuncs.vkDestroyImage = vktable.vkDestroyImage;
                vfuncs.vkFreeMemory = vktable.vkFreeMemory;
                vfuncs.vkGetBufferMemoryRequirements = vktable.vkGetBufferMemoryRequirements;
                vfuncs.vkGetBufferMemoryRequirements2KHR = vktable.vkGetBufferMemoryRequirements2KHR;
                vfuncs.vkGetImageMemoryRequirements = vktable.vkGetImageMemoryRequirements;
                vfuncs.vkGetImageMemoryRequirements2KHR = vktable.vkGetImageMemoryRequirements2KHR;
                vfuncs.vkGetPhysicalDeviceMemoryProperties = vkGetPhysicalDeviceMemoryProperties;
                vfuncs.vkGetPhysicalDeviceProperties = vkGetPhysicalDeviceProperties;
                vfuncs.vkMapMemory = vktable.vkMapMemory;
                vfuncs.vkUnmapMemory = vktable.vkUnmapMemory;
                vfuncs.vkInvalidateMappedMemoryRanges = vktable.vkInvalidateMappedMemoryRanges;
                amc.pVulkanFunctions = &vfuncs;
#endif

                amc.physicalDevice = (api::PhysicalDevice&)(*(device->getHelper()));
                amc.device = api::Device(*device);
                amc.preferredLargeHeapBlockSize = 16 * sizeof(uint32_t);
                amc.flags = VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT || VMA_ALLOCATION_CREATE_MAPPED_BIT;
                amc.pAllocationCallbacks = nullptr;
                amc.pHeapSizeLimit = nullptr;
                vmaCreateAllocator(&amc,&vma);

                // 
                //return std::dynamic_pointer_cast<MemoryAllocator_T>(shared_from_this());
                return shared_from_this();
            };
    };


    // Destruct Smart Memory 
    inline VMAllocation_T::~VMAllocation_T() { if (sDF) { vmaFreeMemory(*(VmaAllocator*)allocator->least(), allocation.alloc); }; };

    inline MemoryAllocation VMAllocation_T::freeImage(const ImageMaker& maker) {
        vmaDestroyImage(*(VmaAllocator*)allocator->least(), maker->least(), allocation.alloc);
        return shared_from_this();
    };
    inline MemoryAllocation VMAllocation_T::freeBuffer(const BufferMaker& maker) {
        vmaDestroyBuffer(*(VmaAllocator*)allocator->least(), maker->least(), allocation.alloc);
        return shared_from_this();
    };
    inline MemoryAllocation VMAllocation_T::free() {
        vmaFreeMemory(*(VmaAllocator*)allocator->least(), allocation.alloc); return shared_from_this();
    }; // after notify for de-allocation

    inline const DeviceMaker& VMAllocation_T::getDevice() const { return allocator->getDevice(); };
    inline DeviceMaker& VMAllocation_T::getDevice() { return allocator->getDevice(); };
    inline const MemoryAllocator& VMAllocation_T::getAllocator() const { return (MemoryAllocator&)allocator; };
    inline MemoryAllocator& VMAllocation_T::getAllocator() { return (MemoryAllocator&)allocator; };
};
