#if defined(_WIN32) && !defined(WIN32)
#define WIN32
#endif

#ifdef WIN32
#define VK_USE_PLATFORM_WIN32_KHR
#endif

//
#define VMA_IMPLEMENTATION
#define VKT_FORCE_VMA_IMPLEMENTATION
#define VKT_ENABLE_GLFW_SUPPORT
#define TINYGLTF_IMPLEMENTATION
#define TINYEXR_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#define VKT_ENABLE_GLFW_LINKED
#define VKT_ENABLE_GLFW_SURFACE

// 
#include "./pch.hpp"

