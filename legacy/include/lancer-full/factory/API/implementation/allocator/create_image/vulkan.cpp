#include "./factory/API/unified/device.hpp"
#include "./factory/API/unified/image.hpp"
#include "./factory/API/unified/allocator.hpp"
//#include "./classes/API/unified/allocator.hpp"

namespace svt {
    namespace api {
        namespace factory {
#ifdef USE_VULKAN
            svt::core::handle_ref<std::shared_ptr<image_t>,core::api::result_t> allocator_t::create_image(
                const std::shared_ptr<device_t>& device, 
                const std::vector<uint32_t>& queue_family_indices, 
                const image_create_info& create_info, 
                const uintptr_t& info_ptr, 
                const image_layout& initial_layout
            ) {
                vk::ImageCreateInfo vk_info{};
                vk_info.usage = vk::ImageUsageFlagBits(create_info.usage_32u);
                vk_info.flags = vk::ImageCreateFlagBits(create_info.flags_32u);
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

                auto image = std::make_shared<image_t>();
                image->image_ = (*device)->createImage(vk_info);
                return { image, core::api::result_t(0u) };
            };
#endif
        };
    };
};
