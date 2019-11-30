#pragma once

#include "../lib/core.hpp"
#include "../API/memory.hpp"

namespace lancer {
    // Vookoo-Like 

    inline auto&& disabledBlendState(){
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

    inline auto&& initialBlendState(const bool& enable = true){
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

    inline auto&& initialDepthStencil(){
        api::PipelineDepthStencilStateCreateInfo depthStencilState_ = {};
        depthStencilState_.depthTestEnable = false;
        depthStencilState_.depthWriteEnable = true;
        depthStencilState_.depthCompareOp = api::CompareOp::eLessOrEqual;
        depthStencilState_.depthBoundsTestEnable = false;
        depthStencilState_.back.failOp = api::StencilOp::eKeep;
        depthStencilState_.back.passOp = api::StencilOp::eKeep;
        depthStencilState_.back.compareOp = api::CompareOp::eAlways;
        depthStencilState_.stencilTestEnable = false;
        depthStencilState_.front = depthStencilState_.back;
        return std::move(depthStencilState_);
    };

    class GraphicsPipeline_T : public std::enable_shared_from_this<GraphicsPipeline_T> {
        
        protected: 
            
            DeviceMaker device = {};
            api::Pipeline* pipeline = nullptr;
            api::RenderPass* renderPass = nullptr;
            api::PipelineLayout* playout = nullptr;
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
            GraphicsPipeline_T(const DeviceMaker& device, const api::GraphicsPipelineCreateInfo& info = {}, api::Pipeline* pipeline = nullptr, const uint32_t& width = 1u, const uint32_t& height = 1u) : pipeline(pipeline), info(info) {
                inputAssemblyState_.topology = api::PrimitiveTopology::eTriangleList;
                rasterizationState_.lineWidth = 1.0f;
                viewport_ = api::Viewport{0.0f, 0.0f, (float)width, (float)height, 0.0f, 1.0f};
                scissor_ = api::Rect2D{{0, 0}, {width, height}};

                // 
                this->info.pInputAssemblyState = &inputAssemblyState_;
                this->info.pRasterizationState = &rasterizationState_;
                this->info.pDepthStencilState = &(depthStencilState_ = initialDepthStencil());
                this->info.pMultisampleState = &multisampleState_;
                this->info.pVertexInputState = &vertexInputState_;
                this->info.pColorBlendState = &colorBlendState_;
                this->info.pViewportState = &(viewportState_ = {{}, 1, &viewport_, 1, &scissor_});
                this->info.pDynamicState = dynamicState_.empty() ? nullptr : &(dynState_ = {{}, (uint32_t)dynamicState_.size(), dynamicState_.data()});
                this->info.subpass = subpass_;
            };
            
            // return pipeline reference 
            api::Pipeline& least() { return *pipeline; };
            const api::Pipeline& least() const { return *pipeline; };
            operator api::Pipeline&() { return *pipeline; };
            operator const api::Pipeline&() const { return *pipeline; };


            // Editable 
            inline auto& getCreateInfo() { return info; };
            inline auto& getViewport() { return  viewport_; };
            inline auto& getScissor() { return  scissor_; };
            inline auto& getColorBlendState() { return colorBlendState_; };
            inline auto& getDepthStencilState() { return depthStencilState_; };
            inline auto& getMultisampleState() { return multisampleState_; };
            inline auto& getRasterizationState() { return rasterizationState_; };
            inline auto& getInputAssemblyState() { return inputAssemblyState_; };
            inline auto& getDynamicStateList() { return dynamicState_; };
            inline auto& getVertexInputState() { return vertexInputState_; };
            inline auto& getVertexBindingDescriptionList() { return vertexBindingDescriptions_; };
            inline auto& getVertexAttributeDescriptionList() { return vertexAttributeDescriptions_; };
            inline auto& getColorBlendAttachmentList() { return colorBlendAttachments_; };
            inline auto& getShaderModuleList() { return modules_; };
            inline auto& getColorBlendAttachment() { return colorBlendAttachments_.back(); };
            inline auto& getVertexBindingDescription() { return vertexBindingDescriptions_.back(); };
            inline auto& getVertexAttributeDescription() { return vertexAttributeDescriptions_.back(); };
            inline auto& getShaderModule() { return modules_.back(); };
            inline auto& getDynamicState() { return dynamicState_.back(); };


            // Viewable 
            inline const auto& getCreateInfo() const { return info; };
            inline const auto& getViewport() const { return viewport_; };
            inline const auto& getScissor() const { return  scissor_; };
            inline const auto& getColorBlendState() const { return colorBlendState_; };
            inline const auto& getDepthStencilState() const { return depthStencilState_; };
            inline const auto& getMultisampleState() const { return multisampleState_; };
            inline const auto& getRasterizationState() const { return rasterizationState_; };
            inline const auto& getInputAssemblyState() const { return inputAssemblyState_; };
            inline const auto& getDynamicStateList() const { return dynamicState_; };
            inline const auto& getVertexInputState() const { return vertexInputState_; };
            inline const auto& getVertexBindingDescriptionList() const { return vertexBindingDescriptions_; };
            inline const auto& getVertexAttributeDescriptionList() const { return vertexAttributeDescriptions_; };
            inline const auto& getColorBlendAttachmentList() const { return colorBlendAttachments_; };
            inline const auto& getShaderModuleList() const { return modules_; };
            inline const auto& getColorBlendAttachment() const { return colorBlendAttachments_.back(); };
            inline const auto& getVertexBindingDescription() const { return vertexBindingDescriptions_.back(); };
            inline const auto& getVertexAttributeDescription() const { return vertexAttributeDescriptions_.back(); };
            inline const auto& getShaderModule() const { return modules_.back(); };
            inline const auto& getDynamicState() const { return dynamicState_.back(); };

            // 
            GraphicsPipelineMaker&& setCreateInfo(const api::GraphicsPipelineCreateInfo &value) { info = value; return shared_from_this(); };
            GraphicsPipelineMaker&& setViewport(const api::Viewport &value) { viewport_ = value; return shared_from_this(); };
            GraphicsPipelineMaker&& setScissor(const api::Rect2D &value) { scissor_ = value; return shared_from_this(); };
            GraphicsPipelineMaker&& setColorBlendState(const api::PipelineColorBlendStateCreateInfo  &value) { colorBlendState_ = value; return shared_from_this(); }
            GraphicsPipelineMaker&& setDepthStencilState(const api::PipelineDepthStencilStateCreateInfo &value) { depthStencilState_ = value; return shared_from_this(); }
            GraphicsPipelineMaker&& setMultisampleState(const api::PipelineMultisampleStateCreateInfo &value) { multisampleState_ = value; return shared_from_this(); }
            GraphicsPipelineMaker&& setRasterizationState(const api::PipelineRasterizationStateCreateInfo &value) { rasterizationState_ = value; return shared_from_this(); }
            GraphicsPipelineMaker&& setInputAssemblyState(const api::PipelineInputAssemblyStateCreateInfo &value) { inputAssemblyState_ = value; return shared_from_this(); }
            GraphicsPipelineMaker&& setDynamicState(const api::DynamicState& value) { dynamicState_.push_back(value); return shared_from_this(); }

            // 
            GraphicsPipelineMaker&& pushVertexBinding(const uint32_t& binding_, const uint32_t& stride_ = 4u, const api::VertexInputRate& inputRate_ = api::VertexInputRate::eVertex) {
                vertexBindingDescriptions_.push_back({binding_, stride_, inputRate_});
                return shared_from_this(); };

            GraphicsPipelineMaker&& pushVertexBinding(const api::VertexInputBindingDescription &desc = {}) {
                vertexBindingDescriptions_.push_back(desc);
                return shared_from_this(); };

            GraphicsPipelineMaker&& pushVertexAttribute(const uint32_t& location_, const uint32_t& binding_ = 0u, api::Format format_ = api::Format::eR32G32B32A32Sfloat, const uint32_t& offset_ = 0u) {
                vertexAttributeDescriptions_.push_back({location_, binding_, format_, offset_});
                return shared_from_this(); };

            GraphicsPipelineMaker&& pushVertexAttribute(const api::VertexInputAttributeDescription &desc = {}) {
                vertexAttributeDescriptions_.push_back(desc);
                return shared_from_this(); };

            GraphicsPipelineMaker&& pushDynamicState(const api::DynamicState& state = {}){ // TODO: existence check 
                dynamicState_.push_back(state);
                return shared_from_this(); };

            GraphicsPipelineMaker&& pushShaderModule(const api::PipelineShaderStageCreateInfo& module = {}){ // TODO: existence check 
                modules_.push_back(module);
                return shared_from_this(); };

            GraphicsPipelineMaker&& pushColorBlendAttachment(const api::PipelineColorBlendAttachmentState& state = initialBlendState()) {
                colorBlendAttachments_.push_back(state);
                return shared_from_this(); };

            // Linking Horse 
            GraphicsPipelineMaker&& link(api::Pipeline* pipeline = nullptr) { this->pipeline = pipeline; return shared_from_this(); };
            GraphicsPipelineMaker&& linkPipelineLayout(api::PipelineLayout* ppal) { this->playout = ppal; return shared_from_this(); };
            GraphicsPipelineMaker&& linkRenderPass(api::RenderPass* rpass) { this->renderPass = rpass; };

            // 
            GraphicsPipelineMaker&& create(bool defaultBlend=true) {

                // Add default colour blend attachment if necessary.
                if (colorBlendAttachments_.empty() && defaultBlend) {
                    colorBlendAttachments_.push_back(disabledBlendState());
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
                //info.dynamicCount = dynamicState_.size();
                info.pDynamicState = dynamicState_.empty() ? nullptr : &(dynState_ = {{}, (uint32_t)dynamicState_.size(), dynamicState_.data()});
                info.pViewportState = &(viewportState_ = {{}, 1, &viewport_, 1, &scissor_});
                info.layout = *playout;
                info.renderPass = *renderPass;
                info.stageCount = (uint32_t)modules_.size();
                info.pStages = modules_.data();
                info.subpass = subpass_;

                // 
                *pipeline = device->least().createGraphicsPipeline(device->getPipelineCache(), info);

                // 
                return shared_from_this(); };
    };
};
