#pragma once

#include "./core/unified/core.hpp"

namespace svt {
    namespace api {
        namespace factory {

            struct device_t {
                core::api::device_t device;
                std::vector<uint32_t> queueFamilyIndices = {};
            };

        };
    };
};
