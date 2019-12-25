#pragma once

#include "./core/unified/core.hpp"
#include "./factory/API/unified/bitfields.hpp"

namespace svt {
    namespace api {

        // TODO: more formats
        enum class format : uint32_t {
            t_undefined = 0u,

            t_r16_uint = 74u,
            t_r16_sint = 75u,
            t_r16_sfloat = 76u,
            t_r16g16_unorm = 77u,
            t_r16g16_snorm = 78u,
            t_r16g16_uscaled = 79u,
            t_r16g16_sscaled = 80u,
            t_r16g16_uint = 81u,
            t_r16g16_sint = 82u,
            t_r16g16_sfloat = 83u,
            t_r16g16g16_unorm = 84u,
            t_r16g16g16_snorm = 85u,
            t_r16g16g16_uscaled = 86u,
            t_r16g16g16_sscaled = 87u,
            t_r16g16g16_uint = 88u,
            t_r16g16g16_sint = 89u,
            t_r16g16g16_sfloat = 90u,
            t_r16g16g16a16_unorm = 91u,
            t_r16g16g16a16_snorm = 92u,
            t_r16g16g16a16_uscaled = 93u,
            t_r16g16g16a16_sscaled = 94u,
            t_r16g16g16a16_uint = 95u,
            t_r16g16g16a16_sint = 96u,
            t_r16g16g16a16_sfloat = 97u,
            t_r32_uint = 98u,
            t_r32_sint = 99u,
            t_r32_sfloat = 100u,
            t_r32g32_uint = 101u,
            t_r32g32_sint = 102u,
            t_r32g32_sfloat = 103u,
            t_r32g32g32_uint = 104u,
            t_r32g32g32_sint = 105u,
            t_r32g32g32_sfloat = 106u,
            t_r32g32g32a32_uint = 107u,
            t_r32g32g32a32_sint = 108u,
            t_r32g32g32a32_sfloat = 109u,
        };

        // TODO: more blend factors (with EXT)
        enum class blend_op : uint32_t {
            t_add = 0u,
            t_sub = 1u,
            t_rev_sub = 2u,
            t_min = 3u,
            t_max = 4u
        };
        
        // 
        enum class sharing_mode : uint32_t {
            t_exclusive = 0u,
            t_concurrent = 1u
        };

        enum class image_tiling : uint32_t {
            t_optimal = 0u,
            t_linear = 1u,
            t_drm_format_modifier = 1000158000u,
        };

        enum class image_type : uint32_t { t_1d = 0u, t_2d = 1u, t_3d = 2u };
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

        enum class dynamic_state : uint32_t {
            t_viewport = 0u,
            t_scissor = 1u,
            t_line_width = 2u,
            t_depth_bias = 3u,
            t_blend_constants = 4u,
            t_depth_bounds = 5u,
            t_stencil_compare_mask = 6u,
            t_stencil_write_mask = 7u,
            t_stencil_reference = 8u,
            t_viewport_w_scaling = 1000087000u,
            t_discard_rectangle = 1000099000u,
            t_sample_locations = 1000143000u,
            t_viewport_shading_rate_palette = 1000164004u,
            t_viewport_coarse_sample_order = 1000164006u,
            t_exclusive_scissor = 1000205001u,
            t_line_stipple = 1000259000u,
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

        enum class convervative_rasterization_mode : uint32_t {
            t_disabled = 0u,
            t_overstimate = 1u,
            t_understimate = 2u,
        };

        enum class attachment_load_op : uint32_t { t_load = 0u, t_clear = 1u, t_dont_care = 2u };
        enum class attachment_store_op : uint32_t { t_store = 0u, t_dont_care = 1u };

        enum class vertex_input_rate : uint32_t {
            t_vertex = 0u,
            t_instance = 1u,
        };

        enum class ray_tracing_shader_group_type : uint32_t {
            t_general = 0u,
            t_triangles_hit = 1u,
            t_procedural_hit = 2u,
        };

        enum class acceleration_structure_type : uint32_t {
            t_top_level = 0u,
            t_bottom_level = 1u
        };

        enum class geometry_type : uint32_t {
            t_triangles = 0u,
            t_aabbs = 1u
        };

        enum class index_type : uint32_t {
            t_uint16 = 0u,
            t_uint32 = 1u,
            t_none = 1000165000u,
            t_uint8 = 1000265000
        };


        enum class buffer_modifier : uint32_t {
            t_unknown = 0u,
            t_gpu_only = 1u,
            t_cpu_to_gpu = 2u,
            t_gpu_to_cpu = 3u,
        };

    };
};
