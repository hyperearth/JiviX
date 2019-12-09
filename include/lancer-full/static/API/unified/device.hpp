#pragma one

#include "../../../core/unified/core.hpp"

namespace svt {
    namespace data {

        struct device_t {
            core::device_t device;
            std::vector<uint32_t> queueFamilyIndices = {};
        };

    };
};
