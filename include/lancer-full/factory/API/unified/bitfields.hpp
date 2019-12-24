#pragma once

#include "./core/unified/core.hpp"
//#include "./factory/API/types.hpp"


namespace svt {
    namespace api {


        struct image_flags { uint32_t
            b_sparse_binding: 1,
            b_sparse_residency: 1,
            b_sparse_aliased: 1,
            b_mutable_format: 1, 
            b_cube_compatible: 1,
            b_2d_array_compatible: 1, 
            b_block_texel_view_compatible: 1, 
            b_extended_usage: 1,
            b_disjoint: 1, 
            b_alias: 1,
            b_protected: 1,
            b_sample_location_compatible_depth: 1,
            b_corner_sampled: 1,
            b_subsampled: 1;
        };

        struct image_usage { uint32_t 
            b_transfer_src : 1,
            b_transfer_dst : 1,
            b_sampled : 1,
            b_storage : 1,
            b_color_attachment: 1, 
            b_depth_stencil_attachment: 1, 
            b_transiend_attachment: 1,
            b_input_attachment: 1,
            b_shading_rate: 1,
            b_fragment_density_map: 1;
        };

        struct sample_count { uint32_t 
            b_1: 1,
            b_2: 1,
            b_4: 1,
            b_8: 1,
            b_16: 1,
            b_32: 1,
            b_64: 1;
        };


        // TODO: extendent bits support
        struct buffer_flags { uint32_t 
            b_sparse_binding : 1,
            b_residency_binding : 1,
            b_sparse_alised : 1,
            b_protected : 1,
            b_device_address_capture : 1;
        };

        struct buffer_usage { uint32_t 
            b_transfer_src : 1,
            b_transfer_dst : 1,
            b_uniform_texel_buffer : 1,
            b_storage_texel_buffer : 1,
            b_uniform_buffer : 1,
            b_storage_buffer : 1,
            b_index_buffer : 1,
            b_vertex_buffer : 1,
            b_indirect_buffer : 1,
            b_conditional_rendering : 1,
            b_ray_tracing : 1,
            b_transform_feedback_buffer : 1,
            b_transform_feedback_counter_buffer : 1,
            b_shader_device_address : 1;
        };

        struct color_mask { uint32_t r: 1, g: 1, b: 1,  a: 1; };

        struct pipeline_stage_flags { uint32_t 
            b_top_of_pipe: 1,
            b_draw_indirect: 1,
            b_vertex_input: 1,
            b_vertex_shader: 1,
            b_tesselation_control_shader: 1,
            b_tesselation_evalution_shader: 1,
            b_geometry_shader: 1,
            b_fragment_shader: 1,
            b_early_fragment_tests: 1,
            b_late_fragment_tests: 1,
            b_color_attachment_output: 1,
            b_compute_shader: 1,
            b_transfer_bit: 1,
            b_bottom_of_pipe: 1,
            b_host: 1,
            b_all_graphics: 1,
            b_all_commands: 1,
            b_command_process: 1,
            b_conditional_rendering: 1,
            b_task_shader: 1,
            b_mesh_shader: 1,
            b_ray_tracing: 1,
            b_shading_rate_image: 1,
            b_fragment_density_process: 1,
            b_transform_feedback: 1,
            b_acceleration_structure: 1;
        };

        struct shader_stage_flags { uint32_t 
            b_vertex: 1,
            b_tesselation_control: 1,
            b_tesselation_evalution: 1,
            b_geometry: 1,
            b_fragment: 1,
            b_compute: 1,
            b_task: 1,
            b_mesh: 1,
            b_raygen: 1,
            b_any_hit: 1,
            b_closest_hit: 1,
            b_miss_hit: 1,
            b_intersection: 1,
            b_callable: 1;
        };

        struct cull_mode { uint32_t 
            b_front: 1,
            b_back: 1;
        };
        
        // TODO: 
        struct access_flags { uint32_t 
            b_indirect_command_read: 1,
            b_index_read: 1,
            b_vertex_attribute_read: 1,
            b_uniform_read_read: 1,
            b_input_attachment_read: 1,
            b_shader_read: 1,
            b_shader_write: 1,
            b_color_attachment_read: 1,
            b_color_attachment_write: 1,
            b_depth_stencil_attachment_read: 1,
            b_depth_stencil_attachment_write: 1,
            b_transfer_read: 1,
            b_transfer_write: 1,
            b_host_read: 1,
            b_host_write: 1,
            b_memory_write: 1,
            b_memory_write: 1,
            b_command_process_read: 1,
            b_command_process_write: 1,
            b_color_attachment_read_noncoherent: 1,
            b_conditional_rendering_read: 1,
            b_acceleration_structure_read: 1,
            b_acceleration_structure_write: 1,
            b_shading_rate_image_read: 1,
            b_fragment_density_map_read: 1,
            b_transform_feedback_write: 1,
            b_transform_feedback_counter_read: 1,
            b_transform_feedback_counter_write: 1;
        };

        struct descriptor_binding_flags { uint32_t
            b_update_after_bind: 1,
            b_update_unused_while_pending: 1,
            b_partially_bound: 1,
            b_variable_descriptor_count: 1;
        };

        struct acceleration_structure_flags { uint32_t
            b_allow_update: 1,
            b_allow_compation: 1,
            b_prefer_fast_trace: 1,
            b_prefer_fast_build: 1,
            b_low_memory: 1;
        };

        struct geometry_flags { uint32_t
            b_opaque: 1,
            b_no_dublicate_any_hit_invocation: 1;
        };


    };
};
