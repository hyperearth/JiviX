#pragma once

#include "../lib/core.hpp"
#include "../API/memory.hpp"

namespace lancer {

    // Vookoo-Like 
    class RenderPass_T : public std::enable_shared_from_this<RenderPass_T> {
        public:
            RenderPass_T(const DeviceMaker &device = {}, const api::RenderPassCreateInfo& info = {}, api::RenderPass* renderPass = nullptr): device(device), renderPass(renderPass), renderPassInfo(info) {  };

            inline api::AttachmentDescription& getAttachmentDescription() { return s.attachmentDescriptions.back(); };
            inline api::SubpassDescription& getSubpassDescription() { return s.subpassDescriptions.back(); };
            inline api::SubpassDependency& getSubpassDependency() { return s.subpassDependencies.back(); };

            inline const api::AttachmentDescription& getAttachmentDescription() const { return s.attachmentDescriptions.back(); };
            inline const api::SubpassDescription& getSubpassDescription() const { return s.subpassDescriptions.back(); };
            inline const api::SubpassDependency& getSubpassDependency() const { return s.subpassDependencies.back(); };

            /// Begin an attachment description.
            /// After this you can call attachment* many times
            inline RenderPassMaker&& addAttachment(const api::Format& format = api::Format::eR8G8B8A8Unorm) {
                api::AttachmentDescription desc{{}, format};
                s.attachmentDescriptions.push_back(desc);
                return shared_from_this();
            };

            inline RenderPassMaker&& link(api::RenderPass* rps) { renderPass = rps; return shared_from_this(); };

            /// Start a subpass description.
            /// After this you can can call subpassColorAttachment many times
            /// and subpassDepthStencilAttachment once.
            inline RenderPassMaker&& addSubpass(const api::PipelineBindPoint& bp) {
                api::SubpassDescription desc{};
                desc.pipelineBindPoint = bp;
                s.subpassDescriptions.push_back(desc);
                return shared_from_this();
            };

            inline RenderPassMaker&& subpassColorAttachment(const uint32_t& attachment = 0u, const api::ImageLayout& layout = api::ImageLayout::eColorAttachmentOptimal) {
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

            inline RenderPassMaker&& subpassDepthStencilAttachment(const uint32_t& attachment = 0u, const api::ImageLayout& layout = api::ImageLayout::eDepthStencilAttachmentOptimal) {
                api::SubpassDescription &subpass = s.subpassDescriptions.back();
                auto *p = getAttachmentReference();
                p->layout = layout;
                p->attachment = attachment;
                subpass.pDepthStencilAttachment = p;
                return shared_from_this();
            };

            inline RenderPassMaker&&  create() {
                renderPassInfo.attachmentCount = (uint32_t)s.attachmentDescriptions.size();
                renderPassInfo.pAttachments = s.attachmentDescriptions.data();
                renderPassInfo.subpassCount = (uint32_t)s.subpassDescriptions.size();
                renderPassInfo.pSubpasses = s.subpassDescriptions.data();
                renderPassInfo.dependencyCount = (uint32_t)s.subpassDependencies.size();
                renderPassInfo.pDependencies = s.subpassDependencies.data();
                *renderPass = device->least().createRenderPass(renderPassInfo);
                return shared_from_this();
            };

            inline RenderPassMaker&& addDependency(const uint32_t& srcSubpass = 0u, const uint32_t& dstSubpass = 0u) {
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
