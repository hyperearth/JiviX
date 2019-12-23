// 
#include "./classes/API/unified/device.hpp"

// 
#include "./factory/API/unified/graphics_pipeline.hpp"
#include "./classes/API/unified/graphics_pipeline.hpp"

namespace svt {
    namespace api {
        namespace classes {
#if defined(USE_VULKAN)
            // TODO: validate 
            svt::core::handle_ref<graphics_pipeline,core::api::result_t> graphics_pipeline::create(const graphics_pipeline_create_info& info) {
                // 
                std::vector<vk::VertexInputBindingDescription> vertex_bindings = {};
                std::vector<vk::VertexInputAttributeDescription> vertex_attribute = {};
                std::vector<vk::DynamicState> dynamic_states = {};
                std::vector<vk::PipelineColorBlendAttachmentState> blend_states = {};
                std::vector<vk::Rect2D> scissors = {};
                std::vector<vk::Viewport> viewports = {};

                // 
                for (auto &d : info.dynamic_states) { dynamic_states.push_back(vk::DynamicState(d)); };
                for (auto &d : info.vertex_input_state.bindings) { vertex_bindings.push_back({d.binding,d.stride,vk::VertexInputRate(d.input_rate)}); };
                for (auto &d : info.vertex_input_state.attributes) { vertex_attribute.push_back({d.location,d.binding,vk::Format(d.format),d.offset}); };
                for (auto &d : info.color_blend_state.attachments) { blend_states.push_back(vk::PipelineColorBlendAttachmentState{ d.enable, vk::BlendFactor(d.src_color_factor), vk::BlendFactor(d.dst_color_factor), vk::BlendOp(d.color_op), vk::BlendFactor(d.src_alpha_factor), vk::BlendFactor(d.dst_alpha_factor), vk::BlendOp(d.alpha_op), vk::ColorComponentFlags(d.color_write_mask_32u) }); };
                for (auto &d : info.viewport_state.viewports) { viewports.push_back((vk::Viewport&)(d)); };
                for (auto &d : info.viewport_state.scissors) { scissors.push_back({(vk::Offset2D&)(d.offset),(vk::Extent2D&)(d.extent)}); };

                // 
                vk::PipelineVertexInputStateCreateInfo vertex_input = {};
                vk::PipelineInputAssemblyStateCreateInfo input_assembly = {};
                vk::PipelineTessellationStateCreateInfo tesselation_state = {};
                vk::PipelineViewportStateCreateInfo viewport = {};
                vk::PipelineRasterizationStateCreateInfo rasterization = {};
                vk::PipelineMultisampleStateCreateInfo multisample = {};
                vk::PipelineDepthStencilStateCreateInfo depth_stencil = {};
                vk::PipelineColorBlendStateCreateInfo color_blend = {};
                vk::PipelineDynamicStateCreateInfo dynamic = {};
                vk::PipelineRasterizationConservativeStateCreateInfoEXT conservative = {};
                vk::PipelineSampleLocationsStateCreateInfoEXT sample_locations = {};

                // 
                sample_locations.sampleLocationsEnable = info.multisample_state.sample_locations.size() > 0u ? true : false;
                sample_locations.sampleLocationsInfo.pSampleLocations = (vk::SampleLocationEXT*)info.multisample_state.sample_locations.data();
                sample_locations.sampleLocationsInfo.sampleLocationsCount = info.multisample_state.sample_locations.size();
                sample_locations.sampleLocationsInfo.sampleLocationGridSize = (vk::Extent2D&)info.multisample_state.sample_location_grid_size_32u;
                sample_locations.sampleLocationsInfo.sampleLocationsPerPixel = vk::SampleCountFlagBits(info.multisample_state.samples_32u);

                // 
                vertex_input.pVertexAttributeDescriptions = vertex_attribute.data();
                vertex_input.pVertexBindingDescriptions = vertex_bindings.data();
                vertex_input.vertexAttributeDescriptionCount = vertex_attribute.size();
                vertex_input.vertexBindingDescriptionCount = vertex_bindings.size();
                
                // 
                color_blend.pAttachments = blend_states.data();
                color_blend.attachmentCount = blend_states.size();
                color_blend.logicOp = vk::LogicOp(info.color_blend_state.logic_op);
                color_blend.logicOpEnable = info.color_blend_state.logic_op_enable;
                memcpy(color_blend.blendConstants, &info.color_blend_state.constants, sizeof(float)*4u);

                // 
                input_assembly.primitiveRestartEnable = info.input_assembly_state.primitive_restart;
                input_assembly.topology = vk::PrimitiveTopology(info.input_assembly_state.topology);
                
                // 
                tesselation_state.patchControlPoints = info.tesselation_state.patch_control_points;
                
                // 
                viewport.pScissors = scissors.data();
                viewport.scissorCount = scissors.size();
                viewport.pViewports = viewports.data();
                viewport.viewportCount = viewports.size();
                
                // 
                conservative.conservativeRasterizationMode = vk::ConservativeRasterizationModeEXT(info.rasterization_state.convervative_rasterization_mode);
                conservative.extraPrimitiveOverestimationSize = info.rasterization_state.extra_primitive_overestimation_size;

                // 
                

                // 
                rasterization.pNext = &conservative;
                rasterization.cullMode = vk::CullModeFlags(info.rasterization_state.cull_mode_32u);
                rasterization.depthBiasEnable = info.rasterization_state.depth_bias;
                rasterization.depthBiasClamp = info.rasterization_state.depth_bias_clamp;
                rasterization.depthBiasConstantFactor = info.rasterization_state.depth_bias_constant_factor;
                rasterization.depthBiasSlopeFactor = info.rasterization_state.depth_bias_clope_factor;
                rasterization.depthClampEnable = info.rasterization_state.depth_clamp;
                rasterization.frontFace = vk::FrontFace(info.rasterization_state.front_face);
                rasterization.lineWidth = info.rasterization_state.line_width;

                // 
                multisample.alphaToCoverageEnable = info.multisample_state.alpha_to_coverage;
                multisample.alphaToOneEnable = info.multisample_state.alpha_to_one;
                multisample.minSampleShading = info.multisample_state.min_sample_shading;
                multisample.pSampleMask = info.multisample_state.sample_mask;
                multisample.rasterizationSamples = vk::SampleCountFlagBits(info.multisample_state.samples_32u);
                multisample.sampleShadingEnable = info.multisample_state.sample_shading;
                multisample.pNext = &sample_locations;

                // 
                depth_stencil.back = vk::StencilOpState{ vk::StencilOp(info.depth_stencil_state.back.fail), vk::StencilOp(info.depth_stencil_state.back.pass), vk::StencilOp(info.depth_stencil_state.back.depth_fail), vk::CompareOp(info.depth_stencil_state.back.compare_op), info.depth_stencil_state.back.compare_mask, info.depth_stencil_state.back.write_mask,  info.depth_stencil_state.back.reference };
                depth_stencil.front = vk::StencilOpState{ vk::StencilOp(info.depth_stencil_state.front.fail), vk::StencilOp(info.depth_stencil_state.front.pass), vk::StencilOp(info.depth_stencil_state.front.depth_fail), vk::CompareOp(info.depth_stencil_state.front.compare_op), info.depth_stencil_state.front.compare_mask, info.depth_stencil_state.front.write_mask,  info.depth_stencil_state.front.reference };
                depth_stencil.depthBoundsTestEnable = info.depth_stencil_state.depth_bounds_test;
                depth_stencil.depthCompareOp = vk::CompareOp(info.depth_stencil_state.compare_op);
                depth_stencil.depthTestEnable = info.depth_stencil_state.depth_test;
                depth_stencil.depthWriteEnable = info.depth_stencil_state.depth_write;
                depth_stencil.minDepthBounds = info.depth_stencil_state.min_depth_bounds;
                depth_stencil.maxDepthBounds = info.depth_stencil_state.max_depth_bounds;
                
                // 
                dynamic.dynamicStateCount = dynamic_states.size();
                dynamic.pDynamicStates = dynamic_states.data();

                // TODO: Shader Modules Support 
                // TODO: Pipeline and RenderPass Layout Support
                vk::GraphicsPipelineCreateInfo vk_info = {};
                vk_info.layout = VkPipelineLayout(info.pipeline_layout);
                vk_info.renderPass = VkRenderPass(info.render_pass);
                vk_info.subpass = info.subpass;
                vk_info.pVertexInputState = &vertex_input;
                vk_info.pInputAssemblyState = &input_assembly;
                vk_info.pTessellationState = &tesselation_state;
                vk_info.pViewportState = &viewport;
                vk_info.pRasterizationState = &rasterization;
                vk_info.pMultisampleState = &multisample;
                vk_info.pDepthStencilState = &depth_stencil;
                vk_info.pColorBlendState = &color_blend;
                vk_info.pDynamicState = &dynamic;
                
                // TODO: pipeline_cache
                pipeline_->pipeline_ = vk::Device(*device_).createGraphicsPipeline(vk::PipelineCache{},vk_info);

                return { *this, core::api::result_t(0u) };
            };
#endif
        };
    };
};
