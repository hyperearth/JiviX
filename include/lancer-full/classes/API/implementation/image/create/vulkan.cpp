#include "./static/API/unified/device.hpp"
#include "./dynamic/API/unified/allocator.hpp"
#include "./classes/API/unified/image.hpp"

namespace svt {
    namespace api {
        namespace classes {
#ifdef USE_VULKAN
            svt::core::handle_ref<image,core::result_t> image::create(const create_info& info, const svt::api::allocator& allocator) {
                // SHOULD OVERRIDE `std::shared_ptr<data::dynamic::image_t>`
                vk::ImageCreateInfo vk_info{};
                vk_info.usage = vk::ImageUsageFlagBits(info.usage32u);
                vk_info.flags = vk::ImageCreateFlagBits(info.flags32u);
                vk_info.extent = vk::Extent3D(info.extent);
                vk_info.sharingMode = vk::SharingMode(info.sharing_mode);
                vk_info.imageType = vk::ImageType(info.info_type);
                vk_info.
                this->image_t->image = this->device_t->device.createImage(vk_info);

                return {*this,core::result_t(0u)};
            };
#endif
        };
    };
};
