#pragma once

#include "./classes/API/types.hpp"
#include "./factory/API/unified/allocator.hpp"
#include "./factory/API/unified/image.hpp"

namespace svt {
    namespace api {
        namespace classes {
            class image {
                protected: 
                    std::shared_ptr<api::factory::image_t> image_t = {};
                    std::shared_ptr<api::factory::device_t> device_t = {};

                public: 
                    struct flags {
                        uint32_t
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

                    struct usage {
                        uint32_t 
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

                    struct sample_count {
                        uint32_t 
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

                    struct create_info {
                        union {
                            uint32_t flags32u = 0u;
                            flags flags;
                        };
                        union {
                            uint32_t usage32u = 0u;
                            usage usage;
                        };
                        union {
                            uint32_t samples32u = 1u;
                            sample_count samples;
                        };
                        
                        sharing_mode sharing_mode = sharing_mode::t_exclusive;
                        image_layout initial_layout = image_layout::t_undefined;
                        image_tiling tiling = image_tiling::t_optimal;
                        format format{0u};
                        extent_3d extent;
                        uint32_t mip_levels = 1u;
                        uint32_t array_layers = 1u;
                    };

                    // 
                    image(const image& image) : device_t(image.device_t), image_t(image.image_t) {};
                    image(const std::shared_ptr<api::factory::image_t>& image_t = {}) : image_t(image_t) {};
                    image(const std::shared_ptr<api::factory::device_t>& device_t, const std::shared_ptr<api::factory::image_t>& image_t = {}) : image_t(image_t), device_t(device_t) {};

                    // UN-safe (Debug) API, always should begin from `_`
                    svt::core::api::image_t _get_image_t();
            };
        };
    };
};
