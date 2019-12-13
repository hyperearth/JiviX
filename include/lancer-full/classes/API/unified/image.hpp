#pragma once

#include "./classes/API/types.hpp"
#include "./dynamic/API/unified/image.hpp"

namespace svt {
    namespace api {

        class image {
            protected: 
                std::shared_ptr<data::dynamic::image_t> image_t = {};
                std::shared_ptr<data::device_t> device_t = {};

            public: 
                struct flags_t {
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
                        bit_protected: 1;
                };

                struct usage_t {
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

                // TODO: more formats
                enum class format_t : uint32_t {
                    e_r16g16g16a16_unorm = 91u,
                    e_r16g16g16a16_snorm = 92u,
                    e_r16g16g16a16_uscaled = 93u,
                    e_r16g16g16a16_sscaled = 94u,

                    e_r16g16g16a16_uint = 95u,
                    e_r16g16g16a16_sint = 96u,
                    e_r16g16g16a16_sfloat = 97u,
                    
                    e_r32g32g32a32_uint = 107u,
                    e_r32g32g32a32_sint = 108u,
                    e_r32g32g32a32_sfloat = 109u,
                };

                struct create_info {
                    union {
                        uint32_t flags32u = 0u;
                        flags_t flags;
                    };
                    union {
                        uint32_t usage32u = 0u;
                        usage_t usage;
                    };
                };

                // 
                enum class sharing_mode_t : uint32_t { e_1d = 0u, e_2d, e_3d };

                // 
                image(const image& image) : device_t(image.device_t), image_t(image.image_t) {};
                image(const std::shared_ptr<data::dynamic::image_t>& image_t = {}) : image_t(image_t) {};
                image(const std::shared_ptr<data::device_t>& device_t, const std::shared_ptr<data::dynamic::image_t>& image_t = {}) : image_t(image_t), device_t(device_t) {};

                // UN-safe (Debug) API, always should begin from `_`
                svt::api::core::image_t _get_image_t();
        };
    };
};
