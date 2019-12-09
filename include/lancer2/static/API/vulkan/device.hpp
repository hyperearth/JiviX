#pragma one

#include "../core/core.hpp"

namespace svt {
    namespace data {

        struct device_t {
            api::Device device;
            std::vector<uint32_t> queueFamilyIndices = {};
        };

    };
};
