#pragma once

#include "../lib/core.hpp"
#include "../API/buffer.hpp"
//#inc;ude <vulkan/VMA.hpp>

namespace lancer {
    
    class VMAllocation : public Allocation, public std::enable_shared_from_this<VMAllocation> {
        protected: 
            std::shared_ptr<VMAllocator> allocator = {};
            VmaAllocation alloc = {};
            VmaAllocationInfo almc = {}; // least registered allocation, not necessary

        public: 
            virtual void Free() override { // after notify for de-allocation
                
            };

            virtual uintptr_t GetPtr() override {
                return uintptr_t(&alloc);
            };

            virtual uint8_t* GetMapped() override {
                return nullptr; // TODO: get mapped memory 
            };

            //~VMAllocation(){
            //    this->Free(); // when object forcely removed (may be crashed)
            //}
    };

    // TODO: Allocator base class 
    class VMAllocator : public Allocator, public std::enable_shared_from_this<VMAllocator> {
        protected: 
            std::shared_ptr<Device> dvc = {}; // NECESSARY
            VmaAllocationInfo almc = {};
            VmaAllocator vma = {};

        public: 

            virtual AllocateForBuffer(vk::Buffer* buffer, std::shared_ptr<Allocation>& allocation, api::BufferCreateInfo bfc = {}) override {
                vmaCreateBuffer(vma, &bfc, &almc, buffer, (VmaAllocation*)allocation->GetPtr(), nullptr);
            };
        
            virtual AllocateForImage(vk::Image* buffer, std::shared_ptr<Allocation>& allocation, api::ImageCreateInfo imc = {}) override {
                vmaCreateImage(vma, &imc, &almc, buffer, (VmaAllocation*)allocation->GetPtr(), nullptr);
            };
    };

};
