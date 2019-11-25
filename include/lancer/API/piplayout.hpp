#pragma once

#include "../lib/core.hpp"
#include "../API/device.hpp"


namespace lancer {

    class PipelineLayout_T : public std::enable_shared_from_this<PipelineLayout_T> {

        protected: 
            DeviceMaker device = {};
            api::PipelineLayoutCreateInfo info = {};
            api::PipelineLayout *playout = nullptr;
            std::vector<vk::PushConstantRange> pconsts = {};
            std::vector<vk::DescriptorSetLayout> pdescls = {};

        public: 
            PipelineLayout_T(const DeviceMaker& device, const api::PipelineLayoutCreateInfo& info = {}, api::PipelineLayout* playout = nullptr) : device(device), playout(playout), info(info) {
            };

            // 
            inline PipelineLayoutMaker&& link(api::PipelineLayout& lays) { playout = &lays; return shared_from_this(); };
            inline PipelineLayoutMaker&& pushConstantRange(const vk::PushConstantRange& constr = {}) { pconsts.push_back(constr); return shared_from_this(); };
            inline PipelineLayoutMaker&& pushDescriptorSetLayout(const vk::DescriptorSetLayout& descl = {}) { pdescls.push_back(descl); return shared_from_this(); };

            // 
            inline api::PushConstantRange& getConstantRange() { return pconsts.back(); };
            inline api::DescriptorSetLayout& getDescriptorSetLayout() { return pdescls.back(); };
            inline api::PipelineLayoutCreateInfo& getCreateInfo() { return info; };

            // Viewable Current State 
            inline const api::PushConstantRange& getConstantRange() const { return pconsts.back(); };
            inline const api::DescriptorSetLayout& getDescriptorSetLayout() const { return pdescls.back(); };
            inline const api::PipelineLayoutCreateInfo& getCreateInfo() const { return info; };
            

            // 
            inline PipelineLayoutMaker&& create() {
                info.setLayoutCount = pdescls.size();
                info.pSetLayouts = pdescls.data();
                info.pushConstantRangeCount = pconsts.size();
                info.pPushConstantRanges = pconsts.data();
                *playout = device->least().createPipelineLayout(info);
                return shared_from_this(); };

    };

};
