#pragma once

#ifdef USE_VULKAN

#include <vulkan/vulkan.hpp>
#ifndef VULKAN_ENABLED
#define VULKAN_ENABLED
#endif

#ifndef VULKAN_HPP_ENABLED
#define VULKAN_HPP_ENABLED
#endif

#endif

namespace svt {
#ifdef USE_VULKAN
#define api vulkan
#endif

#ifdef USE_VULKAN
    namespace api {
        namespace classes {

        };
    };

    namespace core {
        namespace api {
            using result_t = vk::Result;
            using buffer_t = vk::Buffer;
            using device_t = vk::Device;
            using image_t = vk::Image;
            using queue_t = vk::Queue;
            using buffer_region_t = vk::DescriptorBufferInfo;
            using buffer_view_t = vk::BufferView;
            using image_view_t = vk::ImageView;
            using descriptor_set_template_t = vk::DescriptorUpdateTemplate;
            using descriptor_set_layout_t = vk::DescriptorSetLayout;
            using sampler_t = vk::Sampler;
            using image_desc_t = vk::DescriptorImageInfo;
            using physical_device_t = vk::PhysicalDevice;
            using pipeline_layout_t = vk::PipelineLayout;
            using graphics_pipeline_t = vk::Pipeline;
            using compute_pipeline_t = vk::Pipeline;
            using ray_tracing_pipeline_t = vk::Pipeline;
            using descriptor_set_t = vk::DescriptorSet;
            using pipeline_cache_t = vk::PipelineCache;
            using command_buffer_t = vk::CommandBuffer;
            using command_pool_t = vk::CommandPool;
            using instance_t = vk::Instance;
            using render_pass_t = vk::RenderPass;

            #ifndef API_NULL_HANDLE
            #define API_NULL_HANDLE {}
            #endif
        };
    };
#endif

#if defined(USE_VULKAN) && !defined(USE_D3D12)
    using namespace api::classes;
#endif

};
