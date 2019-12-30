#include "utils.hpp"
#include "structs.hpp"

namespace vkt {

    class VmaAllocatedBuffer : public std::enable_shared_from_this<VmaAllocatedBuffer> { public:
        ~VmaAllocatedBuffer() { vmaDestroyBuffer(allocator, *this, allocation); };
         VmaAllocatedBuffer();
         VmaAllocatedBuffer(
            const VmaAllocator& allocator,
            const vkh::VkBufferCreateInfo& createInfo = {},
            VmaMemoryUsage vmaUsage = VMA_MEMORY_USAGE_GPU_ONLY
        ) {
            VmaAllocationCreateInfo vmaInfo = {}; vmaInfo.usage = vmaUsage;
            if (vmaUsage == VMA_MEMORY_USAGE_CPU_TO_GPU || vmaUsage == VMA_MEMORY_USAGE_GPU_TO_CPU) { vmaInfo.flags = VMA_ALLOCATION_CREATE_MAPPED_BIT; };
            vmaCreateBuffer(this->allocator = allocator, createInfo, &vmaInfo, (VkBuffer*)&buffer, &allocation, &allocationInfo);
        };

        // Get mapped memory
        void* map() { void* ptr = nullptr; vmaMapMemory(allocator, allocation, &ptr); return ptr; };
        void* mapped() { if (!allocationInfo.pMappedData) { vmaMapMemory(allocator, allocation, &allocationInfo.pMappedData); }; return allocationInfo.pMappedData; };
        void unmap() { vmaUnmapMemory(allocator, allocation); allocationInfo.pMappedData = nullptr; };

        // 
        operator vk::Buffer& () { return buffer; };
        operator VkBuffer& () { return (VkBuffer&)buffer; };

        // 
        operator const vk::Buffer& () const { return buffer; };
        operator const VkBuffer& () const { return (VkBuffer&)buffer; };

        // Allocation
        operator const VmaAllocation& () const { return allocation; };
        operator const VmaAllocationInfo& () const { return allocationInfo; };

    protected: // 
        vk::Buffer buffer = {};
        VmaAllocation allocation = {};
        VmaAllocationInfo allocationInfo;
        VmaAllocator allocator = {};
    };

    class VmaAllocatedImage : public std::enable_shared_from_this<VmaAllocatedImage> { public:
        ~VmaAllocatedImage() { vmaDestroyImage(allocator, *this, allocation); };
         VmaAllocatedImage();
         VmaAllocatedImage(
            const VmaAllocator& allocator,
            const vkh::VkImageCreateInfo& createInfo = {},
            VmaMemoryUsage vmaUsage = VMA_MEMORY_USAGE_GPU_ONLY
        ) {
            VmaAllocationCreateInfo vmaInfo = {}; vmaInfo.usage = vmaUsage;
            if (vmaUsage == VMA_MEMORY_USAGE_CPU_TO_GPU || vmaUsage == VMA_MEMORY_USAGE_GPU_TO_CPU) { vmaInfo.flags = VMA_ALLOCATION_CREATE_MAPPED_BIT; };
            vmaCreateImage(this->allocator = allocator, createInfo, &vmaInfo, (VkImage*)&image, &allocation, &allocationInfo);
        };

        // Get mapped memory
        void* map() { void* ptr = nullptr; vmaMapMemory(allocator, allocation, &ptr); return ptr; };
        void* mapped() { if (!allocationInfo.pMappedData) { vmaMapMemory(allocator, allocation, &allocationInfo.pMappedData); }; return allocationInfo.pMappedData; };
        void unmap() { vmaUnmapMemory(allocator, allocation); allocationInfo.pMappedData = nullptr; };

        // 
        operator const vk::Image& () const { return image; };
        operator const VkImage& () const { return (VkImage&)image; };

        // Allocation
        operator const VmaAllocation& () const { return allocation; };
        operator const VmaAllocationInfo& () const { return allocationInfo; };

    protected: // 
        vk::Image image = {};
        VmaAllocation allocation = {};
        VmaAllocationInfo allocationInfo = {};
        VmaAllocator allocator = {};
    };

    // 
    template<class T>
    class BufferRegion : public std::enable_shared_from_this<BufferRegion<T>> {
    public: //using T = uint32_t;
        
        template<class Tm = T> BufferRegion(const std::shared_ptr<BufferRegion<Tm>>& region = {}) : buffer(*region), offset(region->offset()), range(region->range()) {};
        BufferRegion(){};
        BufferRegion(const std::shared_ptr<VmaAllocatedBuffer>& buffer, vk::DeviceSize size = 0ull, vk::DeviceSize offset = 0u) : buffer(buffer) {
            bufInfo.buffer = (vk::Buffer&)(*buffer);
            bufInfo.offset = offset;
            bufInfo.range = size * sizeof(T);
        };

        // re-assign buffer region (with another)
        template<class Tm = T> BufferRegion<Tm>& operator=(const std::shared_ptr<BufferRegion<Tm>>& region) { 
            this->buffer = *region, this->offset = region->offset(), this->range = region->range(); return *this; 
        };

        // 
        void unmap() { buffer->unmap(); };
        const T* map() const { return (T*)((uint8_t*)buffer->map()+offset()); };
        T* const map() { return (T*)((uint8_t*)buffer->map()+offset()); };

        // 
        const T* mapped(const uintptr_t& i = 0u) const { return &((T*)((uint8_t*)buffer->mapped()+offset()))[i]; };
        T* const mapped(const uintptr_t& i = 0u) { return &((T*)((uint8_t*)buffer->mapped()+offset()))[i]; };

        // 
        T* const data() { return mapped(); };
        const T* data() const { return mapped(); };

        // 
        vk::DeviceSize size() const { return bufInfo.range / sizeof(T); };

        // at function 
        const T& at(const uintptr_t& i = 0u) const { return *mapped(i); };
        T& at(const uintptr_t& i = 0u) { return *mapped(i); };

        // array operator 
        const T& operator [] (const uintptr_t& i) const { return at(i); };
        T& operator [] (const uintptr_t& i) { return at(i); };

        // begin ptr
        const T* begin() const { return data(); };
        T* const begin() { return data(); };

        // end ptr
        const T* end() const { return &at(size() - 1ul); };
        T* const end() { return &at(size() - 1ul); };

        // 
        operator vk::DescriptorBufferInfo& () { bufInfo.buffer = reinterpret_cast<vk::Buffer&>(*buffer); return bufInfo; };
        operator vk::Buffer& () { return (bufInfo.buffer = reinterpret_cast<vk::Buffer&>(*buffer)); };

        // 
        operator const vk::DescriptorBufferInfo& () const { return bufInfo; };
        operator const vk::Buffer& () const { return *buffer; };

        // 
        const vk::DeviceSize& offset() const { return bufInfo.offset; };
        const vk::DeviceSize& range() const { return bufInfo.range; };

    protected: friend BufferRegion<T>; // 
        vk::DescriptorBufferInfo bufInfo = {};
        std::shared_ptr<VmaAllocatedBuffer> buffer = {};
    };

    template<class T>
    class Vector {
    public:
        Vector() {}
        Vector(const std::shared_ptr<VmaAllocatedBuffer>& buffer, vk::DeviceSize size = 0ull, vk::DeviceSize offset = 0u) { region = std::make_shared<BufferRegion<T>>(buffer, size, offset); };
        Vector(const std::shared_ptr<BufferRegion<T>>& region) : region(region) {};
        Vector(const Vector<T>& vector) : region(vector.region) {};

        // 
        template<class Tm = T> Vector<T>& operator=(const Vector<Tm>& V) { this->region = V.region; return *this; };
        template<class Tm = T> Vector<T>& operator=(const std::shared_ptr<BufferRegion<Tm>>& region) { this->region = region; return *this; };

        // map through
        T* const map() { return region->map(); };
        void unmap() { return region->unmap(); };

        // 
        T* const data() { return region->data(); };
        const T* data() const { return region->data(); };
        vk::DeviceSize size() const { return region->size(); };

        // at function 
        const T& at(const uintptr_t& i) const { return region->at(i); };
        T& at(const uintptr_t& i) { return region->at(i); };

        // array operator 
        const T& operator [] (const uintptr_t& i) const { at(i); };
        T& operator [] (const uintptr_t& i) { return at(i); };

        // begin ptr
        const T* begin() const { region->begin(); };
        T* const begin() { return region->begin(); };

        // end ptr
        const T* end() const { return region->end(); };
        T* const end() { return region->end(); };

        // 
        operator const vk::DescriptorBufferInfo& () const { return *region; };
        operator const vk::Buffer& () const { return *region; };

        // 
        const vk::DeviceSize& range() const { return region->range(); };
        const vk::DeviceSize& offset() const { return region->offset(); };

        // 
        BufferRegion<T>* operator->() { return &(*region); };
        BufferRegion<T>& operator*() { return *region; };

        // 
        const BufferRegion<T>* operator->() const { return &(*region); };
        const BufferRegion<T>& operator*() const { return *region; };

    protected: friend Vector<T>; // 
        std::shared_ptr<BufferRegion<T>> region = {};
    };

};
