#pragma once

#include "./classes/API/types.hpp"
#include "./dynamic/API/unified/allocator.hpp"
#include "./dynamic/API/unified/image.hpp"

namespace svt {
    namespace api {
        namespace classes {
            class image {
                protected: 
                    std::shared_ptr<api::dynamic::image_t> image_t = {};
                    std::shared_ptr<api::statics::device_t> device_t = {};

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
                        format format{0u};
                        extent_3d extent;
                        uint32_t mip_levels = 1u;
                        uint32_t array_layers = 1u;

                        uint32_t tiling = 0u; // TODO: Tiling Type
                        uint32_t initial_layout = 0u; // TODO: Image Layout Type
                    };

                    // 
                    image(const image& image) : device_t(image.device_t), image_t(image.image_t) {};
                    image(const std::shared_ptr<api::dynamic::image_t>& image_t = {}) : image_t(image_t) {};
                    image(const std::shared_ptr<api::statics::device_t>& device_t, const std::shared_ptr<api::dynamic::image_t>& image_t = {}) : image_t(image_t), device_t(device_t) {};

                    // UN-safe (Debug) API, always should begin from `_`
                    svt::core::api::image_t _get_image_t();
            };
        };
    };
};
