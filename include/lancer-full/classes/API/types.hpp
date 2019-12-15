#pragma once

#include "./core/unified/core.hpp"
#include "./factory/API/types.hpp"
#include "./classes/API/unified/enums.hpp"

namespace svt {
    namespace api {
        namespace stu {
            using image = std::shared_ptr<api::factory::image_t>;
            using vector = std::shared_ptr<api::factory::vector_t>;
            using buffer = std::shared_ptr<api::factory::buffer_t>;
            using device = std::shared_ptr<api::factory::device_t>;
            using allocator = std::shared_ptr<api::factory::allocator_t>;
        };

        namespace classes {
            class buffer;
            class device;
            class image;
            class command_buffer;
            class queue;
            class descriptor_set;
            class descriptor_set_layout;
            class pipeline_layout;
            class graphics_pipeline;
            class compute_pipeline;
            class raytracing_pipeline;
            class command_pool;
            class descriptor_pool;
            class allocator;
        };

        
    };
};
