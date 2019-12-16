#pragma once

#include "./core/unified/core.hpp"
#include "./factory/API/types.hpp"

namespace svt {
    namespace api {
        namespace factory {
            class descriptor_set_t : public std::enable_shared_from_this<descriptor_set_t> { public: 
                //std::vector<api::factory::vector_t> buffers = {};
                //std::vector<api::factory::buffer_view_t> buffer_views = {};
                //std::vector<api::factory::image_view_t> image_views = {};

                //core::api::result_t set_buffer();
            };
        };
    };
};
