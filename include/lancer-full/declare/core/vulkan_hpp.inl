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
    //namespace api { using namespace vk; }; // safer version 
    namespace api {
        using namespace svt;
        namespace core {
            using result_t = vk::Result;
            using buffer_t = vk::Buffer;
            using device_t = vk::Device;
            using image_t = vk::Image;
        };
    };
    namespace core {
        
    };
#endif

#if defined(USE_VULKAN) && !defined(USE_D3D12)
    using namespace api;
#endif

};
