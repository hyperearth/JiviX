#include "./factory/API/unified/device.hpp"
#include "./factory/API/unified/allocator.hpp"
#include "./classes/API/unified/image.hpp"

namespace svt {
    namespace api {
        namespace classes {
#ifdef USE_VULKAN
            svt::core::handle_ref<image, core::api::result_t> image::create(const allocator& allocator, const create_info& info, const image_layout& initial_layout) {
                vk::ImageCreateInfo vk_info{};
                vk_info.usage = vk::ImageUsageFlagBits(info.usage32u);
                vk_info.flags = vk::ImageCreateFlagBits(info.flags32u);
                vk_info.sharingMode = vk::SharingMode(info.sharing_mode);
                vk_info.imageType = vk::ImageType(info.image_type);
                vk_info.initialLayout = vk::ImageLayout(initial_layout);
                vk_info.extent = (vk::Extent3D&)(info.extent);
                vk_info.tiling = vk::ImageTiling(info.tiling);
                vk_info.format = vk::Format(info.format);
                vk_info.arrayLayers = info.array_layers;
                vk_info.mipLevels = info.mip_levels;
                vk_info.queueFamilyIndexCount = this->device_t->queueFamilyIndices.size();
                vk_info.pQueueFamilyIndices = this->device_t->queueFamilyIndices.data();
                this->image_t->image = this->device_t->device.createImage(vk_info);
                return { *this,core::api::result_t(0u) };
            };
#endif
        };
    };
};
