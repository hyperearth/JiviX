// 
#include "./core/unified/core.hpp"
#include "./classes/API/unified/device.hpp"

// 
#include "./factory/API/unified/framebuffer.hpp"
#include "./classes/API/unified/framebuffer.hpp"

namespace svt {
    namespace api {
        namespace classes {
#ifdef USE_VULKAN
            // TODO: Extensions for Framebuffers
            svt::core::handle_ref<framebuffer, core::api::result_t> framebuffer::create(const framebuffer_create_info& info) {
                vk::FramebufferCreateInfo vk_info = {};
                vk_info.flags = vk::FramebufferCreateFlags(info.flags);
                vk_info.pAttachments = info.attachments.data();
                vk_info.attachmentCount = info.attachments.size();
                vk_info.renderPass = info.render_pass;
                vk_info.width = info.size.x;
                vk_info.height = info.size.y;
                vk_info.layers = info.size.z;

                // 
                this->framebuffer_ = std::make_shared<api::factory::framebuffer_t>((*device_)->createFramebuffer(vk_info));
                return { *this, core::api::result_t(0u) };
            };
#endif
        };
    };
};
