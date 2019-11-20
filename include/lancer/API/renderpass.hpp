#pragma once

#include "../lib/core.hpp"
#include "../API/memory.hpp"

namespace lancer {
    
    // Vookoo-Like 
    class RenderpassMaker : public std::enable_shared_from_this<RenderpassMaker> {
        public:
            RenderpassMaker(const std::shared_ptr<Device> &device = {}, const api::RenderPass* renderpass = nullptr): device(device), renderpass(renderpass) {  }

            inline api::AttachmentDescription& GetAttachmentDescription() { return attachmentDescriptions.back(); };
            inline api::SubpassDescription& GetSubpassDescription() { return subpassDescriptions.back(); };
            inline api::SubpassDependency& GetSubpassDependency() { return subpassDependencies.back(); };

            inline const api::AttachmentDescription& GetAttachmentDescription() const { return attachmentDescriptions.back(); };
            inline const api::SubpassDescription& GetSubpassDescription() const { return subpassDescriptions.back(); };
            inline const api::SubpassDependency& GetSubpassDependency() const { return subpassDependencies.back(); };

            /// Begin an attachment description.
            /// After this you can call attachment* many times
            inline std::shared_ptr<RenderpassMaker>&& attachmentBegin(api::Format format) {
                api::AttachmentDescription desc{{}, format};
                s.attachmentDescriptions.push_back(desc);
                return shared_from_this();
            };

            inline std::shared_ptr<RenderpassMaker>&& Link(api::RenderpassMaker* rps) { renderpass = rps; return shared_from_this(); };
            inline std::shared_ptr<RenderpassMaker>&& attachmentFlags(api::AttachmentDescriptionFlags value) { s.attachmentDescriptions.back().flags = value; return shared_from_this(); };
            inline std::shared_ptr<RenderpassMaker>&& attachmentFormat(api::Format value) { s.attachmentDescriptions.back().format = value; return shared_from_this(); };
            inline std::shared_ptr<RenderpassMaker>&& attachmentSamples(api::SampleCountFlagBits value) { s.attachmentDescriptions.back().samples = value; return shared_from_this(); };
            inline std::shared_ptr<RenderpassMaker>&& attachmentLoadOp(api::AttachmentLoadOp value) { s.attachmentDescriptions.back().loadOp = value; return shared_from_this(); };
            inline std::shared_ptr<RenderpassMaker>&& attachmentStoreOp(api::AttachmentStoreOp value) { s.attachmentDescriptions.back().storeOp = value; return shared_from_this(); };
            inline std::shared_ptr<RenderpassMaker>&& attachmentStencilLoadOp(api::AttachmentLoadOp value) { s.attachmentDescriptions.back().stencilLoadOp = value; return shared_from_this(); };
            inline std::shared_ptr<RenderpassMaker>&& attachmentStencilStoreOp(api::AttachmentStoreOp value) { s.attachmentDescriptions.back().stencilStoreOp = value; return shared_from_this(); };
            inline std::shared_ptr<RenderpassMaker>&& attachmentInitialLayout(api::ImageLayout value) { s.attachmentDescriptions.back().initialLayout = value; return shared_from_this(); };
            inline std::shared_ptr<RenderpassMaker>&& attachmentFinalLayout(api::ImageLayout value) { s.attachmentDescriptions.back().finalLayout = value; return shared_from_this(); };

            /// Start a subpass description.
            /// After this you can can call subpassColorAttachment many times
            /// and subpassDepthStencilAttachment once.
            inline std::shared_ptr<RenderpassMaker>&& subpassBegin(api::PipelineBindPoint bp) {
                api::SubpassDescription desc{};
                desc.pipelineBindPoint = bp;
                s.subpassDescriptions.push_back(desc);
                return shared_from_this();
            };

            inline std::shared_ptr<RenderpassMaker>&& subpassColorAttachment(api::ImageLayout layout, uint32_t attachment) {
                api::SubpassDescription &subpass = s.subpassDescriptions.back();
                auto *p = getAttachmentReference();
                p->layout = layout;
                p->attachment = attachment;
                if (subpass.colorAttachmentCount == 0) {
                    subpass.pColorAttachments = p;
                }
                subpass.colorAttachmentCount++;
                return shared_from_this();
            };

            inline std::shared_ptr<RenderpassMaker>&& subpassDepthStencilAttachment(api::ImageLayout layout, uint32_t attachment) {
                api::SubpassDescription &subpass = s.subpassDescriptions.back();
                auto *p = getAttachmentReference();
                p->layout = layout;
                p->attachment = attachment;
                subpass.pDepthStencilAttachment = p;
                return shared_from_this();
            };

            inline std::shared_ptr<RenderpassMaker>&&  create() const {
                renderPassInfo.attachmentCount = (uint32_t)s.attachmentDescriptions.size();
                renderPassInfo.pAttachments = s.attachmentDescriptions.data();
                renderPassInfo.subpassCount = (uint32_t)s.subpassDescriptions.size();
                renderPassInfo.pSubpasses = s.subpassDescriptions.data();
                renderPassInfo.dependencyCount = (uint32_t)s.subpassDependencies.size();
                renderPassInfo.pDependencies = s.subpassDependencies.data();
                *renderPass = device.createRenderPass(renderPassInfo);
            };

            inline std::shared_ptr<RenderpassMaker>&& dependencyBegin(uint32_t srcSubpass, uint32_t dstSubpass) {
                api::SubpassDependency desc{};
                desc.srcSubpass = srcSubpass;
                desc.dstSubpass = dstSubpass;
                s.subpassDependencies.push_back(desc);
                return shared_from_this();
            };

            inline std::shared_ptr<RenderpassMaker>&& dependencySrcSubpass(uint32_t value) { s.subpassDependencies.back().srcSubpass = value; return shared_from_this(); };
            inline std::shared_ptr<RenderpassMaker>&& dependencyDstSubpass(uint32_t value) { s.subpassDependencies.back().dstSubpass = value; return shared_from_this(); };
            inline std::shared_ptr<RenderpassMaker>&& dependencySrcStageMask(api::PipelineStageFlags value) { s.subpassDependencies.back().srcStageMask = value; return shared_from_this(); };
            inline std::shared_ptr<RenderpassMaker>&& dependencyDstStageMask(api::PipelineStageFlags value) { s.subpassDependencies.back().dstStageMask = value; return shared_from_this(); };
            inline std::shared_ptr<RenderpassMaker>&& dependencyDependencyFlags(api::DependencyFlags value) { s.subpassDependencies.back().dependencyFlags = value; return shared_from_this(); };
            inline std::shared_ptr<RenderpassMaker>&& dependencySrcAccessMask(api::AccessFlags value) { s.subpassDependencies.back().srcAccessMask = value; return shared_from_this(); };
            inline std::shared_ptr<RenderpassMaker>&& dependencyDstAccessMask(api::AccessFlags value) { s.subpassDependencies.back().dstAccessMask = value; return shared_from_this(); };
            
        protected: 
            constexpr static int max_refs = 64;
            std::shared_ptr<Device> device = {};
            api::RenderPass* renderPass = {};
            api::RenderPassCreateInfo renderPassInfo = {};

            inline api::AttachmentReference *getAttachmentReference() {
                return (s.num_refs < max_refs) ? &s.attachmentReferences[s.num_refs++] : nullptr;
            };

            struct State {
                std::vector<api::AttachmentDescription> attachmentDescriptions;
                std::vector<api::SubpassDescription> subpassDescriptions;
                std::vector<api::SubpassDependency> subpassDependencies;
                std::array<api::AttachmentReference, max_refs> attachmentReferences;
                int num_refs = 0;
                bool ok_ = false;
            } s;
    };

};
