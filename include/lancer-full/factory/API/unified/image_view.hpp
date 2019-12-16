#pragma once

#include "./core/unified/core.hpp"
#include "./factory/API/types.hpp"

namespace svt {
    namespace api {
        namespace factory {
            class image_view_t : public std::enable_shared_from_this<image_view_t> { public: 
                core::api::image_view_t image_view;
            };
        };
    };
};
