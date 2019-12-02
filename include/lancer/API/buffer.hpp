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
            api::BufferCreateInfo bfc = {};
            api::Buffer* lastbuf = nullptr;
            api::BufferView* lbv = nullptr;
            bool smartFree = false;

        public: 
             Buffer_T(const DeviceMaker& device = {}, const api::BufferCreateInfo& bfc = api::BufferCreateInfo().setSharingMode(api::SharingMode::eExclusive), api::Buffer* lastbuf = nullptr) : lastbuf(lastbuf), bfc(bfc), device(device) {};
            ~Buffer_T(){
                if (smartFree) { allocation->freeBuffer(shared_from_this()); };
            }; // Here will notification about free memory

            // Get original Vulkan link 
            inline api::Buffer& least() { return *lastbuf; };
            inline const api::Buffer& least() const { return *lastbuf; };
            operator api::Buffer&() { return *lastbuf; };
            operator const api::Buffer&() const { return *lastbuf; };

            // Editable BufferCreateInfo
            inline api::BufferCreateInfo& getCreateInfo() { return bfc; };
            inline const api::BufferCreateInfo& getCreateInfo() const { return bfc; };

            // Memory now can binded 
            inline BufferMaker linkAllocation(const MemoryAllocation& allocation = {}, const vk::BindBufferMemoryInfo& bindinf = {}) {
                if (!!(this->allocation = allocation)) {
                    const auto mem = allocation->getMemory();
                    if (!!mem) { device->least().bindBufferMemory2(vk::BindBufferMemoryInfo(bindinf).setBuffer(*lastbuf).setMemory(mem)); };
                };
                return shared_from_this(); };

            // Mapping from Allocator
            inline uint8_t* getMapped(){
                return allocation->getMapped();
            };

            //  
            inline BufferMaker queueFamilyIndices(const std::vector<uint32_t>& indices = {}) {
                bfc.queueFamilyIndexCount = indices.size();
                bfc.pQueueFamilyIndices = indices.data();
                return shared_from_this(); };

            // Link Editable Buffer 
            inline BufferMaker link(api::Buffer* buf = nullptr) { lastbuf = buf; 
                return shared_from_this(); };

            // 
            inline BufferMaker allocate(const MemoryAllocator& mem, const uintptr_t& ptx = 0u) {
                mem->allocateForBuffer(lastbuf,allocation=mem->createAllocation(),bfc,ptx); 
                return shared_from_this(); };

            // 
            inline BufferMaker allocate(const uintptr_t& ptx = 0u) { return this->allocate(device->getAllocatorPtr(),ptx); };
            inline BufferMaker free() { allocation->smartFree(); smartFree = true; return shared_from_this(); };

            // 
            inline BufferMaker create() { // 
                //*lastbuf = device->least().createBuffer(bfc); // Allocator Will Create Buffer Anyways
                return shared_from_this(); };

            // Create With Buffer View 
            inline BufferMaker createView(api::BufferView* bfv, const api::Format& format, const uintptr_t& offset = 0u, const size_t& size = 16u) {
                (*bfv = allocation->getDevice()->least().createBufferView(api::BufferViewCreateInfo{{}, *lastbuf, format, offset, size})); lbv = bfv;
                return shared_from_this(); };

            // Create With Region
            // TODO: another format 
            inline BufferRegionU8Maker createRegion(api::DescriptorBufferInfo* reg, const uintptr_t& offset = 0u, const size_t& size = VK_WHOLE_SIZE);

            template<class T = uint8_t>
            inline std::shared_ptr<BufferRegion_T<T>> createRegion(api::DescriptorBufferInfo* reg, const uintptr_t& offset = 0u, const size_t& size = VK_WHOLE_SIZE);
    };



    // TODO: unify functionality 

    template<class T>
    class BufferRegion_T {
        public:
            ~BufferRegion_T<T>(){};
             BufferRegion_T<T>(const BufferMaker& buffer, api::DescriptorBufferInfo* bufInfo) : buffer(buffer), bufInfo(bufInfo) { bufInfo->buffer = *buffer; };
             BufferRegion_T<T>(const BufferMaker& buffer, api::DescriptorBufferInfo* bufInfo, const api::DeviceSize& offset, const api::DeviceSize& size = VK_WHOLE_SIZE) : buffer(buffer), bufInfo(bufInfo) {
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
            operator const api::DescriptorBufferInfo&() const { return *bufInfo; };
            operator const api::Buffer&() const { return *buffer; };
            inline const api::DeviceSize& offset() const { return bufInfo->offset; };

        protected:
            T* mapped = {};
            BufferMaker buffer = {};
            api::DescriptorBufferInfo* bufInfo = {};
    };

    // Geometry RTX Required
    template<class T>
    inline std::shared_ptr<BufferRegion_T<T>> Buffer_T::createRegion(api::DescriptorBufferInfo* reg, const uintptr_t& offset, const size_t& size) {
        (*reg = api::DescriptorBufferInfo{ *lastbuf, offset, size * sizeof(T) }); //return shared_from_this(); 
        return std::make_shared<BufferRegion_T<T>>(shared_from_this(), reg, offset, size);
    };

    // defer implement 
    // TODO: another format of BufferRegion
    inline BufferRegionU8Maker Buffer_T::createRegion(api::DescriptorBufferInfo* reg, const uintptr_t& offset, const size_t& size) {
        (*reg = api::DescriptorBufferInfo{ *lastbuf, offset, size }); //return shared_from_this(); 
        return std::make_shared<BufferRegionU8_T>(shared_from_this(), reg, offset, size);
    };

    // Wrap as Vector (like STD)
    template<class T = uint8_t>
    class Vector {
        public:
            ~Vector() {};
             Vector() {};
             Vector(const BufferMaker& buffer, api::DescriptorBufferInfo& bufInfo, const api::DeviceSize& offset = 0u, const api::DeviceSize& size = VK_WHOLE_SIZE) { region = std::make_shared<BufferRegion_T<T>>(buffer, bufInfo, size, offset); };
             Vector(const Vector<T>& vector) : region(vector.region) {};
             Vector(const std::shared_ptr<BufferRegion_T<T>>& region) : region(region) {};

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
