#include "./factory/API/unified/device.hpp"
#include "./factory/API/unified/buffer.hpp"
#include "./factory/API/unified/allocator.hpp"


namespace svt {
    namespace api {
        namespace factory {
#ifdef USE_VULKAN
            svt::core::handle_ref<std::shared_ptr<buffer_t>,core::api::result_t> allocator_t::create_buffer(
                const std::shared_ptr<device_t>& device, 
                const std::vector<uint32_t>& queue_family_indices, 
                const buffer_create_info& create_info, 
                const uintptr_t& info_ptr
            ) {
                vk::BufferCreateInfo vk_info{};
                vk_info.size = create_info.size;
                vk_info.usage = vk::BufferUsageFlagBits(create_info.usage32u);
                vk_info.flags = vk::BufferCreateFlagBits(create_info.flags32u);
                vk_info.sharingMode = vk::SharingMode(create_info.sharing_mode);
                vk_info.queueFamilyIndexCount = queue_family_indices.size();
                vk_info.pQueueFamilyIndices = queue_family_indices.data();
                
                auto buffer = std::make_shared<buffer_t>();
                buffer->buffer_ = (*device)->createBuffer(vk_info);
                return { buffer, core::api::result_t(0u) };
            };
#endif
        };
    };
};
