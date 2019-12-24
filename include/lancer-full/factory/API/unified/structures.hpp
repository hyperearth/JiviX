#pragma once

#include "./core/unified/core.hpp"
#include "./factory/API/unified/bitfields.hpp"
#include "./factory/API/unified/enums.hpp"

namespace svt {
    namespace api {

        // VK Comaptible 
        struct extent_2d { uint32_t width = 1u, height = 1u; };
        struct extent_3d { uint32_t width = 1u, height = 1u, depth = 1u; };
        struct offset_2d { int32_t x = 0u, x = 0u; };
        struct offset_3d { int32_t x = 0u, y = 0u, z = 0u; };

        struct description_binding {
            union { uint32_t flags_ext_32u = 0b00000000000000000000000000000000u; descriptor_binding_flags flags_ext; };

            uint32_t binding = 0u;
            description_type type = description_type::t_sampler;
            uint32_t count = 1u;
            union { uint32_t shader_stages_32u = 0b00000000000000000000000000000000u; shader_stage_flags shader_stages; };
            core::api::sampler_t* samplers = nullptr; // samplers

            
        };

        struct buffer_create_info {
            union { uint32_t flags_32u = 0b00000000000000000000000000000000u; buffer_flags flags; };
            size_t size = 4u;
            union { uint32_t usage_32u = 0b00000000000000000000000000000000u; buffer_usage usage; };
            sharing_mode sharing_mode = sharing_mode::t_exclusive;
        };

        struct image_create_info {
            union { uint32_t flags_32u = 0b00000000000000000000000000000000u; image_flags flags; };
            image_type image_type = image_type::t_1d;
            format format{ 0u };
            union { glm::uvec3 extent; extent_3d extent_32u; };
            uint32_t mip_levels = 1u;
            uint32_t array_layers = 1u;
            union { uint32_t samples_32u = 0b00000000000000000000000000000000u; sample_count samples; };
            image_tiling tiling = image_tiling::t_optimal;
            union { uint32_t usage_32u = 0b00000000000000000000000000000000u; image_usage usage; };
            sharing_mode sharing_mode = sharing_mode::t_exclusive;
            //image_layout initial_layout = image_layout::t_undefined;
        };

        // VK Comaptible 
        struct stencil_op_state {
            stencil_op fail = stencil_op::t_keep;
            stencil_op pass = stencil_op::t_keep;
            stencil_op depth_fail = stencil_op::t_keep;
            compare_op compare_op = compare_op::t_never;
            uint32_t compare_mask = 0u;
            uint32_t write_mask = 0u;
            uint32_t reference = 0u;
        };

        // VK Comaptible 
        class blend_state { public: 
            core::bool32_t enable = false;
            blend_factor src_color_factor = blend_factor::t_one; blend_factor dst_color_factor = blend_factor::t_one; blend_op color_op = blend_op::t_add;
            blend_factor src_alpha_factor = blend_factor::t_one; blend_factor dst_alpha_factor = blend_factor::t_one; blend_op alpha_op = blend_op::t_add; 
            union { uint32_t color_write_mask_32u = 0u; color_mask color_write_mask; };
        };

        // VK Comaptible 
        class vertex_binding_desc { public: 
            uint32_t binding = 0u;
            uint32_t stride = 4u;
            vertex_input_rate input_rate = vertex_input_rate::t_vertex;
        };

        // VK Comaptible 
        class vertex_attribute_desc { public: 
            uint32_t location = 0u;
            uint32_t binding = 0u;
            format format = format::t_r32g32g32a32_sfloat;
            uint32_t offset = 0u;
        };

        // TODO: vierport aggregation
        // VK Comaptible 
        class viewport { public: float x = -1.f, y = -1.f, width = 2.f, height = 2.f, min_depth = 0.f, max_depth = 1.f; };

        // Aggregated Rect 2D
        // VK Comaptible (should to be)
        class rect_2d { public: 
            union { glm::ivec2 offset; offset_2d offset_32i; };
            union { glm::uvec2 extent; extent_2d extent_32u; };
        };

        // TODO: module support
        class pipeline_shader_stage { public: 
            shader_stage_flags stage = {};
            uintptr_t module = 0u;
            std::string name = "";
            uintptr_t specialization = 0u;
        };


        // TODO: complete pipeline create info, extensions
        class graphics_pipeline_create_info { public: uint32_t flags = 0u;
            std::vector<pipeline_shader_stage> stages = {};

            // 
            struct vertex_input_state {
                std::vector<vertex_binding_desc> bindings = {};
                std::vector<vertex_attribute_desc> attributes = {};
            } vertex_input_state;

            // 
            struct input_assembly_state {
                primitive_topology topology = primitive_topology::t_point_list;
                core::bool32_t primitive_restart = false;
            } input_assembly_state;
            
            // 
            struct tesselation_state {
                uint32_t patch_control_points = 1u;
            } tesselation_state;

            // 
            struct viewport_state {
                std::vector<viewport> viewports = {};
                std::vector<rect_2d> scissors = {};
            } viewport_state;

            // 
            struct rasterization_state {
                core::bool32_t depth_clamp = true;
                core::bool32_t rasterizer_discard = true;
                polygon_mode polygon_mode = polygon_mode::t_fill;
                union { uint32_t cull_mode_32u = 0b00000000000000000000000000000000u; cull_mode cull_mode; };
                front_face front_face = front_face::t_ccw;
                core::bool32_t depth_bias = false;
                float depth_bias_constant_factor;
                float depth_bias_clamp = 0.0001f;
                float depth_bias_clope_factor = 0.f;
                float line_width = 1.f;

                // in-line conservative rasterization
                convervative_rasterization_mode convervative_rasterization_mode = convervative_rasterization_mode::t_disabled;
                float extra_primitive_overestimation_size = 0.0001f;
            } rasterization_state;

            // 
            struct multisample_state {
                union { uint32_t samples_32u = 0b00000000000000000000000000000000u; sample_count samples; };
                core::bool32_t sample_shading = false;
                float min_sample_shading = 0.f;
                const uint32_t* sample_mask = nullptr;
                core::bool32_t alpha_to_coverage = false;
                core::bool32_t alpha_to_one = false;

                union { glm::uvec2 sample_location_grid_size; extent_2d sample_location_grid_size_32u; };
                std::vector<glm::vec2> sample_locations = {};
            } multisample_state;

            // 
            struct depth_stencil_state {
                core::bool32_t depth_test = false;
                core::bool32_t depth_write = false;
                uint32_t compare_op = 0u;
                core::bool32_t depth_bounds_test = false;
                core::bool32_t stencil_test = false;
                stencil_op_state front = {};
                stencil_op_state back = {};
                float min_depth_bounds = 0.f;
                float max_depth_bounds = 1.f;
            } depth_stencil_state;
            
            // Blend State WIP
            struct color_blend_state {
                core::bool32_t logic_op_enable = false;
                logic_op logic_op = logic_op::t_clear;
                std::vector<blend_state> attachments = {};
                glm::vec4 constants{0.f,0.f,0.f,0.f};
            } color_blend_state;
            
            // simpler dynamic states
            std::vector<dynamic_state> dynamic_states = {};

            // TODO: pipeline_layout and render_pass types
            uintptr_t pipeline_layout = 0u;
            uintptr_t render_pass = 0u;
            uint32_t subpass = 0u;

            // 
            std::vector<uintptr_t> base_pipeline_handle = {};
        };

        class compute_pipeline_create_info { public: uint32_t flags = 0u;
            pipeline_shader_stage stage = {};
            uintptr_t pipeline_layout = 0u;

            // 
            std::vector<uintptr_t> base_pipeline_handle = {};
        };




        // VK Comaptible 
        struct attachment_description { uint32_t flags = 0u;
            format format = format::t_r16g16g16a16_unorm;
            union { uint32_t samples_32u = 0b00000000000000000000000000000000u; sample_count samples; };
            attachment_load_op load_op = attachment_load_op::t_load;
            attachment_store_op store_op = attachment_store_op::t_store;
            attachment_load_op stencil_load_op = attachment_load_op::t_load;
            attachment_store_op stencil_store_op = attachment_store_op::t_store;
            image_layout initial_layout = image_layout::t_undefined;
            image_layout final_layout = image_layout::t_color_attachment;
        };

        // VK Comaptible 
        struct attachment_reference {
            uint32_t attachment = 0u;
            image_layout layout = image_layout::t_color_attachment;
        };

        // VK Comaptible 
        struct subpass_dependency {
            uint32_t src_subpass = 0u;
            uint32_t dst_subpass = 0u;
            union { uint32_t src_stage_mask_32u = 0b00000000000000000000000000000000u; pipeline_stage_flags src_stage_mask; };
            union { uint32_t dst_stage_mask_32u = 0b00000000000000000000000000000000u; pipeline_stage_flags dst_stage_mask; };
            union { uint32_t src_access_mask_32u = 0b00000000000000000000000000000000u; access_flags src_access_mask; };
            union { uint32_t dst_access_mask_32u = 0b00000000000000000000000000000000u; access_flags dst_access_mask; };
            uint32_t dependency_flags = 0u;
        };

        struct subpass_description {
            std::vector<attachment_reference> input_attachments = {};
            std::vector<attachment_reference> color_attachments = {};
            attachment_reference depth_stencil_attachment = {};
        };

        class render_pass_create_info { public: uint32_t flags = 0u;
            std::vector<attachment_description> attachments = {};
            std::vector<subpass_description> subpasses = {};
            std::vector<subpass_dependency> dependencies = {};
        };




        //class 
        class ray_tracing_shader_group { public: //uint32_t flags = 0u;
            ray_tracing_shader_group_type type = ray_tracing_shader_group_type::t_general;
            uint32_t general_shader = ~0U;
            uint32_t closest_hit_shader = ~0U;
            uint32_t any_hit_shader = ~0U;
            uint32_t intersection_shader = ~0U;
        };

        // constructor with helpers
        class ray_tracing_pipeline_create_info { public: uint32_t flags = 0u;
            std::vector<pipeline_shader_stage> stages = {};
            ray_tracing_shader_group raygen_shader_group = {};
            std::vector<ray_tracing_shader_group> miss_shader_groups = {};
            std::vector<ray_tracing_shader_group> hit_shader_groups = {};
            std::vector<ray_tracing_shader_group> compiled_shader_groups = {};

            // get offsets of shader groups
            uintptr_t raygen_offset_index() { return 0u; };
            uintptr_t miss_offset_index() {return 1u; };
            uintptr_t hit_offset_index() { return miss_shader_groups.size()+1u; };

            // result groups
            std::vector<ray_tracing_shader_group>& compile_groups(){
                compiled_shader_groups = { raygen_shader_group };
                for (auto& group : miss_shader_groups) { compiled_shader_groups.push_back(group); };
                for (auto& group : hit_shader_groups) { compiled_shader_groups.push_back(group); };
                return compiled_shader_groups;
            };
            
            // 
            ray_tracing_pipeline_create_info& add_shader_stages_group(const std::vector<pipeline_shader_stage>& stages_in = {}) {
                for (auto& stage : stages_in) {
                    if (stage.stage.b_raygen) {
                        const uintptr_t last_idx = stages.size(); stages.push_back(stage);
                        raygen_shader_group.general_shader = last_idx;
                    };
                };

                uintptr_t group_idx = -1U;
                for (auto& stage : stages_in) {
                    if (stage.stage.b_miss_hit) {
                        if (group_idx == -1U) { group_idx = miss_shader_groups.size(); miss_shader_groups.push_back({}); };
                        const uintptr_t last_idx = stages.size(); stages.push_back(stage);
                        miss_shader_groups[group_idx].general_shader = last_idx;
                    };
                };

                group_idx = -1U;
                for (auto& stage : stages_in) {
                    if (stage.stage.b_closest_hit | stage.stage.b_any_hit | stage.stage.b_intersection) {
                        if (group_idx == -1U) { group_idx = hit_shader_groups.size(); hit_shader_groups.push_back({}); };
                        const uintptr_t last_idx = stages.size(); stages.push_back(stage);
                        if (stage.stage.b_closest_hit) {
                            hit_shader_groups[group_idx].type = ray_tracing_shader_group_type::t_triangles_hit, 
                            hit_shader_groups[group_idx].closest_hit_shader = last_idx; 
                        };
                        if (stage.stage.b_any_hit) {
                            hit_shader_groups[group_idx].type = ray_tracing_shader_group_type::t_triangles_hit, 
                            hit_shader_groups[group_idx].any_hit_shader = last_idx; 
                        };
                        if (stage.stage.b_intersection) { 
                            hit_shader_groups[group_idx].type = ray_tracing_shader_group_type::t_procedural_hit, 
                            hit_shader_groups[group_idx].intersection_shader = last_idx; 
                        };
                    };
                };

                return *this;
            };
        };


        // VK Comaptible 
        class descriptor_update_template_entry { public: 
            uint32_t binding = 0u;
            uint32_t dst_array_element = 0u;
            uint32_t descriptor_count = 1u;
            description_type descriptor_type = description_type::t_sampler;
            size_t offset = 0u;
            size_t stride = 8u; 
        };

        // TODO: typing control, add into `.cpp` file
        struct description_handle {
            descriptor_update_template_entry* entry_t = nullptr;
            void* field_t = nullptr;

            // any buffers and images can `write` into types
            template<class T = uint8_t> operator T&() { return (*field_t); };
            template<class T = uint8_t> operator const T&() const { return (*field_t); };
            template<class T = uint8_t> T& offset(const uint32_t& idx = 0u) { return description_handle{entry_t+idx,(T*)field_t}; };
            template<class T = uint8_t> const T& offset(const uint32_t& idx = 0u) const { return description_handle{entry_t+idx,(T*)field_t}; };
            const uint32_t& size() const { return entry_t->descriptor_count; }; 
        };

        class descriptor_set_create_info { public: uint32_t flags = 0u; using T = uintptr_t;
            template<class T = T>
            inline description_handle& _push_description( const descriptor_update_template_entry& entry_ ) { // Un-Safe API again
                const uintptr_t pt0 = heap_.size();
                heap_.resize(pt0+sizeof(T)*entry_.descriptor_count, 0u);
                entries_.push_back({
                    .binding = entry_.binding,
                    .dst_array_element = entry_.dst_array_element,
                    .descriptor_count = entry_.descriptor_count,
                    .descriptor_type = entry_.descriptor_type,
                    .offset = pt0,
                    .stride = sizeof(T)
                });
                handles_.push_back({ &entries_.back(), &heap.back() }); return handles_.back();
            };

            // official function (not template)
            description_handle& push_description( const descriptor_update_template_entry& entry_ = {} ) {
                if (entry_.descriptor_type == description_type::t_storage_buffer || entry_.descriptor_type == description_type::t_uniform_buffer) {
                    return _push_description<core::api::buffer_region_t>(entry_);
                } else 
                if (entry_.descriptor_type == description_type::t_storage_texel_buffer || entry_.descriptor_type == description_type::t_uniform_texel_buffer) {
                    return _push_description<core::api::buffer_view_t>(entry_);
                } else 
                if (entry_.descriptor_type == description_type::t_acceleration_structure) {
                    return _push_description<uintptr_t>(entry_); // TODO: Acceleration Structure Support
                } else {
                    return _push_description<core::api::image_desc_t>(entry_);
                };
            };

            // TODO: move into constructive create_info and update_info
            std::vector<uint8_t> heap_ = {};
            std::vector<descriptor_update_template_entry> entries_ = {};
            std::vector<description_handle> handles_ = {};
        };

        // TODO: resolve typing of update or re-create
        using descriptor_set_update_info = descriptor_set_create_info;


        // 
        class descriptor_set_layout_create_info { public: uint32_t flags = 0u;
            std::vector<description_binding> bindings_ = {};
            descriptor_set_layout_create_info& push_binding( const description_binding& binding_ = {} ) {
                bindings_.push_back(binding_); return *this;
            };
        };

    };
};
