#include "utils.hpp"
#include "structs.hpp"

namespace vkt {

    // 
    class VmaBufferAllocation : public std::enable_shared_from_this<VmaBufferAllocation> { public:
        //~VmaBufferAllocation() { vmaDestroyBuffer(allocator, *this, allocation); };
         VmaBufferAllocation() {};
         VmaBufferAllocation(
            const VmaAllocator& allocator,
            const vkh::VkBufferCreateInfo& createInfo = {},
            VmaMemoryUsage vmaUsage = VMA_MEMORY_USAGE_GPU_ONLY
        ) {
            VmaAllocationCreateInfo vmaInfo = {}; vmaInfo.usage = vmaUsage;
            if (vmaUsage == VMA_MEMORY_USAGE_CPU_TO_GPU || vmaUsage == VMA_MEMORY_USAGE_GPU_TO_CPU) { vmaInfo.flags = VMA_ALLOCATION_CREATE_MAPPED_BIT; };
            assert(vmaCreateBuffer(this->allocator = allocator, createInfo, &vmaInfo, &(VkBuffer&)buffer, &allocation, &allocationInfo) == VK_SUCCESS);
            this->range = createInfo.size;
        };
        VmaBufferAllocation(const VmaBufferAllocation& allocation) : buffer(allocation.buffer), allocation(allocation.allocation), allocationInfo(allocation.allocationInfo), allocator(allocation.allocator), range(allocation.range) {};
        VmaBufferAllocation& operator=(const VmaBufferAllocation& allocation) {
            vmaDestroyBuffer(allocator, *this, allocation); // don't assign into already allocated
            this->buffer = allocation.buffer;
            this->allocation = allocation.allocation;
            this->allocationInfo = allocation.allocationInfo;
            this->allocator = allocation.allocator;
            this->range = allocation.range;
            return *this;
        };

        // Get mapped memory
        void* map() { void* ptr = nullptr; vmaMapMemory(allocator, allocation, &ptr); return ptr; };
        void* mapped() { if (!allocationInfo.pMappedData) { vmaMapMemory(allocator, allocation, &allocationInfo.pMappedData); }; return allocationInfo.pMappedData; };
        void unmap() { vmaUnmapMemory(allocator, allocation); allocationInfo.pMappedData = nullptr; };

        // 
        //vk::Buffer& buffer() { return buffer; };
        //VkBuffer& buffer() { return (VkBuffer&)buffer; };

        // 
        //const vk::Buffer& buffer() const { return buffer; };
        //const VkBuffer& buffer() const { return (VkBuffer&)buffer; };

        // 
        operator vk::Buffer& () { return buffer; };
        operator VkBuffer& () { return (VkBuffer&)buffer; };

        // 
        operator const vk::Buffer& () const { return buffer; };
        operator const VkBuffer& () const { return (VkBuffer&)buffer; };

        // VMA HACK FOR EXTRACT DEVICE
        operator const vk::Device& () const { return (vk::Device&)(allocator->m_hDevice); };
        operator const VkDevice& () const { return allocator->m_hDevice; };

        // Allocation
        operator const VmaAllocation& () const { return allocation; };
        operator const VmaAllocationInfo& () const { return allocationInfo; };

    public: // in-variant 
        vk::Buffer buffer = {};
        vk::DeviceSize range = 0ull;
        //vk::DeviceSize stride = 1ull;
        //
    protected: friend VmaBufferAllocation; // 
        VmaAllocation allocation = {};
        VmaAllocationInfo allocationInfo = {};
        VmaAllocator allocator = {};
    };

    // 
    class VmaImageAllocation : public std::enable_shared_from_this<VmaImageAllocation> { public:
        //~VmaImageAllocation() { vmaDestroyImage(allocator, *this, allocation); };
         VmaImageAllocation() {};
         VmaImageAllocation(
            const VmaAllocator& allocator,
            const vkh::VkImageCreateInfo& createInfo = {},
            VmaMemoryUsage vmaUsage = VMA_MEMORY_USAGE_GPU_ONLY
        ) {
            VmaAllocationCreateInfo vmaInfo = {}; vmaInfo.usage = vmaUsage;
            if (vmaUsage == VMA_MEMORY_USAGE_CPU_TO_GPU || vmaUsage == VMA_MEMORY_USAGE_GPU_TO_CPU) { vmaInfo.flags = VMA_ALLOCATION_CREATE_MAPPED_BIT; };
            assert(vmaCreateImage(this->allocator = allocator, createInfo, &vmaInfo, &(VkImage&)image, &allocation, &allocationInfo) == VK_SUCCESS);
        };
        VmaImageAllocation(const VmaImageAllocation& allocation) : image(allocation.image), allocation(allocation.allocation), allocationInfo(allocation.allocationInfo), allocator(allocation.allocator) {};
        VmaImageAllocation& operator=(const VmaImageAllocation& allocation) {
            vmaDestroyImage(allocator, *this, allocation); // don't assign into already allocated
            this->image = allocation.image;
            this->allocation = allocation.allocation;
            this->allocationInfo = allocation.allocationInfo;
            this->allocator = allocation.allocator;
            return *this;
        };

        // Get mapped memory
        void* map() { void* ptr = nullptr; vmaMapMemory(allocator, allocation, &ptr); return ptr; };
        void* mapped() { if (!allocationInfo.pMappedData) { vmaMapMemory(allocator, allocation, &allocationInfo.pMappedData); }; return allocationInfo.pMappedData; };
        void unmap() { vmaUnmapMemory(allocator, allocation); allocationInfo.pMappedData = nullptr; };

        // VMA HACK FOR EXTRACT DEVICE
        operator const vk::Image& () const { return image; };
        operator const vk::Device& () const { return(vk::Device&)(allocator->m_hDevice); };

        // 
        operator const VkImage& () const { return (VkImage&)image; };
        operator const VkDevice& () const { return allocator->m_hDevice; };

        // 
        operator vk::Image& () { return image; };
        operator vk::Device& () { return(vk::Device&)(allocator->m_hDevice); };

        // 
        operator VkImage& () { return (VkImage&)image; };
        operator VkDevice& () { return allocator->m_hDevice; };

        // Allocation
        operator const VmaAllocation& () const { return allocation; };
        operator const VmaAllocationInfo& () const { return allocationInfo; };

        // 
        operator VmaAllocation& () { return allocation; };
        operator VmaAllocationInfo& () { return allocationInfo; };

    // 
    protected: friend VmaImageAllocation; // 
        vk::Image image = {};
        VmaAllocation allocation = {};
        VmaAllocationInfo allocationInfo = {};
        VmaAllocator allocator = {};
    };

    // 
    class ImageRegion : public std::enable_shared_from_this<ImageRegion> { public: 
        ImageRegion(){};
        ImageRegion(const std::shared_ptr<VmaImageAllocation>& allocation, const vkh::VkImageViewCreateInfo& info = {}, const vk::ImageLayout& layout = vk::ImageLayout::eGeneral) {
            this->allocation = allocation;
            this->imgInfo.imageView = ((vk::Device&)(*allocation)).createImageView(vk::ImageViewCreateInfo(info).setImage(*allocation));
            this->imgInfo.imageLayout = VkImageLayout(layout);
            this->subresourceRange = info.subresourceRange;
        };
        ImageRegion(const ImageRegion& region) { 
            this->allocation = region; 
            this->subresourceRange = (vk::ImageSubresourceRange&)region;
            this->imgInfo = (vk::DescriptorImageInfo&)(region); 
        };
        ImageRegion& operator=(const ImageRegion& region){
            this->allocation = region.allocation;
            this->subresourceRange = region.subresourceRange;
            this->imgInfo = region.imgInfo;
            return *this;
        };

        //vkh::VkImageSubresourceRange& subresourceRange() { return this->subresourceRange; };
        vkh::VkImageSubresourceLayers subresourceLayers(const uint32_t mipLevel =  0u) const { return {
            .aspectMask = this->subresourceRange.aspectMask,
            .mipLevel = this->subresourceRange.baseMipLevel + mipLevel,
            .baseArrayLayer = this->subresourceRange.baseArrayLayer,
            .layerCount = this->subresourceRange.layerCount
        };};

        // set methods for direct control
        ImageRegion& setImageLayout(const VkImageLayout layout = {}) { imgInfo.imageLayout = layout; return *this; };
        ImageRegion& setSampler(const VkSampler& sampler = {}) { imgInfo.sampler = sampler; return *this; };
        ImageRegion& setImageLayout(const vk::ImageLayout layout = {}) { imgInfo.imageLayout = (const VkImageLayout&)layout; return *this; };
        ImageRegion& setSampler(const vk::Sampler& sampler = {}) { imgInfo.sampler = (const VkSampler&)sampler; return *this; };

        // 
        operator std::shared_ptr<VmaImageAllocation>&() { return this->allocation; };
        operator vkh::VkImageSubresourceRange&() { return this->subresourceRange; };
        operator vkh::VkDescriptorImageInfo&() { return this->imgInfo; };
        operator vk::DescriptorImageInfo&() { return this->imgInfo; };
        operator vk::ImageSubresourceRange&() { return this->subresourceRange; };
        operator vk::ImageView&() { return reinterpret_cast<vk::ImageView&>(this->imgInfo.imageView); };
        operator vk::ImageLayout&() { return reinterpret_cast<vk::ImageLayout&>(this->imgInfo.imageLayout); };
        operator vk::Image&() { return *allocation; };
        operator vk::Sampler&() { return (vk::Sampler&)imgInfo.sampler; };
        operator vk::Device&() { return *allocation; };
        operator ::VkDescriptorImageInfo&() { return this->imgInfo; };
        operator ::VkImageSubresourceRange&() { return this->subresourceRange; };
        operator VkImageView&() { return reinterpret_cast<VkImageView&>(this->imgInfo.imageView); };
        operator VkImageLayout&() { return reinterpret_cast<VkImageLayout&>(this->imgInfo.imageLayout); };
        operator VkImage&() { return *allocation; };
        operator VkSampler&() { return (VkSampler&)imgInfo.sampler; };
        operator VkDevice&() { return *allocation; };

        // 
        operator const std::shared_ptr<VmaImageAllocation>&() const { return this->allocation; };
        operator const vkh::VkImageSubresourceRange&() const { return this->subresourceRange; };
        operator const vkh::VkDescriptorImageInfo&() const { return this->imgInfo; };
        operator const vk::DescriptorImageInfo&() const { return this->imgInfo; };
        operator const vk::ImageSubresourceRange&() const { return this->subresourceRange; };
        operator const vk::ImageView&() const { return reinterpret_cast<const vk::ImageView&>(this->imgInfo.imageView); };
        operator const vk::ImageLayout&() const { return reinterpret_cast<const vk::ImageLayout&>(this->imgInfo.imageLayout); };
        operator const vk::Image&() const { return *allocation; };
        operator const vk::Sampler&() const { return (vk::Sampler&)imgInfo.sampler; };
        operator const vk::Device&() const { return *allocation; };
        operator const ::VkDescriptorImageInfo&() const { return this->imgInfo; };
        operator const ::VkImageSubresourceRange&() const { return this->subresourceRange; };
        operator const VkImageView&() const { return reinterpret_cast<const VkImageView&>(this->imgInfo.imageView); };
        operator const VkImageLayout&() const { return reinterpret_cast<const VkImageLayout&>(this->imgInfo.imageLayout); };
        operator const VkImage&() const { return *allocation; };
        operator const VkSampler&() const { return (VkSampler&)imgInfo.sampler; };
        operator const VkDevice&() const { return *allocation; };
        operator const vk::ImageSubresourceLayers() const { return vk::ImageSubresourceLayers{ (vk::ImageAspectFlags&)subresourceRange.aspectMask, subresourceRange.baseMipLevel, subresourceRange.baseArrayLayer, subresourceRange.layerCount }; };

        // 
        VmaImageAllocation* operator->() { return &(*allocation); };
        VmaImageAllocation& operator*() { return (*allocation); };
        const VmaImageAllocation* operator->() const { return &(*allocation); };
        const VmaImageAllocation& operator*() const { return (*allocation); };

    protected: friend VmaImageAllocation; // 
        vkh::VkDescriptorImageInfo imgInfo = {};
        std::shared_ptr<VmaImageAllocation> allocation = {};

    public: // irrevalent sources
        vkh::VkImageSubresourceRange subresourceRange = {};
    };

    // Wrapper Class
    template<class T = uint8_t>
    class Vector { // 
    public:
        Vector() {};
        Vector(const std::shared_ptr<VmaBufferAllocation>& allocation, vk::DeviceSize offset = 0u, vk::DeviceSize size = VK_WHOLE_SIZE) : allocation(allocation), bufInfo({ allocation->buffer,offset,size }) {};

        // 
        template<class Tm = T> Vector(const Vector<Tm>& V) : allocation(V), bufInfo({ V.buffer(), V.offset(), V.range() }) {};
        template<class Tm = T> Vector<T>& operator=(const Vector<Tm>& V) { this->allocation = V, this->bufInfo = vk::DescriptorBufferInfo(V.buffer(), V.offset(), V.range()); return *this; };

        // 
        void unmap() { allocation->unmap(); };
        const T* map() const { return (T*)((uint8_t*)allocation->map()+offset()); };
        T* const map() { return (T*)((uint8_t*)allocation->map()+offset()); };

        // 
        const T* mapped(const uintptr_t& i = 0u) const { return &((T*)((uint8_t*)allocation->mapped()+offset()))[i]; };
        T* const mapped(const uintptr_t& i = 0u) { return &((T*)((uint8_t*)allocation->mapped()+offset()))[i]; };

        // 
        T* const data() { return mapped(); };
        const T* data() const { return mapped(); };

        // 
        vk::BufferView& createBufferView(const vk::Format& format = vk::Format::eUndefined) {
            vkh::VkBufferViewCreateInfo info = {};
            info.buffer = bufInfo.buffer;
            info.offset = bufInfo.offset;
            info.range = bufInfo.range;
            info.format = format; // TODO: AUTO-FORMAT
            return (view = (vk::Device&)(bufInfo.buffer).createBufferView(info));
        };

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
        operator std::shared_ptr<VmaBufferAllocation>& () { return allocation; };
        operator vkh::VkDescriptorBufferInfo& () { bufInfo.buffer = (vk::Buffer&)(*allocation); return reinterpret_cast<vkh::VkDescriptorBufferInfo&>(bufInfo); };
        operator vk::DescriptorBufferInfo& () { bufInfo.buffer = (vk::Buffer&)(*allocation); return bufInfo; };
        operator vk::Buffer& () { return (vk::Buffer&)(bufInfo.buffer = (vk::Buffer&)*allocation); };
        operator vk::Device& () { return *allocation; };
        operator vk::BufferView& () { return view; };
        operator ::VkDescriptorBufferInfo& () { bufInfo.buffer = (vk::Buffer&)(*allocation); return reinterpret_cast<::VkDescriptorBufferInfo&>(bufInfo); };
        operator VkBuffer& () { return (VkBuffer&)(bufInfo.buffer = (vk::Buffer&) * allocation); };
        operator VkDevice& () { return *allocation; };
        operator VkBufferView& () { return view; };

        // 
        operator const std::shared_ptr<VmaBufferAllocation>& () const { return allocation; };
        operator const vkh::VkDescriptorBufferInfo& () const { return reinterpret_cast<const vkh::VkDescriptorBufferInfo&>(bufInfo); };
        operator const vk::DescriptorBufferInfo& () const { return bufInfo; };
        operator const vk::Buffer& () const { return *allocation; };
        operator const vk::Device& () const { return *allocation; };
        operator const vk::BufferView& () const { return view; };
        operator const ::VkDescriptorBufferInfo& () const { return reinterpret_cast<const ::VkDescriptorBufferInfo&>(bufInfo); };
        operator const VkBuffer&() const { return *allocation; };
        operator const VkDevice&() const { return *allocation; };
        operator const VkBufferView& () const { return reinterpret_cast<const VkBufferView&>(view); };

        // 
        vk::DeviceSize& offset() { return this->bufInfo.offset; };
        //vk::DeviceSize& stride() { return this->stride; };

        // 
        const vk::DeviceSize& offset() const { return this->bufInfo.offset; };
        //const vk::DeviceSize& stride() const { return this->stride; };

        // 
        vk::DeviceSize range() const { return (this->bufInfo.range != VK_WHOLE_SIZE ? std::min(this->bufInfo.range, this->allocation->range - this->bufInfo.offset) : (this->allocation->range - this->bufInfo.offset)); };
        vk::DeviceSize size() const { return this->range() / this->stride; };

        // 
        vk::Buffer& buffer() { return reinterpret_cast<vk::Buffer&>(allocation->buffer); };
        //VkBuffer& buffer() { return reinterpret_cast<VkBuffer&>(allocation->buffer); };

        // 
        const vk::Buffer& buffer() const { return reinterpret_cast<vk::Buffer&>(allocation->buffer); };
        //const VkBuffer& buffer() const { return reinterpret_cast<VkBuffer&>(allocation->buffer); };

        // typed casting 
        template<class Tm = T> Vector<Tm>& cast() { return reinterpret_cast<Vector<Tm>&>(*this); };
        template<class Tm = T> const Vector<Tm>& cast() const { return reinterpret_cast<const Vector<Tm>&>(*this); };

        // 
        bool has() const { return allocation ? true : false; };

        // 
        VmaBufferAllocation* operator->() { return &(*allocation); };
        VmaBufferAllocation& operator*() { return (*allocation); };
        const VmaBufferAllocation* operator->() const { return &(*allocation); };
        const VmaBufferAllocation& operator*() const { return (*allocation); };

        //
        protected: friend Vector<T>; // 
        protected: vk::DescriptorBufferInfo bufInfo = {};
        public   : vk::DeviceSize stride = sizeof(T);
        protected: vk::BufferView view = {};
        protected: std::shared_ptr<VmaBufferAllocation> allocation = {};
    };
};
