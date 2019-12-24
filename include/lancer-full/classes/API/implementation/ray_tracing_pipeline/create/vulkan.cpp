// 
#include "./classes/API/unified/device.hpp"

// 
#include "./factory/API/unified/ray_tracing_pipeline.hpp"
#include "./classes/API/unified/ray_tracing_pipeline.hpp"

namespace svt {
    namespace api {
        namespace classes {
#if defined(USE_VULKAN)
            // TODO: validate 
            svt::core::handle_ref<ray_tracing_pipeline,core::api::result_t> ray_tracing_pipeline::create(const ray_tracing_pipeline_create_info& info) {
                auto stages = std::vector<vk::PipelineShaderStageCreateInfo>{};
                auto groups = std::vector<vk::RayTracingShaderGroupCreateInfoNV>{};

                for (auto& stage : info.stages) {
                    auto stage_info = vk::PipelineShaderStageCreateInfo{};
                    stage_info.flags = {};
                    stage_info.module = VkShaderModule(stage.module);
                    stage_info.pName = stage.name.c_str();//"main";
                    stage_info.stage = vk::ShaderStageFlagBits(stage.stage_32u);
                    stage_info.pSpecializationInfo = nullptr;
                    stages.push_back(stage_info);
                };

                for (auto& group : info.compiled_shader_groups) {
                    auto group_info = vk::RayTracingShaderGroupCreateInfoNV{};
                    group_info.anyHitShader = group.any_hit_shader;
                    group_info.closestHitShader = group.closest_hit_shader;
                    group_info.intersectionShader = group.intersection_shader;
                    group_info.generalShader = group.general_shader;
                    group_info.type = vk::RayTracingShaderGroupTypeNV(group.type);
                    groups.push_back(group_info);
                };

                auto vk_info = vk::RayTracingPipelineCreateInfoNV{};
                vk_info.pStages = stages.data();
                vk_info.pGroups = groups.data();
                vk_info.stageCount = stages.size();
                vk_info.groupCount = groups.size();
                pipeline_->pipeline_ = vk::Device(*device_).createRayTracingPipelineNV(*device_,vk_info);

                return { *this, core::api::result_t(0u) };
            };
#endif
        };
    };
};
