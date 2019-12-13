#pragma once

#ifdef USE_VULKAN

#include <vulkan/vulkan.h>
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
            using result_t = VkResult;
            using buffer_t = VkBuffer;
            using device_t = VkDevice;
            using image_t = VkImage;
            using buffer_region_t = VkDescriptorBufferInfo;
        };
    };
#endif

#if defined(USE_VULKAN) && !defined(USE_D3D12)
    using namespace api::classes;
#endif

};
