#pragma once

#include "./core/unified/core.hpp"
#include "./factory/API/types.hpp"
#include "./factory/API/unified/enums.hpp"
#include "./factory/API/unified/device.hpp"

namespace svt {
    namespace api {
        namespace stu {
            using image = std::shared_ptr<api::factory::image_t>;
            using queue = std::shared_ptr<api::factory::queue_t>;
            using sampler = std::shared_ptr<api::factory::sampler_t>;
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
            using pipeline_layout = std::shared_ptr<api::factory::pipeline_layout_t>;
            using command_buffer = std::shared_ptr<api::factory::command_buffer_t>;
            using command_pool = std::shared_ptr<api::factory::command_pool_t>;

            struct device_t {
                stu::device device_ = {};
                std::vector<uint32_t> queue_family_indices_ = {};

                operator std::vector<uint32_t>&() { return queue_family_indices_; };
                operator const std::vector<uint32_t>&() const { return queue_family_indices_; };
                operator stu::device&() { return device_; };
                operator const stu::device&() const { return device_; };

                api::factory::device_t& operator *() { return *device_; };
                const api::factory::device_t& operator *() const { return *device_; };

                operator uintptr_t&() { return (*device_); };
                operator const uintptr_t&() const { return (*device_); };
                operator core::api::device_t&() { return (*device_); };
                operator const core::api::device_t&() const { return (*device_); };
            };
        };




        namespace classes {
            class buffer;
            class device;
            class queue;
            class image;
            class command_buffer;
            class descriptor_set;
            class descriptor_set_layout;
            class pipeline_layout;
            class graphics_pipeline;
            class compute_pipeline;
            class ray_tracing_pipeline;
            class descriptor_pool;
            class allocator;
            class sampler;
            class buffer_view;
            class image_view;
            class allocation;
            class sampler;
            class pipeline_layout;
            class command_pool_t;
            class command_buffer_t;

            template<class T = uint8_t>
            class vector;
        };

        
    };
};
