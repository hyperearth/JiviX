#pragma once

#include "./core/unified/core.hpp"
#include "./dynamic/API/types.hpp"

namespace svt {
    namespace api {
        namespace factory {

            // Can Be Extended By VMA Allocators
            class image_t : public std::enable_shared_from_this<image_t> { public: 
                //std::vector<uint32_t> queueFamilyIndices = {};
                core::api::image_t image;

                virtual ~image_t(){};
            };
        };
    };
};
