#pragma one

#include "../core/core.hpp"

namespace svt {
    namespace data {

        struct buffer_t {
            api::Buffer buffer;
            std::vector<uint32_t> queueFamilyIndices = {};
        };

    };
};
