#pragma once

#include "../lib/core.hpp"
#include "../API/memory.hpp"

namespace lancer {
    // Vookoo-Like 

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

    class Pipeline : public std::enable_shared_from_this<Pipeline> {

        protected: 

            std::shared_ptr<Device> device = {};
            api::Pipeline* pipeline = nullptr;
            api::GraphicsPipelineCreateInfo info = {};
            api::PipelineInputAssemblyStateCreateInfo inputAssemblyState_;
            api::Viewport viewport_;
            api::Rect2D scissor_;
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

            Pipeline(const std::shared_ptr<Device>& device, api::Pipeline* pipeline = nullptr, api::GraphicsPipelineCreateInfo info = {}, const uint32_t& width = 1u, const uint32_t& height = 1u) : pipeline(pipeline), info(info) {
                inputAssemblyState_.topology = api::PrimitiveTopology::eTriangleList;
                rasterizationState_.lineWidth = 1.0f;
                viewport_ = api::Viewport{0.0f, 0.0f, (float)width, (float)height, 0.0f, 1.0f};
                scissor_ = api::Rect2D{{0, 0}, {width, height}};

                // Set up depth test, but do not enable it.
                // TODO: make these fields editable
                depthStencilState_.depthTestEnable = VK_FALSE;
                depthStencilState_.depthWriteEnable = VK_TRUE;
                depthStencilState_.depthCompareOp = api::CompareOp::eLessOrEqual;
                depthStencilState_.depthBoundsTestEnable = VK_FALSE;
                depthStencilState_.back.failOp = api::StencilOp::eKeep;
                depthStencilState_.back.passOp = api::StencilOp::eKeep;
                depthStencilState_.back.compareOp = api::CompareOp::eAlways;
                depthStencilState_.stencilTestEnable = VK_FALSE;
                depthStencilState_.front = depthStencilState_.back;

                info.pInputAssemblyState = &inputAssemblyState_;
                info.pRasterizationState = &rasterizationState_;
                info.pDepthStencilState = &depthStencilState_;
                info.pMultisampleState = &multisampleState_;
                info.pVertexInputState = &vertexInputState_;
                info.pColorBlendState = &colorBlendState_;
                info.pViewportState = &(viewportState_ = {{}, 1, &viewport_, 1, &scissor_});
                info.pDynamicState = dynamicState_.empty() ? nullptr : &(dynState_ = {{}, (uint32_t)dynamicState_.size(), dynamicState_.data()});
                info.subpass = subpass_;
            };

            // TODO: add support for editing
            std::shared_ptr<Pipeline>&& Viewport(const api::Viewport &value) { viewport_ = value; return shared_from_this(); };
            std::shared_ptr<Pipeline>&& Scissor(const api::Rect2D &value) { scissor_ = value; return shared_from_this(); };
            std::shared_ptr<Pipeline>&& ColorBlendState(const api::PipelineColorBlendStateCreateInfo  &value) { colorBlendState_ = value; return shared_from_this(); }
            std::shared_ptr<Pipeline>&& DepthStencilState(const api::PipelineDepthStencilStateCreateInfo &value) { depthStencilState_ = value; return shared_from_this(); }
            std::shared_ptr<Pipeline>&& MultisampleState(const api::PipelineMultisampleStateCreateInfo &value) { multisampleState_  = value; return shared_from_this(); }
            std::shared_ptr<Pipeline>&& RasterizationState(const api::PipelineRasterizationStateCreateInfo &value) { rasterizationState_ = value; return shared_from_this(); }
            std::shared_ptr<Pipeline>&& InputAssemblyState(const api::PipelineInputAssemblyStateCreateInfo &value) { inputAssemblyState_ = value; return shared_from_this(); }
            std::shared_ptr<Pipeline>&& DynamicState(api::DynamicState value) { dynamicState_.push_back(value); return shared_from_this(); }

            // 
            std::shared_ptr<Pipeline>&& VertexBinding(uint32_t binding_, uint32_t stride_, const api::VertexInputRate& inputRate_ = api::VertexInputRate::eVertex) {
                vertexBindingDescriptions_.push_back({binding_, stride_, inputRate_});
                return shared_from_this(); };

            std::shared_ptr<Pipeline>&& VertexBinding(const api::VertexInputBindingDescription &desc) {
                vertexBindingDescriptions_.push_back(desc);
                return shared_from_this(); };

            std::shared_ptr<Pipeline>&& VertexAttribute(uint32_t location_, uint32_t binding_, api::Format format_, uint32_t offset_) {
                vertexAttributeDescriptions_.push_back({location_, binding_, format_, offset_});
                return shared_from_this(); };

            std::shared_ptr<Pipeline>&& VertexAttribute(const api::VertexInputAttributeDescription &desc) {
                vertexAttributeDescriptions_.push_back(desc);
                return shared_from_this(); };

            std::shared_ptr<Pipeline>&& DynamicState(const api::DynamicState& state){ // TODO: existence check 
                dynamicState_.push_back(state);
                return shared_from_this(); };

            std::shared_ptr<Pipeline>&& PushBlendState(const api::PipelineColorBlendAttachmentState& state){
                colorBlendAttachments_.push_back(state);
                return shared_from_this(); };

            std::shared_ptr<Pipeline>&& Link(api::Pipeline* pipeline = nullptr) {
                this->pipeline = pipeline;
                return shared_from_this(); };

            std::shared_ptr<Pipeline>&& Create(const api::PipelineCache &pipelineCache, const api::PipelineLayout &pipelineLayout, const api::RenderPass &renderPass, bool defaultBlend=true){

                // Add default colour blend attachment if necessary.
                if (colorBlendAttachments_.empty() && defaultBlend) {
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
                    colorBlendAttachments_.push_back(blend);
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
                *pipeline = device->Least().createGraphicsPipeline(pipelineCache, info);

                // 
                return shared_from_this(); };
    };

};
