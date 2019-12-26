#pragma once

#include "./classes/API/types.hpp"
#include "./factory/API/unified/device.hpp"
#include "./factory/API/unified/queue.hpp"
#include "./factory/API/unified/command_pool.hpp"
#include "./factory/API/unified/descriptor_pool.hpp"

namespace svt {
    namespace api {
        namespace classes {
            class device { public: 
                device(const device& device) : device_(device) {};
                device(const stu::device_t& device_ = {}) : device_(device_) {};

                // TODO: add assigment by core types and shared_ptr types
                device& operator=(const device &device) { this->device_ = device; return *this; };

                // TODO: move into `.cpp` file
                operator stu::physical_device&() { return device_; };
                operator stu::device&() { return device_; };
                operator stu::queue&() { return queue_; };
                operator stu::command_pool&() { return command_pool_; };
                operator stu::descriptor_pool&() { return descriptor_pool_; };
                operator stu::device_t&() { return device_; };
                operator const stu::physical_device&() const { return device_; };
                operator const stu::device&() const { return device_; };
                operator const stu::queue&() const { return queue_; };
                operator const stu::command_pool&() const { return command_pool_; };
                operator const stu::descriptor_pool&() const { return descriptor_pool_; };
                operator const stu::device_t&() const { return device_; };

                // 
                api::factory::device_t* operator->() { return &(*this->device_); };
                const api::factory::device_t* operator->() const { return &(*this->device_); };
                api::factory::device_t& operator*() { return (*this->device_); };
                const api::factory::device_t& operator*() const { return (*this->device_); };

                // 
                operator uintptr_t&() { return device_; };
                operator const uintptr_t&() const { return device_; };

                operator core::api::device_t&() { return device_; };
                operator const core::api::device_t&() const { return device_; };

                operator core::api::physical_device_t&() { return device_; };
                operator const core::api::physical_device_t&() const { return device_; };

                operator core::api::queue_t&() { return *queue_; };
                operator const core::api::queue_t&() const { return *queue_; };

                operator core::api::command_pool_t&() { return *command_pool_; };
                operator const core::api::command_pool_t&() const { return *command_pool_; };
                
                operator core::api::descriptor_pool_t&() { return *descriptor_pool_; };
                operator const core::api::descriptor_pool_t&() const { return *descriptor_pool_; };

                // 
                swapchain create_swapchain(const swapchain_create_info& info = {}) const;
                framebuffer create_framebuffer(const framebuffer_create_info& info = {}) const;
                descriptor_pool create_descriptor_pool(const descriptor_pool_create_info& info = {}) const;
                descriptor_set_layout create_descriptor_set_layout(const descriptor_set_layout_create_info& info = {}) const;
                graphics_pipeline create_graphics_pipeline(const graphics_pipeline_create_info& info = {}) const;
                compute_pipeline create_compute_pipeline(const compute_pipeline_create_info& info = {}) const;
                ray_tracing_pipeline create_ray_tracing_pipeline(const ray_tracing_pipeline_create_info& info = {}) const;
                render_pass create_render_pass(const render_pass_create_info& info = {}) const;
                //TODO: pipeline_layout create_pipeline_layout(const pipeline_layout_create_info& info = {}) const;
                //TODO: queue create_queue(const queue_create_info& info = {}) const;
                

                // TODO: Implement Allocator Create
                template<class T = api::factory::allocator_t> 
                allocator create_allocator(const uintptr_t& info = 0ull);

            // TODO: full support, multi-threading
            protected: friend device; 
                stu::device_t device_ = {};
                stu::queue queue_ = {};
                stu::command_pool command_pool_ = {};
                stu::descriptor_pool descriptor_pool_ = {};
            };
        };
    };
};
