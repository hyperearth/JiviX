#pragma once

#include "../lib/core.hpp"
#include "../API/memory.hpp"
#include "../API/device.hpp"
#include "../API/buffer.hpp"
#include "../API/image.hpp"
#include <VMA/vk_mem_alloc.h>


namespace lancer {

    // 
    class VMAllocator_T;
    using VMAllocator = std::shared_ptr<VMAllocator_T>;
    class VMAllocation_T;
    using VMAllocation = std::shared_ptr<VMAllocation_T>;


    class VMAllocation_T : public MemoryAllocation_T {
        protected: 
            //std::shared_ptr<VMAllocator> allocator = {};
            VmaAllocation alloc = {};
            VmaAllocationInfo alcmc = {}; // least registered allocation, not necessary
            friend VMAllocator;
            friend MemoryAllocator;
            friend DeviceMaker;

        public: 
            // unique constructor 
            ~VMAllocation_T() {};
             VMAllocation_T() {};
             VMAllocation_T(const DeviceMaker& dvc);
             VMAllocation_T(const VmaAllocation& allocation, const VmaAllocationInfo& alloc_info = {}) : alloc(std::move(allocation)), alcmc(std::move(alloc_info)) {
            };

            virtual void free() override {}; // after notify for de-allocation
            virtual uintptr_t getCIP() override { return uintptr_t(&alcmc); };
            virtual uintptr_t getPtr() override { return uintptr_t(&alloc); };
            virtual uint8_t* getMapped() override { return (uint8_t*)alcmc.pMappedData; };

            // TODO: smart de-allocate memory 
            //~VMAllocation(){
            //    this->Free(); // when object forcely removed (may be crashed)
            //}
    };

    class VMAllocator_T : public MemoryAllocator_T/*, public std::enable_shared_from_this<VMAllocator_T>*/ {
        protected: 
            DeviceMaker dvc = {};
            VmaAllocatorCreateInfo  amc = {}; // Template
            VmaAllocator vma = {};
            
            friend MemoryAllocator;
            friend DeviceMaker;

        public: 
            VMAllocator_T(const DeviceMaker& dvc = {}) : dvc(device) { if (dvc) { this->initialize(dvc); }; };

            // 
            virtual MemoryAllocator&& allocateForBuffer(api::Buffer* buffer, MemoryAllocation& allocation, const api::BufferCreateInfo& bfc = {}, const uintptr_t& ptx = 0u) override {
                vmaCreateBuffer(vma, (VkBufferCreateInfo*)&bfc, (VmaAllocationCreateInfo*)ptx, (VkBuffer*)buffer, (VmaAllocation*)allocation->getPtr(), (VmaAllocationInfo*)allocation->getCIP());
                return std::dynamic_pointer_cast<MemoryAllocator_T>(shared_from_this()); };

            // 
            virtual MemoryAllocator&& allocateForImage(api::Image* image, MemoryAllocation& allocation, const api::ImageCreateInfo& imc = {}, const uintptr_t& ptx = 0u) override {
                vmaCreateImage(vma, (VkImageCreateInfo*)&imc, (VmaAllocationCreateInfo*)ptx, (VkImage*)image, (VmaAllocation*)allocation->getPtr(), (VmaAllocationInfo*)allocation->getCIP());
                return std::dynamic_pointer_cast<MemoryAllocator_T>(shared_from_this()); };

            // Sometimes required special allocation
            virtual MemoryAllocation&& createAllocation() override { return std::dynamic_pointer_cast<MemoryAllocation_T>(std::make_shared<VMAllocation_T>(dvc)); };

            // 
            virtual MemoryAllocator&& linkDevice(DeviceMaker&& device = {}) override { this->dvc = std::move(device); return std::dynamic_pointer_cast<MemoryAllocator_T>(shared_from_this()); };
            virtual MemoryAllocator&& linkDevice(const DeviceMaker& device = {}) override { this->dvc = device; return std::dynamic_pointer_cast<MemoryAllocator_T>(shared_from_this()); };
            virtual MemoryAllocator&& initialize() override {
#ifdef VOLK_H_
                // load API calls for context
                volkLoadDevice(api::Device(*device));

                // create VMA memory allocator (with Volk support)
                VolkDeviceTable vktable = {};
                volkLoadDeviceTable(&vktable, api::Device(*device));

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
#endif

                // create Vma allocator
#ifdef VOLK_H_
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
                return std::dynamic_pointer_cast<MemoryAllocator_T>(shared_from_this());
            };
    };
};
