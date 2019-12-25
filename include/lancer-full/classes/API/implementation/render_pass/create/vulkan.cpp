// 
#include "./core/unified/core.hpp"
#include "./classes/API/unified/device.hpp"

// 
#include "./factory/API/unified/render_pass.hpp"
#include "./classes/API/unified/render_pass.hpp"

namespace svt {
    namespace api {
        namespace classes {
#ifdef USE_VULKAN
            // TODO: KHR_render_pass_2 support
            svt::core::handle_ref<render_pass, core::api::result_t> render_pass::create(const render_pass_create_info& info) {

                // 
                std::vector<vk::SubpassDescription> subpasses = {};
                for (auto& subpass : info.subpasses) {
                    auto vk_subpass = vk::SubpassDescription{};
                    vk_subpass.pColorAttachments = (vk::AttachmentReference*)subpass.color_attachments.data();
                    vk_subpass.colorAttachmentCount = subpass.color_attachments.size();
                    vk_subpass.pDepthStencilAttachment = (vk::AttachmentReference*)&subpass.depth_stencil_attachment;
                    vk_subpass.pInputAttachments = (vk::AttachmentReference*)subpass.input_attachments.data();
                    vk_subpass.inputAttachmentCount = subpass.input_attachments.size();
                    vk_subpass.pResolveAttachments = (vk::AttachmentReference*)subpass.resolve_attachments.data();
                };

                // 
                vk::RenderPassCreateInfo render_pass = {};
                render_pass.subpassCount = subpasses.size();
                render_pass.pSubpasses = subpasses.data();
                render_pass.attachmentCount = info.attachments.size();
                render_pass.pAttachments = (vk::AttachmentDescription*)info.attachments.data();
                render_pass.dependencyCount = info.dependencies.size();
                render_pass.pDependencies = (vk::SubpassDependency*)info.dependencies.data();
                render_pass.flags = vk::RenderPassCreateFlags(info.flags);

                // 
                this->render_pass_ = std::make_shared<api::factory::render_pass_t>((*device_)->createRenderPass(render_pass));
                return { *this, core::api::result_t(0u) };
            };
#endif
        };
    };
};
