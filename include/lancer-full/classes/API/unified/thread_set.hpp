#pragma once

#include "./classes/API/types.hpp"
#include "./classes/API/unified/device.hpp"
#include "./classes/API/unified/queue.hpp"
#include "./classes/API/unified/command_pool.hpp"
#include "./classes/API/unified/descriptor_pool.hpp"

namespace svt {
    namespace api {
        namespace classes {

            class thread_set { public: // 
                thread_set(const thread_set& thread) : thread_(thread) {};
                thread_set(const stu::thread_set_t& thread_) : thread_(thread_) {};

                // TODO: add assigment by core types and shared_ptr types
                thread_set& operator=(const thread_set& thread) { this->thread_ = (stu::thread_set_t&)thread; return *this; };
                thread_set& operator=(const stu::thread_set_t& thread_) { this->thread_ = thread_; return *this; };

                // TODO: move into `.cpp` file
                operator stu::thread_set_t&() { return thread_; };
                operator stu::physical_device&() { return thread_; };
                operator stu::device&() { return thread_; };
                operator stu::queue&() { return thread_; };
                operator stu::command_pool&() { return thread_; };
                operator stu::descriptor_pool&() { return thread_; };
                operator stu::device_t&() { return thread_; };
                operator const stu::thread_set_t&() const { return thread_; };
                operator const stu::physical_device&() const { return thread_; };
                operator const stu::device&() const { return thread_; };
                operator const stu::queue&() const { return thread_; };
                operator const stu::command_pool&() const { return thread_; };
                operator const stu::descriptor_pool&() const { return thread_; };
                operator const stu::device_t&() const { return thread_; };

                // 
                //api::factory::device_t* operator->() { return &(*thread_.device_); };
                //const api::factory::device_t* operator->() const { return &(*thread_.device_); };
                //api::factory::device_t& operator*() { return (*thread_.device_); };
                //const api::factory::device_t& operator*() const { return (*thread_.device_); };

                // 
                //operator uintptr_t&() { return thread_.device_; };
                //operator const uintptr_t&() const { return thread_.device_; };
                operator core::api::device_t&() { return *thread_.device_; };
                operator const core::api::device_t&() const { return *thread_.device_; };
                operator core::api::physical_device_t&() { return *thread_.device_; };
                operator const core::api::physical_device_t&() const { return *thread_.device_; };
                operator core::api::queue_t&() { return *thread_.queue_; };
                operator const core::api::queue_t&() const { return *thread_.queue_; };
                operator core::api::command_pool_t&() { return *thread_.command_pool_; };
                operator const core::api::command_pool_t&() const { return *thread_.command_pool_; };
                operator core::api::descriptor_pool_t&() { return *thread_.descriptor_pool_; };
                operator const core::api::descriptor_pool_t&() const { return *thread_.descriptor_pool_; };

                // hack with device 
                swapchain create_swapchain(const swapchain_create_info& info = {}) const;
                framebuffer create_framebuffer(const framebuffer_create_info& info = {}) const;
                descriptor_pool create_descriptor_pool(const descriptor_pool_create_info& info = {}) const;
                descriptor_set_layout create_descriptor_set_layout(const descriptor_set_layout_create_info& info = {}) const;
                graphics_pipeline create_graphics_pipeline(const graphics_pipeline_create_info& info = {}) const;
                compute_pipeline create_compute_pipeline(const compute_pipeline_create_info& info = {}) const;
                ray_tracing_pipeline create_ray_tracing_pipeline(const ray_tracing_pipeline_create_info& info = {}) const;
                render_pass create_render_pass(const render_pass_create_info& info = {}) const;
                pipeline_layout create_pipeline_layout(const pipeline_layout_create_info& info = {}) const;
                //queue create_queue(const queue_create_info& info = {}) const;
                

                // TODO: Implement Allocator Create
                template<class T = api::factory::allocator_t> 
                allocator create_allocator(const uintptr_t& info = 0ull);

            // TODO: full support, multi-threading
            protected: friend device; 
                stu::thread_set_t thread_ = {};
                //stu::device_t device_ = {};
                //stu::queue queue_ = {};
                //stu::command_pool command_pool_ = {};
                //stu::descriptor_pool descriptor_pool_ = {};
            };
        };
    };
};
