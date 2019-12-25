#pragma once

// Default Backend
#if !defined(USE_D3D12) && !defined(USE_VULKAN)
#define USE_VULKAN
#endif

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
            using descriptor_pool_t = vk::DescriptorPool;
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
            using dispatch_t = vk::DispatchLoaderDynamic;
            using shader_module_t = vk::ShaderModule;
            using acceleration_structure_t = vk::AccelerationStructureNV;
            using framebuffer_t = vk::Framebuffer;
            using swapchain_t = vk::SwapchainKHR;
            using surface_t = vk::SurfaceKHR;

            #ifndef API_NULL_HANDLE
            #define API_NULL_HANDLE {}
            #endif

            // TODO: Getting Recommended Properties
            class physical_device_properties_t { public: 
            #if defined(USE_VULKAN)
                //vk::PhysicalDeviceProperties2 properties_ = {};
                //vk::PhysicalDeviceConservativeRasterizationPropertiesEXT conservative_rasterization_properties_ = {};
                //vk::PhysicalDeviceSubgroupSizeControlPropertiesEXT subgroup_size_control_properties_ = {};
                //vk::PhysicalDeviceSampleLocationsPropertiesEXT sample_locations_properties = {};
                //vk::PhysicalDeviceRayTracingPropertiesNV ray_tracing_properties_ = {};
                //vk::MultisamplePropertiesEXT multisample_properties_ = {};
            #endif
            };

            // TODO: Getting Recommended Features
            class physical_device_features_t { public: 
            #if defined(USE_VULKAN)
                //vk::PhysicalDeviceFeatures2 features_ = {};
                //vk::PhysicalDeviceSubgroupSizeControlFeaturesEXT subgroup_size_control_features_ = {};
            #endif
            };

        };
    };
#endif

#if defined(USE_VULKAN) && !defined(USE_D3D12)
    using namespace api::classes;
#endif

};
