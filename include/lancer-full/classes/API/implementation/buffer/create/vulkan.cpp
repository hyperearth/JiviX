#include "./factory/API/unified/device.hpp"
#include "./factory/API/unified/allocator.hpp"
#include "./classes/API/unified/buffer.hpp"


namespace svt {
    namespace api {
        namespace classes {
#ifdef USE_VULKAN
            svt::core::handle_ref<buffer,core::api::result_t> buffer::create(const allocator& allocator, const buffer_create_info& info) {
                // SHOULD OVERRIDE `std::shared_ptr<data::factory::buffer_t>`
                vk::BufferCreateInfo vk_info{};
                vk_info.size = info.size;
                vk_info.usage = vk::BufferUsageFlagBits(info.usage32u);
                vk_info.flags = vk::BufferCreateFlagBits(info.flags32u);
                vk_info.sharingMode = vk::SharingMode(info.sharing_mode);
                vk_info.queueFamilyIndexCount = device_->queue_family_indices_.size();
                vk_info.pQueueFamilyIndices = device_->queue_family_indices_.data();
                (*buffer_) = (*device_)->createBuffer(vk_info);
                return {*this,core::api::result_t(0u)};
            };
#endif
        };
    };
};
