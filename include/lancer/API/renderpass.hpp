#pragma once

#include "../lib/core.hpp"
#include "../API/memory.hpp"

namespace lancer {

    // Vookoo-Like 
    class RenderPass_T : public std::enable_shared_from_this<RenderPass_T> {
        public:
            RenderPass_T(const Device &device = {}, const api::RenderPassCreateInfo& info = {}, const api::RenderPass* renderpass = nullptr): device(device), renderpass(renderpass), renderPassInfo(info) {  };

            inline api::AttachmentDescription& getAttachmentDescription() { return attachmentDescriptions.back(); };
            inline api::SubpassDescription& getSubpassDescription() { return subpassDescriptions.back(); };
            inline api::SubpassDependency& getSubpassDependency() { return subpassDependencies.back(); };

            inline const api::AttachmentDescription& getAttachmentDescription() const { return attachmentDescriptions.back(); };
            inline const api::SubpassDescription& getSubpassDescription() const { return subpassDescriptions.back(); };
            inline const api::SubpassDependency& getSubpassDependency() const { return subpassDependencies.back(); };

            /// Begin an attachment description.
            /// After this you can call attachment* many times
            inline RenderPassMaker&& attachmentBegin(const api::Format& format) {
                api::AttachmentDescription desc{{}, format};
                s.attachmentDescriptions.push_back(desc);
                return shared_from_this();
            };

/* LEGACY, RESERVED
            inline RenderPass&& Link(api::RenderPass* rps) { renderpass = rps; return shared_from_this(); };
            inline RenderPass&& attachmentFlags(api::AttachmentDescriptionFlags value) { s.attachmentDescriptions.back().flags = value; return shared_from_this(); };
            inline RenderPass&& attachmentFormat(api::Format value) { s.attachmentDescriptions.back().format = value; return shared_from_this(); };
            inline RenderPass&& attachmentSamples(api::SampleCountFlagBits value) { s.attachmentDescriptions.back().samples = value; return shared_from_this(); };
            inline RenderPass&& attachmentLoadOp(api::AttachmentLoadOp value) { s.attachmentDescriptions.back().loadOp = value; return shared_from_this(); };
            inline RenderPass&& attachmentStoreOp(api::AttachmentStoreOp value) { s.attachmentDescriptions.back().storeOp = value; return shared_from_this(); };
            inline RenderPass&& attachmentStencilLoadOp(api::AttachmentLoadOp value) { s.attachmentDescriptions.back().stencilLoadOp = value; return shared_from_this(); };
            inline RenderPass&& attachmentStencilStoreOp(api::AttachmentStoreOp value) { s.attachmentDescriptions.back().stencilStoreOp = value; return shared_from_this(); };
            inline RenderPass&& attachmentInitialLayout(api::ImageLayout value) { s.attachmentDescriptions.back().initialLayout = value; return shared_from_this(); };
            inline RenderPass&& attachmentFinalLayout(api::ImageLayout value) { s.attachmentDescriptions.back().finalLayout = value; return shared_from_this(); };
            inline RenderPass&& dependencySrcSubpass(uint32_t value) { s.subpassDependencies.back().srcSubpass = value; return shared_from_this(); };
            inline RenderPass&& dependencyDstSubpass(uint32_t value) { s.subpassDependencies.back().dstSubpass = value; return shared_from_this(); };
            inline RenderPass&& dependencySrcStageMask(api::PipelineStageFlags value) { s.subpassDependencies.back().srcStageMask = value; return shared_from_this(); };
            inline RenderPass&& dependencyDstStageMask(api::PipelineStageFlags value) { s.subpassDependencies.back().dstStageMask = value; return shared_from_this(); };
            inline RenderPass&& dependencyDependencyFlags(api::DependencyFlags value) { s.subpassDependencies.back().dependencyFlags = value; return shared_from_this(); };
            inline RenderPass&& dependencySrcAccessMask(api::AccessFlags value) { s.subpassDependencies.back().srcAccessMask = value; return shared_from_this(); };
            inline RenderPass&& dependencyDstAccessMask(api::AccessFlags value) { s.subpassDependencies.back().dstAccessMask = value; return shared_from_this(); };
*/

            /// Start a subpass description.
            /// After this you can can call subpassColorAttachment many times
            /// and subpassDepthStencilAttachment once.
            inline RenderPassMaker&& subpassBegin(const api::PipelineBindPoint& bp) {
                api::SubpassDescription desc{};
                desc.pipelineBindPoint = bp;
                s.subpassDescriptions.push_back(desc);
                return shared_from_this();
            };

            inline RenderPassMaker&& subpassColorAttachment(const api::ImageLayout& layout, const uint32_t& attachment) {
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

            inline RenderPassMaker&& subpassDepthStencilAttachment(const api::ImageLayout& layout, uint32_t attachment) {
                api::SubpassDescription &subpass = s.subpassDescriptions.back();
                auto *p = getAttachmentReference();
                p->layout = layout;
                p->attachment = attachment;
                subpass.pDepthStencilAttachment = p;
                return shared_from_this();
            };

            inline RenderPassMaker&&  create() const {
                renderPassInfo.attachmentCount = (uint32_t)s.attachmentDescriptions.size();
                renderPassInfo.pAttachments = s.attachmentDescriptions.data();
                renderPassInfo.subpassCount = (uint32_t)s.subpassDescriptions.size();
                renderPassInfo.pSubpasses = s.subpassDescriptions.data();
                renderPassInfo.dependencyCount = (uint32_t)s.subpassDependencies.size();
                renderPassInfo.pDependencies = s.subpassDependencies.data();
                *renderPass = device.createRenderPass(renderPassInfo);
            };

            inline RenderPassMaker&& dependencyBegin(const uint32_t& srcSubpass, const uint32_t& dstSubpass) {
                api::SubpassDependency desc{};
                desc.srcSubpass = srcSubpass;
                desc.dstSubpass = dstSubpass;
                s.subpassDependencies.push_back(desc);
                return shared_from_this();
            };

        protected: 
            constexpr static int max_refs = 64;
            DeviceMaker device = {};
            api::RenderPass* renderPass = nullptr;
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
