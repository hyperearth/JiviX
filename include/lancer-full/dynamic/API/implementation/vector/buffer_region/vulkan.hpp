#pragma once

#include "./core/unified/core.hpp"
#include "./dynamic/API/types.hpp"
#include "./dynamic/API/unified/buffer.hpp"
#include "./dynamic/API/unified/vector.hpp"

namespace svt {
    namespace api {
        namespace dynamic {

            vector_t::operator const svt::core::api::buffer_region_t&() const {
                return *(svt::core::api::buffer_region_t*)(&this->handle);
            };

            vector_t::operator svt::core::api::buffer_region_t&() {
                return *(svt::core::api::buffer_region_t*)(&this->handle);
            };

        };
    };
};
