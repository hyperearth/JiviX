// #

#if defined(_WIN32) && !defined(WIN32)
#define WIN32
#endif

#ifdef WIN32
#define VK_USE_PLATFORM_WIN32_KHR
#endif

//
#define VMA_IMPLEMENTATION
#define TINYGLTF_IMPLEMENTATION
#define TINYEXR_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION

//
#include "./pch.hpp"

//
#include <vma/vk_mem_alloc.h>
#include <vkt3/fw.hpp>
#include <JiviX/JiviX.hpp>
#include <misc/tinyexr.h>
#include <misc/tiny_gltf.h>

//
namespace vkx {

};
