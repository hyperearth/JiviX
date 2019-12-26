//#pragma once

#include "./classes/API/types.hpp"
#include "./classes/API/unified/device.hpp"
#include "./classes/API/unified/swapchain.hpp"
#include "./classes/API/unified/framebuffer.hpp"
#include "./classes/API/unified/descriptor_set_layout.hpp"
#include "./classes/API/unified/descriptor_set.hpp"
#include "./classes/API/unified/graphics_pipeline.hpp"
#include "./classes/API/unified/compute_pipeline.hpp"
#include "./classes/API/unified/ray_tracing_pipeline.hpp"
#include "./classes/API/unified/render_pass.hpp"
#include "./classes/API/unified/pipeline_layout.hpp"
#include "./classes/API/unified/thread_set.hpp"

namespace svt {
    namespace api {
        namespace classes {
#ifdef USE_VULKAN
            swapchain thread_set::create_swapchain(const swapchain_create_info& info = {}) const { return device((stu::device_t&)thread_).create_swapchain(info); };
            framebuffer thread_set::create_framebuffer(const framebuffer_create_info& info = {}) const { return device((stu::device_t&)thread_).create_framebuffer(info); };
            descriptor_pool thread_set::create_descriptor_pool(const descriptor_pool_create_info& info = {}) const { return device((stu::device_t&)thread_).create_descriptor_pool(info); };
            descriptor_set_layout thread_set::create_descriptor_set_layout(const descriptor_set_layout_create_info& info = {}) const { return device((stu::device_t&)thread_).create_descriptor_set_layout(info); };
            graphics_pipeline thread_set::create_graphics_pipeline(const graphics_pipeline_create_info& info = {}) const { return device((stu::device_t&)thread_).create_graphics_pipeline(info); };
            compute_pipeline thread_set::create_compute_pipeline(const compute_pipeline_create_info& info = {}) const { return device((stu::device_t&)thread_).create_compute_pipeline(info); };
            ray_tracing_pipeline thread_set::create_ray_tracing_pipeline(const ray_tracing_pipeline_create_info& info = {}) const { return device((stu::device_t&)thread_).create_ray_tracing_pipeline(info); };
            render_pass thread_set::create_render_pass(const render_pass_create_info& info = {}) const { return device((stu::device_t&)thread_).create_render_pass(info); };
            pipeline_layout thread_set::create_pipeline_layout(const pipeline_layout_create_info& info = {}) const { return device((stu::device_t&)thread_).create_pipeline_layout(info); };
#endif
        };
    };
};
