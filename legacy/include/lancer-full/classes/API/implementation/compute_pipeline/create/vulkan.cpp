// 
#include "./classes/API/unified/device.hpp"

// 
#include "./factory/API/unified/compute_pipeline.hpp"
#include "./classes/API/unified/compute_pipeline.hpp"

namespace svt {
    namespace api {
        namespace classes {
#if defined(USE_VULKAN)
            // TODO: validate 
            // TODO: force add subgroup size control support
            svt::core::handle_ref<compute_pipeline,core::api::result_t> compute_pipeline::create(const compute_pipeline_create_info& info) {
                auto vk_info = vk::ComputePipelineCreateInfo{};
                auto stage_info = vk::PipelineShaderStageCreateInfo{};
                stage_info.flags = {};
                stage_info.module = VkShaderModule(info.stage.module);
                stage_info.pName = "main";
                stage_info.stage = vk::ShaderStageFlagBits(info.stage.stage_32u);
                stage_info.pSpecializationInfo = nullptr;

                vk_info.flags = {};
                vk_info.stage = stage_info;
                vk_info.layout = *layout_;
                pipeline_->pipeline_ = vk::Device(*device_).createComputePipeline(*device_,vk_info);
                return { *this, core::api::result_t(0u) };
            };
#endif
        };
    };
};
