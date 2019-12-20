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

        struct image_sample_count { uint32_t 
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
            union {
                uint32_t flags32u = 0b00000000000000000000000000000000;
                buffer_flags flags;
            };

            size_t size = 4u;

            union {
                uint32_t usage32u = 0b00000000000000000000000000000000;
                buffer_usage usage;
            };

            sharing_mode sharing_mode = sharing_mode::t_exclusive;
        };

        struct image_create_info {
            union {
                uint32_t flags32u = 0b00000000000000000000000000000000;
                image_flags flags;
            };

            image_type image_type = image_type::t_1d;
            format format{ 0u };
            extent_3d extent;
            uint32_t mip_levels = 1u;
            uint32_t array_layers = 1u;

            union {
                uint32_t samples32u = 0b00000000000000000000000000000001;
                image_sample_count samples;
            };

            image_tiling tiling = image_tiling::t_optimal;

            union {
                uint32_t usage32u = 0b00000000000000000000000000000000;
                image_usage usage;
            };

            sharing_mode sharing_mode = sharing_mode::t_exclusive;
            //image_layout initial_layout = image_layout::t_undefined;
        };


        // TODO: Complete Pipeline Support
        class pipeline_shader_stage { public: 
            
        };

        class graphics_pipeline_create_info { public: uint32_t flags = 0u;
            std::vector<pipeline_shader_stage> stages = {};

            struct dynamic_state {} dynamic_state;
            struct viewport_state {} viewport_state;
            struct tesselation_state {} tesselation_state;
            struct multisample_state {} multisample_state;
            struct color_blend_state {} color_blend_state;
            struct vertex_input_state {} vertex_input_state;
            struct rasterization_state {} rasterization_state;
            struct depth_stencil_state {} depth_stencil_state;
            struct vertex_assembly_state {} vertex_assembly_state;

            //struct conservative_rasterization_state {} conservative_rasterization_state;
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
