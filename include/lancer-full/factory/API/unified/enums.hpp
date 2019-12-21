#pragma once

#include "./core/unified/core.hpp"
#include "./factory/API/types.hpp"

namespace svt {
    namespace api {

        // 
        enum class sharing_mode : uint32_t {
            t_exclusive = 0u,
            t_concurrent = 1u
        };

        // TODO: more formats
        enum class format : uint32_t {
            t_undefined = 0u,

            t_r16g16g16a16_unorm = 91u,
            t_r16g16g16a16_snorm = 92u,
            t_r16g16g16a16_uscaled = 93u,
            t_r16g16g16a16_sscaled = 94u,

            t_r16g16g16a16_uint = 95u,
            t_r16g16g16a16_sint = 96u,
            t_r16g16g16a16_sfloat = 97u,
            
            t_r32g32g32a32_uint = 107u,
            t_r32g32g32a32_sint = 108u,
            t_r32g32g32a32_sfloat = 109u,
        };
        
        

        struct image_flags { uint32_t
            bit_sparse_binding: 1,
            bit_sparse_residency: 1,
            bit_sparse_aliased: 1,
            bit_mutable_format: 1, 
            bit_cube_compatible: 1,
            bit_2d_array_compatible: 1, 
            bit_block_texel_view_compatible: 1, 
            bit_extended_usage: 1,
            bit_disjoint: 1, 
            bit_alias: 1,
            bit_protected: 1,
            bit_sample_location_compatible_depth: 1,
            bit_corner_sampled: 1,
            bit_subsampled: 1;
        };

        struct image_usage { uint32_t 
            bit_transfer_src : 1,
            bit_transfer_dst : 1,
            bit_sampled : 1,
            bit_storage : 1,
            bit_color_attachment: 1, 
            bit_depth_stencil_attachment: 1, 
            bit_transiend_attachment: 1,
            bit_input_attachment: 1,
            bit_shading_rate: 1,
            bit_fragment_density_map: 1;
        };

        struct sample_count { uint32_t 
            bit_1: 1,
            bit_2: 1,
            bit_4: 1,
            bit_8: 1,
            bit_16: 1,
            bit_32: 1,
            bit_64: 1;
        };

        enum class image_tiling : uint32_t {
            t_optimal = 0u,
            t_linear = 1u,
            t_drm_format_modifier = 1000158000u,
        };

        enum class image_type : uint32_t {
            t_1d = 0u, t_2d, t_3d,
        };

        enum class image_layout : uint32_t {
            t_undefined = 0u,
            t_general = 1u,
            t_color_attachment = 2u,
            t_depth_stencil_attachment = 3u,
            t_depth_stencil_read_only = 4u,
            t_shader_read_only = 5u,
            t_transfer_src = 6u,
            t_transfer_dst = 7u,
            t_preinitialized = 8u,
            t_depth_read_only_stencil_attachment = 1000117000u,
            t_depth_attachment_stencil_read_only = 1000117001u,
            t_present_src = 1000001002u,
            t_shared_present = 1000111000u,
            t_shading_rate_optimal = 1000164003u,
            t_fragment_density_optimal = 1000218000u,
            t_depth_attachment_optimal = 1000241000u,
            t_depth_read_only_optimal = 1000241001u,
            t_stencil_attachment_optimal = 1000241002u,
            t_stencil_read_only_optimal = 1000241003,
        };

        // TODO: extendent bits support
        struct buffer_flags { uint32_t 
            bit_sparse_binding : 1,
            bit_residency_binding : 1,
            bit_sparse_alised : 1,
            bit_protected : 1,
            bit_device_address_capture : 1;
        };

        struct buffer_usage { uint32_t 
            bit_transfer_src : 1,
            bit_transfer_dst : 1,
            bit_uniform_texel_buffer : 1,
            bit_storage_texel_buffer : 1,
            bit_uniform_buffer : 1,
            bit_storage_buffer : 1,
            bit_index_buffer : 1,
            bit_vertex_buffer : 1,
            bit_indirect_buffer : 1,
            bit_conditional_rendering : 1,
            bit_ray_tracing : 1,
            bit_transform_feedback_buffer : 1,
            bit_transform_feedback_counter_buffer : 1,
            bit_shader_device_address : 1;
        };

        struct color_mask { uint32_t 
            r: 1,
            g: 1,
            b: 1,
            a: 1;
        };

        enum class description_type : uint32_t {
            t_sampler = 0u,
            t_combined_image_sampler = 1u,
            t_sampled_image = 2u,
            t_storage_image = 3u,
            t_uniform_texel_buffer = 4u,
            t_storage_texel_buffer = 5u,
            t_uniform_buffer = 6u,
            t_storage_buffer = 7u,
            t_uniform_buffer_dynamic = 8u,
            t_storage_buffer_dynamic = 9u,
            t_input_attachment = 10u,
            t_inline_uniform_block = 1000138000u,
            t_acceleration_structure = 1000165000u
        };

        
        // structs by C++20


        // TODO: GLM types and compatibles
        struct extent_2d { uint32_t width = 1u, height = 1u; };
        struct extent_3d { uint32_t width = 1u, height = 1u, depth = 1u; };
        struct offset_2d { int32_t x = 0u, x = 0u; };
        struct offset_3d { int32_t x = 0u, y = 0u, z = 0u; };

        // 
        struct description_entry {
            description_type type = description_type::t_sampler;
            uint32_t dst_binding = 0u;
            uint32_t dst_item_id = 0u;
            uint32_t descs_count = 1u;
        };

        // TODO: typing control, add into `.cpp` file
        struct description_handle {
            description_entry* entry_t = nullptr;
            void* field_t = nullptr;

            // any buffers and images can `write` into types
            template<class T = uint8_t> operator T&() { return (*field_t); };
            template<class T = uint8_t> operator const T&() const { return (*field_t); };
            template<class T = uint8_t> T& offset(const uint32_t& idx = 0u) { return cpu_handle{entry_t+idx,(T*)field_t}; };
            template<class T = uint8_t> const T& offset(const uint32_t& idx = 0u) const { return cpu_handle{entry_t+idx,(T*)field_t}; };
            const uint32_t& size() const { return entry_t->descs_count; }; 
        };

        struct description_binding {
            uint32_t binding = 0u;

            // TODO: resolve header ordering conflicts
            description_type type = description_type::t_sampler;

            // 
            uint32_t count = 1u;

            // TODO: shader stage flags type
            uint32_t shader_stages = 0b00000000000000000000000000000000;

            // samplers
            core::api::sampler_t* samplers = nullptr;
            
            // TODO: flags EXT type support
            uint32_t flags_ext = 0b00000000000000000000000000000000;

            // 
            //std::vector<sampler_t> vSamplers = {};
        };

        struct buffer_create_info {
            union { uint32_t flags_32u = 0b00000000000000000000000000000000; buffer_flags flags; };
            size_t size = 4u;
            union { uint32_t usage_32u = 0b00000000000000000000000000000000; buffer_usage usage; };
            sharing_mode sharing_mode = sharing_mode::t_exclusive;
        };

        struct image_create_info {
            union { uint32_t flags_32u = 0b00000000000000000000000000000000; image_flags flags; };
            image_type image_type = image_type::t_1d;
            format format{ 0u };
            union { glm::uvec3 extent; extent_3d extent_32u; };
            uint32_t mip_levels = 1u;
            uint32_t array_layers = 1u;
            union { uint32_t samples_32u = 0b00000000000000000000000000000001; sample_count samples; };
            image_tiling tiling = image_tiling::t_optimal;
            union { uint32_t usage_32u = 0b00000000000000000000000000000000; image_usage usage; };
            sharing_mode sharing_mode = sharing_mode::t_exclusive;
            //image_layout initial_layout = image_layout::t_undefined;
        };


        // TODO: Complete Pipeline Support
        class pipeline_shader_stage { public: 
            
        };

        // TODO: more blend factors (with EXT)
        enum class blend_op : uint32_t {
            t_add = 0u,
            t_sub = 1u,
            t_rev_sub = 2u,
            t_min = 3u,
            t_max = 4u
        };

        enum class logic_op : uint32_t {
            t_clear = 0u,
            t_and = 1u,
            t_and_rev = 2u,
            t_copy = 3u,
            t_add_inv = 4u,
            t_no_op = 5u,
            t_xor = 6u,
            t_or = 7u,
            t_nor = 8u,
            t_equivalent = 9u,
            t_inv = 10u,
            t_or_rev = 11u,
            t_copy_inv = 12u,
            t_or_inv = 13u,
            t_nand = 14u, 
            t_set = 15u,
        };

        // blend factors
        enum class blend_factor : uint32_t {
            t_zero = 0u,
            t_one = 1u,
            t_src_color = 2u,
            t_one_minus_src_color = 3u, 
            t_dst_color = 4u,
            t_one_minus_dst_color = 5u, 
            t_src_alpha = 6u,
            t_one_minus_src_alpha = 7u, 
            t_dst_alpha = 8u,
            t_one_minus_dst_alpha = 9u,
            t_const_color = 10u,
            t_one_minus_const_color = 11u,
            t_const_alpha = 12u,
            t_one_minus_const_alpha = 13u,
            t_alpha_saturate = 14u,
            t_src1_color = 15u,
            t_one_minus_src1_color = 16u,
            t_src1_alpha = 17u,
            t_one_minus_src1_alpha = 18u,
        };

        enum class primitive_topology : uint32_t {
            t_point_list = 0u,
            t_line_list = 1u,
            t_line_strip = 2u,
            t_triangle_list = 3u,
            t_triangle_strip = 4u,
            t_triangle_fan = 5u,
            t_line_list_with_adjacency = 6u,
            t_line_strip_with_adjacency = 7u,
            t_triangle_list_adjacency = 8u,
            t_triangle_strip_adjacency = 9u,
            t_patch_list = 10u,
        };

        enum class polygon_mode : uint32_t {
            t_fill = 0u,
            t_line = 1u,
            t_point = 2u,
            t_fill_rectangle = 1000153000u
        };

        enum class front_face : uint32_t {
            t_ccw = 0u,
            t_cw = 1u
        };

        enum class compare_op : uint32_t {
            t_never = 0u,
            t_less = 1u,
            t_equal = 2u,
            t_less_equal = 3u,
            t_greater = 4u,
            t_not_equal = 5u,
            t_greater_or_equal = 6u,
            t_always = 7u
        };

        // TODO: dynamic states
        enum class dynamic_state : uint32_t {
            viewport = 0u,
        };

        enum class stencil_op : uint32_t {
            t_keep = 0u,
            t_zero = 1u,
            t_replace = 2u,
            t_increment_and_clamp = 3u,
            t_decrement_and_clamp = 4u,
            t_invert = 5u,
            t_increment_and_wrap = 6u,
            t_decrement_and_wrap = 7u
        };

        struct stencil_op_state {
            stencil_op fail = stencil_op::t_keep;
            stencil_op pass = stencil_op::t_keep;
            stencil_op depth_fail = stencil_op::t_keep;
            compare_op compare_op = compare_op::t_never;
            uint32_t compare_mask = 0u;
            uint32_t write_mask = 0u;
            uint32_t reference = 0u;
        };

        struct cull_mode { uint32_t 
            bit_front: 1,
            bit_back: 1;
        };


        class blend_state { public: 
            bool enable = false;
            blend_factor src_color_factor = blend_factor::t_one; blend_op color_op = blend_op::t_add; blend_factor dst_color_factor = blend_factor::t_one;
            blend_factor src_alpha_factor = blend_factor::t_one; blend_op alpha_op = blend_op::t_add; blend_factor dst_alpha_factor = blend_factor::t_one;
            union { uint32_t color_write_mask_32u = 0u; color_mask color_write_mask; };
        };

        // TODO:  
        class vertex_binding_desc { public: 
            
        };

        // TODO:  
        class vertex_attribute_desc { public: 
            
        };

        // TODO: 
        class viewport { public: float x = 0.f, y = 0.f, width = 1.f, height = 1.f, min_depth = 0.f, max_depth = 1.f; };
        class rect_2d { public: 
            union { glm::ivec2 offset; offset_2d offset_32i; };
            union { glm::uvec2 extent; extent_2d extent_32u; };
        };

        // TODO: complete pipeline create info
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
                bool primitive_restart = false;
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
                bool depth_clamp = true;
                bool rasterizer_discard = true;
                polygon_mode polygon_mode = polygon_mode::t_fill;
                union { uint32_t cull_mode_32u = 0b00000000000000000000000000000000; cull_mode cull_mode; };
                front_face front_face = front_face::t_ccw;
                bool depth_bias = false;
                float depth_bias_constant_factor;
                float depth_bias_clamp = 0.0001f;
                float depth_bias_clope_factor = 0.f;
                float line_width = 1.f;
            } rasterization_state;

            // 
            struct multisample_state {
                union { uint32_t samples_32u = 0b00000000000000000000000000000001; sample_count samples; };
                bool sample_shading = false;
                float min_sample_shading = 0.f;
                const uint32_t* sample_mask = nullptr;
                bool alpha_to_coverage = false;
                bool alpha_to_one = false;
            } multisample_state;

            // 
            struct depth_stencil_state {
                bool depth_test = false;
                bool depth_write = false;
                uint32_t compare_op = 0u;
                bool depth_bounds_test = false;
                bool stencil_test = false;
                stencil_op_state front = {};
                stencil_op_state back = {};
                float min_depth_bounds = 0.f;
                float max_depth_bounds = 1.f;
            } depth_stencil_state;
            
            // Blend State WIP
            struct color_blend_state {
                bool logic_op_enabled = false;
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

    };
};
