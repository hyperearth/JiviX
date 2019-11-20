#pragma once

#include <cstdint>
#include "../lib/core.hpp"
#include "../API/memory.hpp"


namespace lancer {
    // Vookoo-Like 
    class Buffer : public std::enable_shared_from_this<Buffer> {
        protected: 
            std::shared_ptr<Device> device = {};
            std::shared_ptr<Allocation> allocation = {}; // least allocation, may be vector 
            api::Buffer* lastbuf = nullptr;
            api::BufferView* lbv = nullptr;
            api::BufferCreateInfo bfc = {};

        public: 
             Buffer(const std::shared_ptr<Device>& device, api::Buffer* lastbuf = nullptr, api::BufferCreateInfo bfc = api::BufferCreateInfo().setSharingMode(api::SharingMode::eExclusive)) : lastbuf(lastbuf), bfc(bfc), device(device) {};
            ~Buffer(){}; // Here will notification about free memory

            // Get original Vulkan link 
            api::Buffer& Least() { return *lastbuf; };
            operator api::Buffer&() { return *lastbuf; };
            const api::Buffer& Least() const { return *lastbuf; };
            operator const api::Buffer&() const { return *lastbuf; };

            // Editable BufferCreateInfo
            api::BufferCreateInfo& GetCreateInfo() { return bfc; };
            const api::BufferCreateInfo& GetCreateInfo() const { return bfc; };

            // Mapping from Allocator
            uint8_t* GetMapped(){
                return allocation->GetMapped();
            };

            //  
            std::shared_ptr<Buffer>&& QueueFamilyIndices(const std::vector<uint32_t>& indices = {}) {
                bfc.queueFamilyIndexCount = indices.size();
                bfc.pQueueFamilyIndices = indices.data();
                return shared_from_this(); };

            // Link Editable Buffer 
            std::shared_ptr<Buffer>&& Link(api::Buffer* buf) { lastbuf = buf; 
                return shared_from_this(); };

            // 
            std::shared_ptr<Buffer>&& Allocate(const std::shared_ptr<Allocator>& mem, const uintptr_t& ptx = 0u) {
                mem->AllocateForBuffer(lastbuf,allocation=mem->CreateAllocation(),bfc,ptx); 
                return shared_from_this(); };

            // 
            std::shared_ptr<Buffer>&& Create() { // 
                *lastbuf = device->Least().createBuffer(bfc);
                return shared_from_this(); };

            // Create With Buffer View 
            std::shared_ptr<Buffer>&& CreateView(api::BufferView* bfv, const api::Format& format, const uintptr_t& offset = 0u, const size_t& size = 16u) {
                (*bfv = allocation->GetDevice()->Least().createBufferView(api::BufferViewCreateInfo{{}, *lastbuf, format, offset, size})); lbv = bfv;
                return shared_from_this(); };

            // Create With Region
            // TODO: another format 
            std::shared_ptr<BufferRegion<uint8_t>>&& CreateRegion(api::DescriptorBufferInfo* reg, const uintptr_t& offset = 0u, const size_t& size = 16u);
    };



    // TODO: unify functionality 

    template<class T = uint8_t>
    class BufferRegion {
    public:
        BufferRegion(const std::shared_ptr<Buffer>& buffer, api::DescriptorBufferInfo* bufInfo, const api::DeviceSize& size = 0ull, const api::DeviceSize& offset = 0u) : buffer(buffer) {
            bufInfo->buffer = (api::Buffer)(*buffer);
            bufInfo->offset = offset;
            bufInfo->range = size * sizeof(T);
            //this->map();
        };

        T* const& map() { mapped = (T*)((uint8_t*)buffer->GetMapped() + bufInfo->offset); return mapped; };
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

    // defer implement 
    // TODO: another format of BufferRegion
    inline std::shared_ptr<BufferRegion<uint8_t>>&& Buffer::CreateRegion(api::DescriptorBufferInfo* reg, const uintptr_t& offset, const size_t& size) {
        (*reg = api::DescriptorBufferInfo{*lastbuf, offset, size}); //return shared_from_this(); 
        return std::move(std::make_shared<BufferRegion<uint8_t>>(shared_from_this(), reg, offset, size));
    };

    // Wrap as Vector (like STD)
    template<class T = uint8_t>
    class Vector {
    public:
        Vector() {}
        Vector(const std::shared_ptr<Buffer>& buffer, api::DescriptorBufferInfo* bufInfo, const api::DeviceSize& size = 0ull, const api::DeviceSize& offset = 0u) {
            region = std::make_shared<BufferRegion<T>>(buffer, bufInfo, size, offset);
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
