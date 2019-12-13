#pragma once

#include "./classes/API/types.hpp"

namespace svt {
    namespace api {
        
        struct extent_2d { uint32_t width = 1u, height = 1u; };
        struct extent_3d { uint32_t width = 1u, height = 1u, depth = 1u; };
        struct offset_2d { int32_t x = 0u, x = 0u; };
        struct offset_3d { int32_t x = 0u, y = 0u, z = 0u; };

        // 
        enum class sharing_mode : uint32_t {
            t_exclusive = 0u,
            t_concurrent = 1u
        };

        // TODO: more formats
        enum class format : uint32_t {
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
        

    };
};
