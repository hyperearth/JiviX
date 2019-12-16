#pragma once

#include "./core/unified/core.hpp"
#include "./factory/API/types.hpp"

namespace svt {
    namespace api {
        namespace factory {
            class buffer_view_t : public std::enable_shared_from_this<buffer_view_t> { public: 
                core::api::buffer_view_t buffer_view;
            };
        };
    };
};
