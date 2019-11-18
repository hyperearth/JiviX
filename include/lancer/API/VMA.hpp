#pragma once

#include "../lib/core.hpp"
#include "../API/memory.hpp"
#include "../API/device.hpp"
#include "../API/buffer.hpp"
#include <VMA/vk_mem_alloc.h>
//#inc;ude <vulkan/VMA.hpp>

namespace lancer {

    // 
    class VMAllocator;

    class VMAllocation : public Allocation, public std::enable_shared_from_this<VMAllocation> {
        protected: 
            //std::shared_ptr<VMAllocator> allocator = {};
            VmaAllocation alloc = {};
            VmaAllocationInfo alcmc = {}; // least registered allocation, not necessary
            friend VMAllocator;

        public: 
            // unique constructor 
            VMAllocation(VmaAllocation& allocation, VmaAllocationInfo& alloc_info) : alloc(std::move(allocation)), alcmc(std::move(alloc_info)) {};

            virtual void Free() override { // after notify for de-allocation
                
            };

            virtual uintptr_t GetCIP() override { return uintptr_t(&alcmc); };
            virtual uintptr_t GetPtr() override { return uintptr_t(&alloc); };
            virtual uint8_t* GetMapped() override { return (uint8_t*)alcmc.pMappedData; };

            // TODO: smart de-allocate memory 
            //~VMAllocation(){
            //    this->Free(); // when object forcely removed (may be crashed)
            //}
    };

    class VMAllocator : public Allocator, public std::enable_shared_from_this<VMAllocator> {
        protected: 
            std::shared_ptr<Device> dvc = {};
            VmaAllocationCreateInfo amc = {}; // Template
            VmaAllocator vma = {};

        public: 
            VMAllocator(const std::shared_ptr<Device>& dvc) : dvc(device) {}; // TODO: Allocator Construction 

            // 
            virtual void AllocateForBuffer(api::Buffer* buffer, std::shared_ptr<Allocation>& allocation, const api::BufferCreateInfo& bfc = {}, const uintptr_t& ptx = 0u) override {
                vmaCreateBuffer(vma, (VkBufferCreateInfo*)&bfc, ptx?(VmaAllocationCreateInfo*)(ptx):(&amc), (VkBuffer*)buffer, (VmaAllocation*)allocation->GetPtr(), (VmaAllocationInfo*)allocation->GetCIP());
            };

            // 
            virtual void AllocateForImage(api::Image* image, std::shared_ptr<Allocation>& allocation, const api::ImageCreateInfo& imc = {}, const uintptr_t& ptx = 0u) override {
                vmaCreateImage(vma, (VkImageCreateInfo*)&imc, ptx?(VmaAllocationCreateInfo*)(ptx):(&amc), (VkImage*)image, (VmaAllocation*)allocation->GetPtr(), (VmaAllocationInfo*)allocation->GetCIP());
            };

            // 
            virtual void Initialize(const std::shared_ptr<Device>& device = {}) override (const std::shared_ptr<Device>& device) {
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
                //VmaAllocatorCreateInfo amc = {};
#ifdef VOLK_H_
                amc.pVulkanFunctions = &vfuncs;
#endif
                amc.physicalDevice = device->GetHelper();
                amc.device = api::Device(*device);
                amc.preferredLargeHeapBlockSize = 16 * sizeof(uint32_t);
                amc.flags = VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT || VMA_ALLOCATION_CREATE_MAPPED_BIT;
                amc.pAllocationCallbacks = nullptr;
                amc.pHeapSizeLimit = nullptr;
                vmaCreateAllocator(&amc,vma);
            };
    };
};
