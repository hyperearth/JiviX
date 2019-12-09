#pragma once

#ifdef USE_VULKAN

#include <vulkan/vulkan.h>

#ifndef VULKAN_ENABLED
#define VULKAN_ENABLED
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
            using result_t = VkResult;
            using buffer_t = VkBuffer;
            using device_t = VkDevice;
        };
    };
    namespace core {
        
    };
#endif

#if defined(USE_VULKAN) && !defined(USE_D3D12)
    using namespace api;
#endif

};
