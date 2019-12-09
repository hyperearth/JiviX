#pragma once

#include <vulkan/vulkan.h>

#ifndef VULKAN_ENABLED
#define VULKAN_ENABLED
#endif

namespace svt {
    namespace core {
    
        using result_t = VkResult;
        using buffer_t = VkBuffer;
        using device_t = VkDevice;
        
    };
};
