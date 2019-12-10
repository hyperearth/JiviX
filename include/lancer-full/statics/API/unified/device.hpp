#pragma once

#include "../../../core/unified/core.hpp"

namespace svt {
    namespace api {
        namespace data {

            struct device_t {
                core::device_t device;
                std::vector<uint32_t> queueFamilyIndices = {};
            };

        };
    };
};
