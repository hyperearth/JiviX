#include "./factory/API/unified/device.hpp"
#include "./factory/API/unified/allocator.hpp"
#include "./classes/API/unified/buffer.hpp"


namespace svt {
    namespace api {
        namespace classes {
#ifdef USE_VULKAN
            svt::core::handle_ref<buffer,core::api::result_t> buffer::create(const create_info& info, const allocator& allocator) {
                // SHOULD OVERRIDE `std::shared_ptr<data::factory::buffer_t>`
                vk::BufferCreateInfo vk_info{};
                vk_info.size = info.size;
                vk_info.usage = vk::BufferUsageFlagBits(info.usage32u);
                vk_info.flags = vk::BufferCreateFlagBits(info.flags32u);
                vk_info.sharingMode = vk::SharingMode(info.sharing_mode);
                vk_info.queueFamilyIndexCount = this->device_t->queueFamilyIndices.size();
                vk_info.pQueueFamilyIndices = this->device_t->queueFamilyIndices.data();
                this->buffer_t->buffer = this->device_t->device.createBuffer(vk_info);
                return {*this,core::api::result_t(0u)};
            };
#endif
        };
    };
};
