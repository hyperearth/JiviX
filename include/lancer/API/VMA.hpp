#pragma once

#include "../lib/core.hpp"
#include "../API/buffer.hpp"
//#inc;ude <vulkan/VMA.hpp>

namespace lancer {

    class VMAllocation : public Allocation, public std::enable_shared_from_this<VMAllocation> {
        protected: 
            //std::shared_ptr<VMAllocator> allocator = {};
            VmaAllocation alloc = {};
            VmaAllocationInfo almc = {}; // least registered allocation, not necessary

        public: 
            VMAllocation(VmaAllocation& allocation){
                alloc = std::move(allocation);
            };

            virtual void Free() override { // after notify for de-allocation

            };

            virtual void SetCIP(const uintptr_t& cip) override {
                almc = *((VmaAllocationInfo*)cip);
            };

            virtual uintptr_t GetCIP() override {
                return uintptr_t(&almc);
            };

            virtual uintptr_t GetPtr() override {
                return uintptr_t(&alloc);
            };

            virtual uint8_t* GetMapped() override {
                return nullptr; // TODO: get mapped memory 
            };

            // TODO: smart de-allocate memory 
            //~VMAllocation(){
            //    this->Free(); // when object forcely removed (may be crashed)
            //}
    };

    class VMAllocator : public Allocator, public std::enable_shared_from_this<VMAllocator> {
        protected: 
            std::shared_ptr<Device> dvc = {};
            VmaAllocator vma = {};
            VmaAllocationInfo almc = {}; // Template

        public: 
            VMAllocator(std::shared_ptr<Device>& device){
                dvc = device;
            };

            virtual void AllocateForBuffer(api::Buffer* buffer, std::shared_ptr<Allocation>& allocation, const api::BufferCreateInfo& bfc = {}) override {
                vmaCreateBuffer(vma, (VkBufferCreateInfo*)&bfc, &almc, (VkBuffer*)buffer, (VmaAllocation*)allocation->GetPtr(), nullptr);
                allocation->SetCIP(almc);
            };
        
            virtual void AllocateForImage(api::Image* image, std::shared_ptr<Allocation>& allocation, const api::ImageCreateInfo& imc = {}) override {
                vmaCreateImage(vma, (VkImageCreateInfo*)&imc, &almc, (VkImage*)image, (VmaAllocation*)allocation->GetPtr(), nullptr);
                allocation->SetCIP(almc);
            };
    };
};
