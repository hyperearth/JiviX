#pragma once

#include "../lib/core.hpp"
#include "../API/memory.hpp"

namespace lancer {
    // Vookoo-Like 

    auto&& DisabledBlendState(){
        api::PipelineColorBlendAttachmentState blend{};
        blend.blendEnable = 0;
        blend.srcColorBlendFactor = api::BlendFactor::eOne;
        blend.dstColorBlendFactor = api::BlendFactor::eZero;
        blend.colorBlendOp = api::BlendOp::eAdd;
        blend.srcAlphaBlendFactor = api::BlendFactor::eOne;
        blend.dstAlphaBlendFactor = api::BlendFactor::eZero;
        blend.alphaBlendOp = api::BlendOp::eAdd;
        typedef api::ColorComponentFlagBits ccbf;
        blend.colorWriteMask = ccbf::eR|ccbf::eG|ccbf::eB|ccbf::eA;
        return std::move(blend);
    };

    auto&& InitialBlendState(const bool& enable = true){
        api::PipelineColorBlendAttachmentState blend = {};
        blend.blendEnable = enable;
        blend.srcColorBlendFactor = api::BlendFactor::eSrcAlpha;
        blend.dstColorBlendFactor = api::BlendFactor::eOneMinusSrcAlpha;
        blend.colorBlendOp = api::BlendOp::eAdd;
        blend.srcAlphaBlendFactor = api::BlendFactor::eSrcAlpha;
        blend.dstAlphaBlendFactor = api::BlendFactor::eOneMinusSrcAlpha;
        blend.alphaBlendOp = api::BlendOp::eAdd;
        typedef api::ColorComponentFlagBits ccbf;
        blend.colorWriteMask = ccbf::eR|ccbf::eG|ccbf::eB|ccbf::eA;
        return std::move(blend);
    };

    auto&& InitialDepthStencil(){
        api::PipelineDepthStencilStateCreateInfo depthStencilState_ = {};
        depthStencilState_.depthTestEnable = VK_FALSE;
        depthStencilState_.depthWriteEnable = VK_TRUE;
        depthStencilState_.depthCompareOp = api::CompareOp::eLessOrEqual;
        depthStencilState_.depthBoundsTestEnable = VK_FALSE;
        depthStencilState_.back.failOp = api::StencilOp::eKeep;
        depthStencilState_.back.passOp = api::StencilOp::eKeep;
        depthStencilState_.back.compareOp = api::CompareOp::eAlways;
        depthStencilState_.stencilTestEnable = VK_FALSE;
        depthStencilState_.front = depthStencilState_.back;
        return std::move(depthStencilState_);
    };

    class GraphicsPipeline : public std::enable_shared_from_this<GraphicsPipeline> {
        
        protected: 
            
            std::shared_ptr<Device> device = {};
            api::Pipeline* pipeline = nullptr;
            api::GraphicsPipelineCreateInfo info = {};
            api::Viewport viewport_;
            api::Rect2D scissor_;
            api::PipelineInputAssemblyStateCreateInfo inputAssemblyState_;
            api::PipelineRasterizationStateCreateInfo rasterizationState_;
            api::PipelineMultisampleStateCreateInfo multisampleState_;
            api::PipelineDepthStencilStateCreateInfo depthStencilState_;
            api::PipelineColorBlendStateCreateInfo colorBlendState_;
            api::PipelineViewportStateCreateInfo viewportState_;
            api::PipelineVertexInputStateCreateInfo vertexInputState_;
            api::PipelineDynamicStateCreateInfo dynState_;
            std::vector<api::PipelineColorBlendAttachmentState> colorBlendAttachments_;
            std::vector<api::PipelineShaderStageCreateInfo> modules_;
            std::vector<api::VertexInputAttributeDescription> vertexAttributeDescriptions_;
            std::vector<api::VertexInputBindingDescription> vertexBindingDescriptions_;
            std::vector<api::DynamicState> dynamicState_;
            uint32_t subpass_ = 0u;

        public: 

            // 
            GraphicsPipeline(const std::shared_ptr<Device>& device, api::Pipeline* pipeline = nullptr, api::GraphicsPipelineCreateInfo info = {}, const uint32_t& width = 1u, const uint32_t& height = 1u) : pipeline(pipeline), info(info) {
                inputAssemblyState_.topology = api::PrimitiveTopology::eTriangleList;
                rasterizationState_.lineWidth = 1.0f;
                viewport_ = api::Viewport{0.0f, 0.0f, (float)width, (float)height, 0.0f, 1.0f};
                scissor_ = api::Rect2D{{0, 0}, {width, height}};

                // 
                info.pInputAssemblyState = &inputAssemblyState_;
                info.pRasterizationState = &rasterizationState_;
                info.pDepthStencilState = &(depthStencilState_ = InitialDepthStencil());
                info.pMultisampleState = &multisampleState_;
                info.pVertexInputState = &vertexInputState_;
                info.pColorBlendState = &colorBlendState_;
                info.pViewportState = &(viewportState_ = {{}, 1, &viewport_, 1, &scissor_});
                info.pDynamicState = dynamicState_.empty() ? nullptr : &(dynState_ = {{}, (uint32_t)dynamicState_.size(), dynamicState_.data()});
                info.subpass = subpass_;
            };

            // Editable 
            auto& GetPipelineCreateInfo() { return info; };
            auto& GetViewport() { return  viewport_; };
            auto& GetScissor() { return  scissor_; };
            auto& GetColorBlendState() { return colorBlendState_; };
            auto& GetDepthStencilState() { return depthStencilState_; };
            auto& GetMultisampleState() { return multisampleState_; };
            auto& GetRasterizationState() { return rasterizationState_; };
            auto& GetInputAssemblyState() { return inputAssemblyState_; };
            auto& GetDynamicState() { return dynamicState_; };
            auto& GetVertexInputState() { return vertexInputState_; };
            auto& GetVertexBindingDescriptions() { return vertexBindingDescriptions_ };
            auto& GetVertexAttributeDescriptions() { return vertexAttributeDescriptions_ };

            // Viewable 
            const auto& GetPipelineCreateInfo() const { return info; };
            const auto& GetViewport() const { return viewport_; };
            const auto& GetScissor() const { return  scissor_; };
            const auto& GetColorBlendState() const { return colorBlendState_; };
            const auto& GetDepthStencilState() const { return depthStencilState_; };
            const auto& GetMultisampleState() const { return multisampleState_; };
            const auto& GetRasterizationState() const { return rasterizationState_; };
            const auto& GetInputAssemblyState() const { return inputAssemblyState_; };
            const auto& GetDynamicState() const { return dynamicState_; };
            const auto& GetVertexInputState() const { return vertexInputState_; };
            const auto& GetVertexBindingDescriptions() const { return vertexBindingDescriptions_ };
            const auto& GetVertexAttributeDescriptions() const { return vertexAttributeDescriptions_ };

            // 
            std::shared_ptr<GraphicsPipeline>&& SetPipelineCreateInfo(const api::GraphicsPipelineCreateInfo &value) { info = value; return shared_from_this(); };
            std::shared_ptr<GraphicsPipeline>&& SetViewport(const api::Viewport &value) { viewport_ = value; return shared_from_this(); };
            std::shared_ptr<GraphicsPipeline>&& SetScissor(const api::Rect2D &value) { scissor_ = value; return shared_from_this(); };
            std::shared_ptr<GraphicsPipeline>&& SetColorBlendState(const api::PipelineColorBlendStateCreateInfo  &value) { colorBlendState_ = value; return shared_from_this(); }
            std::shared_ptr<GraphicsPipeline>&& SetDepthStencilState(const api::PipelineDepthStencilStateCreateInfo &value) { depthStencilState_ = value; return shared_from_this(); }
            std::shared_ptr<GraphicsPipeline>&& SetMultisampleState(const api::PipelineMultisampleStateCreateInfo &value) { multisampleState_ = value; return shared_from_this(); }
            std::shared_ptr<GraphicsPipeline>&& SetRasterizationState(const api::PipelineRasterizationStateCreateInfo &value) { rasterizationState_ = value; return shared_from_this(); }
            std::shared_ptr<GraphicsPipeline>&& SetInputAssemblyState(const api::PipelineInputAssemblyStateCreateInfo &value) { inputAssemblyState_ = value; return shared_from_this(); }
            std::shared_ptr<GraphicsPipeline>&& SetDynamicState(const api::DynamicState& value) { dynamicState_.push_back(value); return shared_from_this(); }

            // 
            std::shared_ptr<GraphicsPipeline>&& PushVertexBinding(const uint32_t& binding_, const uint32_t& stride_ = 4u, const api::VertexInputRate& inputRate_ = api::VertexInputRate::eVertex) {
                vertexBindingDescriptions_.push_back({binding_, stride_, inputRate_});
                return shared_from_this(); };

            std::shared_ptr<GraphicsPipeline>&& PushVertexBinding(const api::VertexInputBindingDescription &desc = {}) {
                vertexBindingDescriptions_.push_back(desc);
                return shared_from_this(); };

            std::shared_ptr<GraphicsPipeline>&& PushVertexAttribute(const uint32_t& location_, const uint32_t& binding_ = 0u, api::Format format_ = api::Format::eR32G32B32A32Sfloat, const uint32_t& offset_ = 0u) {
                vertexAttributeDescriptions_.push_back({location_, binding_, format_, offset_});
                return shared_from_this(); };

            std::shared_ptr<GraphicsPipeline>&& PushVertexAttribute(const api::VertexInputAttributeDescription &desc = {}) {
                vertexAttributeDescriptions_.push_back(desc);
                return shared_from_this(); };

            std::shared_ptr<GraphicsPipeline>&& PushDynamicState(const api::DynamicState& state = {}){ // TODO: existence check 
                dynamicState_.push_back(state);
                return shared_from_this(); };

            std::shared_ptr<GraphicsPipeline>&& PushColorBlendAttachment(const api::PipelineColorBlendAttachmentState& state = {}) {
                colorBlendAttachments_.push_back(state ? state : InitialBlendState());
                return shared_from_this(); };

            std::shared_ptr<GraphicsPipeline>&& Link(api::Pipeline* pipeline = nullptr) {
                this->pipeline = pipeline;
                return shared_from_this(); };

            // Edit States 
            api::PipelineColorBlendAttachmentState& GetColorBlendAttachment() {
                return colorBlendAttachments_.back(); };

            api::VertexInputBindingDescription& GetVertexBindingDescription() {
                return vertexBindingDescriptions_.back(); };

            api::VertexInputAttributeDescription& GetVertexAttributeDescription() {
                return vertexAttributeDescriptions_.back(); };

            // View States 
            const api::PipelineColorBlendAttachmentState& GetColorBlendAttachment() const {
                return colorBlendAttachments_.back(); };

            const api::VertexInputBindingDescription& GetVertexBindingDescription() const {
                return vertexBindingDescriptions_.back(); };

            const api::VertexInputAttributeDescription& GetVertexAttributeDescription() const {
                return vertexAttributeDescriptions_.back(); };

            // 
            std::shared_ptr<GraphicsPipeline>&& Create(const api::PipelineLayout &pipelineLayout, const api::RenderPass &renderPass, bool defaultBlend=true) {

                // Add default colour blend attachment if necessary.
                if (colorBlendAttachments_.empty() && defaultBlend) {
                    colorBlendAttachments_.push_back(DisabledBlendState());
                };

                // Blend States 
                auto count = (uint32_t)colorBlendAttachments_.size();
                colorBlendState_.attachmentCount = count;
                colorBlendState_.pAttachments = count ? colorBlendAttachments_.data() : nullptr;

                // Vertex Input States Update  
                vertexInputState_.vertexAttributeDescriptionCount = (uint32_t)vertexAttributeDescriptions_.size();
                vertexInputState_.pVertexAttributeDescriptions = vertexAttributeDescriptions_.data();
                vertexInputState_.vertexBindingDescriptionCount = (uint32_t)vertexBindingDescriptions_.size();
                vertexInputState_.pVertexBindingDescriptions = vertexBindingDescriptions_.data();

                // 
                info.pDynamicState = dynamicState_.empty() ? nullptr : &(dynState_ = {{}, (uint32_t)dynamicState_.size(), dynamicState_.data()});
                info.pViewportState = &(viewportState_ = {{}, 1, &viewport_, 1, &scissor_});
                info.layout = pipelineLayout;
                info.renderPass = renderPass;
                info.stageCount = (uint32_t)modules_.size();
                info.pStages = modules_.data();
                info.subpass = subpass_;

                // 
                *pipeline = device->Least().createGraphicsPipeline(device->GetPipelineCache(), info);

                // 
                return shared_from_this(); };
    };
};
