#pragma once

#include "../lib/core.hpp"
#include "../API/device.hpp"
#include "../API/memory.hpp"
#include "../API/buffer.hpp"
#include <VMA/vk_mem_alloc.h>
//#inc;ude <vulkan/VMA.hpp>

namespace lancer {

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
            virtual void AllocateForBuffer(api::Buffer* buffer, std::shared_ptr<Allocation>& allocation, const api::BufferCreateInfo& bfc = {}, const uint32_t& ptx = 0u) override {
                vmaCreateBuffer(vma, (VkBufferCreateInfo*)&bfc, ptx?(VmaAllocationCreateInfo*)(ptx):(&amc), (VkBuffer*)buffer, (VmaAllocation*)allocation->GetPtr(), (VmaAllocationInfo*)allocation->GetCIP());
            };

            // 
            virtual void AllocateForImage(api::Image* image, std::shared_ptr<Allocation>& allocation, const api::ImageCreateInfo& imc = {}, const uint32_t& ptx = 0u) override {
                vmaCreateImage(vma, (VkImageCreateInfo*)&imc, ptx?(VmaAllocationCreateInfo*)(ptx):(&amc), (VkImage*)image, (VmaAllocation*)allocation->GetPtr(), (VmaAllocationInfo*)allocation->GetCIP());
            };
    };

    
    // TODO: buffer copying data and zero-initializer





};


