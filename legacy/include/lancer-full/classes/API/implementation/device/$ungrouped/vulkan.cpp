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

namespace svt {
    namespace api {
        namespace classes {
#ifdef USE_VULKAN
            swapchain device::create_swapchain(const swapchain_create_info& info) const {
                auto swapchain_ = swapchain(device_); swapchain_.create(info); return swapchain_;
            };
            framebuffer device::create_framebuffer(const framebuffer_create_info& info) const {
                auto framebuffer_ = framebuffer(device_); framebuffer_.create(info); return framebuffer_;
            };
            descriptor_set_layout device::create_descriptor_set_layout(const descriptor_set_layout_create_info& info) const {
                auto descriptor_set_layout_ = descriptor_set_layout(device_); descriptor_set_layout_.create(info); return descriptor_set_layout_;
            };
            graphics_pipeline device::create_graphics_pipeline(const graphics_pipeline_create_info& info) const {
                auto graphics_pipeline_ = graphics_pipeline(device_); graphics_pipeline_.create(info); return graphics_pipeline_;
            };
            compute_pipeline device::create_compute_pipeline(const compute_pipeline_create_info& info) const {
                auto compute_pipeline_ = compute_pipeline(device_); compute_pipeline_.create(info); return compute_pipeline_;
            };
            ray_tracing_pipeline device::create_ray_tracing_pipeline(const ray_tracing_pipeline_create_info& info) const {
                auto ray_tracing_pipeline_ = ray_tracing_pipeline(device_); ray_tracing_pipeline_.create(info); return ray_tracing_pipeline_;
            };
            render_pass device::create_render_pass(const render_pass_create_info& info) const {
                auto render_pass_ = render_pass(device_); render_pass_.create(info); return render_pass_;
            };
            pipeline_layout device::create_pipeline_layout(const pipeline_layout_create_info& info) const {
                auto pipeline_layout_ = pipeline_layout(device_); pipeline_layout_.create(info); return pipeline_layout_;
            };
#endif
        };
    };
};
