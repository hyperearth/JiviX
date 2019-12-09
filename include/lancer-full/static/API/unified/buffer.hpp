#pragma one

#include "../../../core/unified/core.hpp"

namespace svt {
    namespace data {

        struct buffer_t {
            core::buffer_t buffer;
            std::vector<uint32_t> queueFamilyIndices = {};
        };

    };
};
