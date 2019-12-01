#pragma once

#include "../lib/core.hpp"
#include "../API/device.hpp"


namespace lancer {

    class PipelineLayout_T : public std::enable_shared_from_this<PipelineLayout_T> {

        protected: 
            DeviceMaker device = {};
            api::PipelineLayoutCreateInfo info = {};
            api::PipelineLayout *playout = nullptr;
            std::vector<api::PushConstantRange> pconsts = {};
            std::vector<api::DescriptorSetLayout> pdescls = {};

        public: 
            PipelineLayout_T(const DeviceMaker& device, const api::PipelineLayoutCreateInfo& info = {}, api::PipelineLayout* playout = nullptr) : device(device), playout(playout), info(info) {
            };

            // 
            inline PipelineLayoutMaker link(api::PipelineLayout& lays) { playout = &lays; return shared_from_this(); };
            inline PipelineLayoutMaker pushConstantRange(const api::PushConstantRange& constr = {}) { pconsts.push_back(constr); return shared_from_this(); };
            inline PipelineLayoutMaker pushDescriptorSetLayout(const api::DescriptorSetLayout& descl = {}) { pdescls.push_back(descl); return shared_from_this(); };

            // 
            inline api::PushConstantRange& getConstantRange() { return pconsts.back(); };
            inline api::DescriptorSetLayout& getDescriptorSetLayout() { return pdescls.back(); };
            inline api::PipelineLayoutCreateInfo& getCreateInfo() { return info; };

            // Viewable Current State 
            inline const api::PushConstantRange& getConstantRange() const { return pconsts.back(); };
            inline const api::DescriptorSetLayout& getDescriptorSetLayout() const { return pdescls.back(); };
            inline const api::PipelineLayoutCreateInfo& getCreateInfo() const { return info; };

            // Useful for re-assigment manipulations
            inline std::vector<api::PushConstantRange>& getConstantRangeList() { return pconsts; };
            inline std::vector<api::DescriptorSetLayout>& getDescriptorSetLayoutList() { return pdescls; };
            inline const std::vector<api::PushConstantRange>& getConstantRangeList() const { return pconsts; };
            inline const std::vector<api::DescriptorSetLayout>& getDescriptorSetLayoutList() const { return pdescls; };

            // return pipeline layout reference 
            api::PipelineLayout& least() { return *playout; };
            const api::PipelineLayout& least() const { return *playout; };
            operator api::PipelineLayout&() { return *playout; };
            operator const api::PipelineLayout&() const { return *playout; };

            // 
            inline PipelineLayoutMaker create() {
                info.setLayoutCount = pdescls.size();
                info.pSetLayouts = pdescls.data();
                info.pushConstantRangeCount = pconsts.size();
                info.pPushConstantRanges = pconsts.data();
                *playout = device->least().createPipelineLayout(info);
                return shared_from_this(); };

    };

};
