#pragma once

#include "./core/unified/core.hpp"
#include "./factory/API/types.hpp"
#include "./factory/API/unified/enums.hpp"

namespace svt {
    namespace api {
        namespace stu {
            using image = std::shared_ptr<api::factory::image_t>;
            using vector = std::shared_ptr<api::factory::vector_t>;
            using buffer = std::shared_ptr<api::factory::buffer_t>;
            using device = std::shared_ptr<api::factory::device_t>;
            using allocator = std::shared_ptr<api::factory::allocator_t>;
            using buffer_view = std::shared_ptr<api::factory::buffer_view_t>;
            using image_view = std::shared_ptr<api::factory::image_view_t>;
            using sampler = std::shared_ptr<api::factory::sampler_t>;
            using allocation = std::shared_ptr<api::factory::allocation_t>;
            using descriptor_set = std::shared_ptr<api::factory::descriptor_set_t>;
            using descriptor_set_layout = std::shared_ptr<api::factory::descriptor_set_layout_t>;
            using ray_tracing_pipeline = std::shared_ptr<api::factory::ray_tracing_pipeline_t>;
            using graphics_pipeline = std::shared_ptr<api::factory::graphics_pipeline_t>;
            using compute_pipeline = std::shared_ptr<api::factory::compute_pipeline_t>;
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
            class ray_tracing_pipeline;
            class command_pool;
            class descriptor_pool;
            class allocator;
            class sampler;
            class buffer_view;
            class image_view;
            class allocation;

            template<class T = uint8_t>
            class vector;
        };

        
    };
};
