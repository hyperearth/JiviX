#pragma once

#include "../../../core/unified/core.hpp"

namespace svt {
    namespace api {
        namespace data {

            struct buffer_t {
                core::buffer_t buffer;
                std::vector<uint32_t> queueFamilyIndices = {};
            };

        };
    };
};
