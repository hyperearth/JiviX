#include "utils.hpp"
#include "structs.hpp"

namespace vkt {

    class VmaAllocatedBuffer : public std::enable_shared_from_this<VmaAllocatedBuffer> { public:
        VmaAllocatedBuffer();
        VmaAllocatedBuffer(
            const VmaAllocator& allocator = {},
            const vkh::VkBufferCreateInfo& createInfo = {},
            VmaMemoryUsage vmaUsage = VMA_MEMORY_USAGE_GPU_ONLY
        ) {
            VmaAllocationCreateInfo vmaInfo = {};
            vmaInfo.usage = vmaUsage;
            if (vmaUsage == VMA_MEMORY_USAGE_CPU_TO_GPU || vmaUsage == VMA_MEMORY_USAGE_GPU_TO_CPU) {
                vmaInfo.flags = VMA_ALLOCATION_CREATE_MAPPED_BIT;
            };

            vmaCreateBuffer(this->allocator = allocator, (VkBufferCreateInfo*)&createInfo, &vmaInfo, (VkBuffer*)&bufInfo.buffer, &allocation, &allocationInfo);
        };

        ~VmaAllocatedBuffer() {
            vmaDestroyBuffer(allocator, *this, allocation);
        };

        // Get mapped memory
        void* map() { void* ptr = nullptr; vmaMapMemory(allocator, allocation, &ptr); return ptr; };
        void* mapped() { if (!allocationInfo.pMappedData) { vmaMapMemory(allocator, allocation, &allocationInfo.pMappedData); }; return allocationInfo.pMappedData; };

        // GPU unmap memory
        void unmap() { vmaUnmapMemory(allocator, allocation); allocationInfo.pMappedData = nullptr; };

        // vk::Device caster
        //operator vk::Buffer&() { return buffer; };
        operator const vk::Buffer& () const { return bufInfo.buffer; };
        operator const VkBuffer& () const { return (VkBuffer&)bufInfo.buffer; };

        // Allocation
        //operator VmaAllocation&() { return allocation; };
        operator const VmaAllocation& () const { return allocation; };

        // AllocationInfo
        //operator VmaAllocationInfo&() { return allocationInfo; };
        operator const VmaAllocationInfo& () const { return allocationInfo; };

        // 
        operator const vk::DescriptorBufferInfo& () const { return bufInfo; };

    protected:
        vk::DescriptorBufferInfo bufInfo = {};
        VmaAllocation allocation = {};
        VmaAllocationInfo allocationInfo;
        VmaAllocator allocator = {};
    };

    class VmaAllocatedImage : public std::enable_shared_from_this<VmaAllocatedImage> { public:
        VmaAllocatedImage();
        VmaAllocatedImage(
            const VmaAllocator& allocator = {},
            const vkh::VkImageCreateInfo& createInfo = {},
            VmaMemoryUsage vmaUsage = VMA_MEMORY_USAGE_GPU_ONLY
        ) {
            VmaAllocationCreateInfo vmaInfo = {};
            vmaInfo.usage = vmaUsage;
            if (vmaUsage == VMA_MEMORY_USAGE_CPU_TO_GPU || vmaUsage == VMA_MEMORY_USAGE_GPU_TO_CPU) {
                vmaInfo.flags = VMA_ALLOCATION_CREATE_MAPPED_BIT;
            };

            vmaCreateImage(this->allocator = allocator, (VkImageCreateInfo*)&createInfo, &vmaInfo, (VkImage*)&image, &allocation, &allocationInfo);
        };

        ~VmaAllocatedImage() {
            vmaDestroyImage(allocator, *this, allocation);
        };

        // Get mapped memory
        void* map() { void* ptr = nullptr; vmaMapMemory(allocator, allocation, &ptr); return ptr; };
        void* mapped() { if (!allocationInfo.pMappedData) { vmaMapMemory(allocator, allocation, &allocationInfo.pMappedData); }; return allocationInfo.pMappedData; };

        // GPU unmap memory
        void unmap() { vmaUnmapMemory(allocator, allocation); allocationInfo.pMappedData = nullptr; };

        // vk::Device caster
        //operator vk::Buffer&() { return buffer; };
        operator const vk::Image& () const { return image; };
        operator const VkImage& () const { return (VkImage&)image; };

        // Allocation
        //operator VmaAllocation&() { return allocation; };
        operator const VmaAllocation& () const { return allocation; };

        // AllocationInfo
        //operator VmaAllocationInfo&() { return allocationInfo; };
        operator const VmaAllocationInfo& () const { return allocationInfo; };

        // 
        operator const vk::ImageView& () const { return imageDesc.imageView; };
        operator const vk::DescriptorImageInfo() const { return imageDesc; };

    protected:
        vk::Image image = {};
        vk::DescriptorImageInfo imageDesc = {};
        VmaAllocation allocation = {};
        VmaAllocationInfo allocationInfo = {};
        VmaAllocator allocator = {};
    };

    // 
    template<class T>
    class BufferRegion {
    public:
        BufferRegion(const std::shared_ptr<VmaAllocatedBuffer>& buffer, vk::DeviceSize size = 0ull, vk::DeviceSize offset = 0u) : buffer(buffer) {
            bufInfo.buffer = (vk::Buffer) * buffer;
            bufInfo.offset = offset;
            bufInfo.range = size * sizeof(T);
        };

        T* const& map() { mapped = (T*)((uint8_t*)buffer->map() + bufInfo.offset); return mapped; };
        void unmap() { buffer->unmap(); };

        T* const& data() { this->mapped(); return mapped; };
        const T*& data() const { return mapped; };

        size_t size() const { return size_t(bufInfo.range / sizeof(T)); };
        const vk::DeviceSize& range() const { return bufInfo.range; };

        // at function 
        const T& at(const uintptr_t& i) const { return mapped[i]; };
        T& at(const uintptr_t& i) { return mapped[i]; };

        // array operator 
        const T& operator [] (const uintptr_t& i) const { return at(i); };
        T& operator [] (const uintptr_t& i) { return at(i); };

        // begin ptr
        const T*& begin() const { return data(); };
        T* const& begin() { return data(); };

        // end ptr
        const T*& end() const { return &at(size() - 1ul); };
        T* end() { return &at(size() - 1ul); };

        operator const vk::DescriptorBufferInfo& () const { return bufInfo; };
        operator const vk::Buffer& () const { return *buffer; };
        const vk::DeviceSize& offset() const { return bufInfo.offset; };

    protected:
        T* mapped = {};
        std::shared_ptr<VmaAllocatedBuffer> buffer = {};
        vk::DescriptorBufferInfo bufInfo = {};
    };

    template<class T>
    class Vector {
    public:
        Vector() {}
        Vector(const std::shared_ptr<VmaAllocatedBuffer>& buffer, vk::DeviceSize size = 0ull, vk::DeviceSize offset = 0u) { region = std::make_shared<BufferRegion<T>>(buffer, size, offset); };
        Vector(const std::shared_ptr<BufferRegion<T>>& region) : region(region) {};
        Vector(const Vector<T>& vector) : region(vector.region) {};

        // map through
        T* const& map() { return region->map(); };
        void unmap() { return region->unmap(); };

        T* const& data() { return region->data(); };
        const T*& data() const { return region->data(); };

        // sizing 
        size_t size() const { return region->size(); };
        const vk::DeviceSize& range() const { return region->range(); };

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
        operator const vk::DescriptorBufferInfo& () const { return *region; };
        operator const vk::Buffer& () const { return *region; };
        const vk::DeviceSize& offset() const { return region->offset(); };

        // 
        BufferRegion<T>* operator->() { return &(*region); };
        BufferRegion<T>& operator*() { return *region; };
        const BufferRegion<T>* operator->() const { return &(*region); };
        const BufferRegion<T>& operator*() const { return *region; };

    protected:
        std::shared_ptr<BufferRegion<T>> region = {};
    };

};
