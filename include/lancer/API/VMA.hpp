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
            VmaAllocationCreateInfo amc = {}; // Template
            VmaAllocator vma = {};

        public: 
            VMAllocator(const std::shared_ptr<Device>& dvc) : dvc(device) {};

            // 
            virtual void AllocateForBuffer(api::Buffer* buffer, std::shared_ptr<Allocation>& allocation, const api::BufferCreateInfo& bfc = {}) override {
                vmaCreateBuffer(vma, (VkBufferCreateInfo*)&bfc, &amc, (VkBuffer*)buffer, (VmaAllocation*)allocation->GetPtr(), (VmaAllocationInfo*)allocation->GetCIP());
            };

            // 
            virtual void AllocateForImage(api::Image* image, std::shared_ptr<Allocation>& allocation, const api::ImageCreateInfo& imc = {}) override {
                vmaCreateImage(vma, (VkImageCreateInfo*)&imc, &amc, (VkImage*)image, (VmaAllocation*)allocation->GetPtr(), (VmaAllocationInfo*)allocation->GetCIP());
            };
    };

    
    // TODO: buffer copying data and zero-initializer

    
    template<class T = uint8_t>
    class BufferRegion {
    public:
        BufferRegion(api::DescriptorBufferInfo* bufInfo, const std::shared_ptr<Buffer>& buffer, api::DeviceSize size = 0ull, api::DeviceSize offset = 0u) : buffer(buffer) {
            bufInfo->buffer = (api::Buffer)(*buffer);
            bufInfo->offset = offset;
            bufInfo->range = size * sizeof(T);
            //this->map();
        };

        T* const& map() { mapped = (T*)((uint8_t*)buffer->GetMapped() + bufInfo->offset); return mapped; };
        //void unmap() { buffer->unmap(); };

        T* const& data() { this->map(); return mapped; };
        const T*& data() const { return mapped; };

        // 
        size_t size() const { return size_t(bufInfo->range / sizeof(T)); };
        const api::DeviceSize& range() const { return bufInfo->range; };

        // at function 
        const T& at(const uintptr_t& i) const { return mapped[i]; };
        T& at(const uintptr_t& i) { return mapped[i]; };

        // array operator 
        const T& operator [] (const uintptr_t& i) const { return at(i); };
        T& operator [] (const uintptr_t& i) { return at(i); };

        // begin ptr
        const T*& begin() const { return data(); };
        T* const& begin() { return map(); };

        // end ptr
        const T*& end() const { return &at(size() - 1ul); };
        T* end() { return &at(size() - 1ul); };

        // 
        operator const api::DescriptorBufferInfo&() const { return &bufInfo; };
        operator const api::Buffer&() const { return *buffer; };
        const api::DeviceSize& offset() const { return bufInfo->offset; };

    protected:
        T* mapped = {};
        std::shared_ptr<Buffer> buffer = {};
        api::DescriptorBufferInfo* bufInfo = {};
    };

    template<class T = uint8_t>
    class Vector {
    public:
        Vector() {}
        Vector(api::DescriptorBufferInfo* bufInfo, const std::shared_ptr<Buffer>& buffer, api::DeviceSize size = 0ull, api::DeviceSize offset = 0u) {
            region = std::make_shared<BufferRegion<T>>(bufInfo, buffer, size, offset);
        };
        Vector(const std::shared_ptr<BufferRegion<T>>& region) : region(region) {};
        Vector(const Vector<T>& vector) : region(vector.region) {};

        // map through
        T* const& map() { return region->map(); };
        void unmap() { return region->unmap(); };

        T* const& data() { return region->data(); };
        const T*& data() const { return region->data(); };

        // sizing 
        size_t size() const { return region->size(); };
        const api::DeviceSize& range() const { return region->range(); };

        // at function 
        const T& at(const uintptr_t& i) const { return region->at(i); };
        T& at(const uintptr_t& i) { return region->at(i); };

        // array operator 
        const T& operator [] (const uintptr_t& i) const { at(i); };
        T& operator [] (const uintptr_t& i) { return at(i); };

        // begin ptr
        const T*& begin() const { region->begin(); };
        T* const& begin() { return region->begin(); };

        // end ptr
        const T* end() const { return region->end(); };
        T* end() { return region->end(); };

        // 
        operator const api::DescriptorBufferInfo&() const { return *region; };
        operator const api::Buffer&() const { return *region; };
        const api::DeviceSize& offset() const { return region->offset(); };

    protected:
        std::shared_ptr<BufferRegion<T>> region = {};
    };



};


