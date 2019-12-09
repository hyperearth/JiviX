#pragma once

#include <vulkan/vulkan.hpp>

#ifndef VULKAN_ENABLED
#define VULKAN_ENABLED
#endif

#ifndef VULKAN_HPP_ENABLED
#define VULKAN_HPP_ENABLED
#endif

namespace svt {
    namespace core {
        namespace api { using namespace vk; }; // safer version 

        using result_t = api::Result;
        using buffer_t = api::Buffer;
        using device_t = api::Device;
        
    };
};
