#include "./factory/API/unified/device.hpp"
#include "./factory/VMA/unified/image.hpp"
#include "./factory/VMA/unified/allocator.hpp"


namespace svt {
    namespace vma {
        namespace factory {
#ifdef USE_VULKAN
            svt::core::handle_ref<std::shared_ptr<svt::api::factory::image_t>,core::api::result_t> allocator_t::create_image(
                const std::shared_ptr<svt::api::factory::device_t>& device, 
                const std::vector<uint32_t>& queue_family_indices, 
                const svt::api::image_create_info& create_info, 
                const uintptr_t& info_ptr, 
                const svt::api::image_layout& initial_layout
            ) {
                vk::ImageCreateInfo vk_info{};
                vk_info.usage = vk::ImageUsageFlagBits(create_info.usage32u);
                vk_info.flags = vk::ImageCreateFlagBits(create_info.flags32u);
                vk_info.sharingMode = vk::SharingMode(create_info.sharing_mode);
                vk_info.imageType = vk::ImageType(create_info.image_type);
                vk_info.initialLayout = vk::ImageLayout(initial_layout);
                vk_info.extent = (vk::Extent3D&)(create_info.extent);
                vk_info.tiling = vk::ImageTiling(create_info.tiling);
                vk_info.format = vk::Format(create_info.format);
                vk_info.arrayLayers = create_info.array_layers;
                vk_info.mipLevels = create_info.mip_levels;
                vk_info.queueFamilyIndexCount = queue_family_indices.size();
                vk_info.pQueueFamilyIndices = queue_family_indices.data();

                auto image = std::make_shared<image_t>(); image->allocation_ = std::make_unique<internal_allocation_t>(); // Create Image With Internal Allocation
                vmaCreateImage(allocator_,(VkImageCreateInfo*)(&vk_info),(VmaAllocationCreateInfo*)info_ptr,(VkImage*)(&image->image_),&image->allocation_->allocation_,&image->allocation_->allocation_info_);
                return { std::dynamic_pointer_cast<svt::api::factory::image_t>(image), core::api::result_t(0u) };
            };
#endif
        };
    };
};
