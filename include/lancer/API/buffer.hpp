#pragma once

#include <cstdint>
#include "../lib/core.hpp"
#include "../API/memory.hpp"


namespace lancer {
    // Vookoo-Like 
    class Buffer_T : public std::enable_shared_from_this<Buffer_T> {
        protected: 
            DeviceMaker device = {};
            MemoryAllocation allocation = {}; // least allocation, may be vector 
            api::Buffer* lastbuf = nullptr;
            api::BufferView* lbv = nullptr;
            api::BufferCreateInfo bfc = {};

        public: 
             Buffer_T(const DeviceMaker& device, api::Buffer* lastbuf = nullptr, api::BufferCreateInfo bfc = api::BufferCreateInfo().setSharingMode(api::SharingMode::eExclusive)) : lastbuf(lastbuf), bfc(bfc), device(device) {};
            ~Buffer_T(){}; // Here will notification about free memory

            // Get original Vulkan link 
            inline api::Buffer& least() { return *lastbuf; };
            operator api::Buffer&() { return *lastbuf; };
            inline const api::Buffer& least() const { return *lastbuf; };
            operator const api::Buffer&() const { return *lastbuf; };

            // Editable BufferCreateInfo
            inline api::BufferCreateInfo& getCreateInfo() { return bfc; };
            inline const api::BufferCreateInfo& getCreateInfo() const { return bfc; };

            // Mapping from Allocator
            inline uint8_t* getMapped(){
                return allocation->getMapped();
            };

            //  
            inline BufferMaker&& queueFamilyIndices(const std::vector<uint32_t>& indices = {}) {
                bfc.queueFamilyIndexCount = indices.size();
                bfc.pQueueFamilyIndices = indices.data();
                return shared_from_this(); };

            // Link Editable Buffer 
            inline BufferMaker&& link(api::Buffer* buf = nullptr) { lastbuf = buf; 
                return shared_from_this(); };

            // 
            inline BufferMaker&& allocate(const MemoryAllocator& mem, const uintptr_t& ptx = 0u) {
                mem->allocateForBuffer(lastbuf,allocation=mem->createAllocation(),bfc,ptx); 
                return shared_from_this(); };

            // 
            inline BufferMaker&& create() { // 
                *lastbuf = device->least().createBuffer(bfc);
                return shared_from_this(); };

            // Create With Buffer View 
            inline BufferMaker&& createView(api::BufferView* bfv, const api::Format& format, const uintptr_t& offset = 0u, const size_t& size = 16u) {
                (*bfv = allocation->getDevice()->least().createBufferView(api::BufferViewCreateInfo{{}, *lastbuf, format, offset, size})); lbv = bfv;
                return shared_from_this(); };

            // Create With Region
            // TODO: another format 
            inline BufferRegionU8Maker&& createRegion(api::DescriptorBufferInfo* reg = nullptr, const uintptr_t& offset = 0u, const size_t& size = 16u);
            inline BufferRegionU8Maker&& createRegion(api::DescriptorBufferInfo* reg);
    };



    // TODO: unify functionality 

    template<class T = uint8_t>
    class BufferRegion_T {
        public:
            ~BufferRegion_T<T>(){};
             BufferRegion_T<T>(const Buffer& buffer, api::DescriptorBufferInfo* bufInfo) : buffer(buffer), bufInfo(bufInfo) { bufInfo->buffer = *buffer; };
             BufferRegion_T<T>(const Buffer& buffer = {}, api::DescriptorBufferInfo* bufInfo = nullptr, const api::DeviceSize& size = 16u, const api::DeviceSize& offset = 0u) : buffer(buffer), bufInfo(bufInfo) {
                bufInfo->buffer = *buffer;
                bufInfo->offset = offset;
                bufInfo->range = size * sizeof(T);
            };

            T* const& map() { mapped = (T*)((uint8_t*)buffer->getMapped() + bufInfo->offset); return mapped; };
            T* const& data() { this->map(); return mapped; };
            inline const T*& data() const { return mapped; };

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
            inline const api::DeviceSize& offset() const { return bufInfo->offset; };

        protected:
            T* mapped = {};
            Buffer buffer = {};
            api::DescriptorBufferInfo* bufInfo = {};
    };

    // defer implement 
    // TODO: another format of BufferRegion
    inline BufferRegionU8Maker&& Buffer_T::createRegion(api::DescriptorBufferInfo* reg) { reg->buffer = *lastbuf; return std::move(std::make_shared<BufferRegionU8_T>(shared_from_this(), reg)); };
    inline BufferRegionU8Maker&& Buffer_T::createRegion(api::DescriptorBufferInfo* reg, const uintptr_t& offset, const size_t& size) {
        (*reg = api::DescriptorBufferInfo{*lastbuf, offset, size}); //return shared_from_this(); 
        return std::move(std::make_shared<BufferRegionU8_T>(shared_from_this(), reg, offset, size));
    };

    // Wrap as Vector (like STD)
    template<class T = uint8_t>
    class Vector {
        public:
            Vector~() {};
            //Vector () {};
            Vector (const Buffer& buffer = {}, api::DescriptorBufferInfo& bufInfo = nullptr, const api::DeviceSize& size = 0ull, const api::DeviceSize& offset = 0u) { region = std::make_shared<BufferRegion_T<T>>(buffer, bufInfo, size, offset); };
            Vector (const Buffer& buffer     , api::DescriptorBufferInfo& bufInfo) { region = std::make_shared<BufferRegion_T<T>>(buffer, bufInfo); };
            Vector (const Vector<T>& vector) : region(vector.region) {};
            Vector (const std::shared_ptr<BufferRegion_T<T>>& region) : region(region) {};

            // map through
            T* const& map() { return region->map(); };
            inline void unmap() { return region->unmap(); };

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
            inline const api::DeviceSize& offset() const { return region->offset(); };

        protected:
            std::shared_ptr<BufferRegion_T<T>> region = {};
    };

};
