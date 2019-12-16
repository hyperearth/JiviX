#pragma once

#include "./core/unified/core.hpp"
#include "./factory/API/types.hpp"

namespace svt {
    namespace api {
        namespace factory {
            class sampler_t : public std::enable_shared_from_this<sampler_t> { public: 
                core::api::sampler_t sampler;
            };
        };
    };
};
