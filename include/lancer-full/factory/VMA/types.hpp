#pragma once

#include "./core/unified/core.hpp"
#include "./factory/API/unified/enums.hpp"
#include "./factory/API/types.hpp"

namespace svt {
    namespace vma {
        namespace factory {
            class device_t;
            class command_buffer_t;
            class queue_t;
            class descriptor_set_t;
            class descriptor_set_layout_t;
            class pipeline_layout_t;
            class raytracing_pipeline_t;
            class graphics_pipeline_t;
            class compute_pipeline_t;
            class command_pool_t;
            class descriptor_pool_t;
            class allocator_t;
            class buffer_t;
            class vector_t;
            class image_t;
            class buffer_view_t;
            class image_view_t;
            class allocation_t;
            class sampler_t;
        };
    };
};
