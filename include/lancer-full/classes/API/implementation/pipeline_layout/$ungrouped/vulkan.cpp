//#pragma once

#include "./classes/API/types.hpp"
#include "./classes/API/unified/pipeline_layout.hpp"

namespace svt {
    namespace api {
        namespace classes {
#ifdef USE_VULKAN
            svt::core::handle_ref<pipeline_layout,core::api::result_t> pipeline_layout::create(const pipeline_layout_create_info& info = {}) {
                vk::PipelineLayoutCreateInfo vk_info = {};
                vk_info.flags = vk::PipelineLayoutCreateFlags(info.flags);
                vk_info.pPushConstantRanges = (vk::PushConstantRange*)info.push_constant_ranges_.data();
                vk_info.pushConstantRangeCount = info.push_constant_ranges_.size();
                vk_info.pSetLayouts = info.layouts_.data();
                vk_info.setLayoutCount = info.layouts_.size();
                return {*this,core::api::result_t(0u)};
            };
#endif
        };
    };
};
