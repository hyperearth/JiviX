// #
#if defined(_WIN32) && !defined(WIN32)
#define WIN32
#endif

//
#ifdef WIN32
#define VK_USE_PLATFORM_WIN32_KHR
#endif

//
#define VMA_IMPLEMENTATION
#define TINYEXR_IMPLEMENTATION

//
#include "./pch.hpp"

//
#include <vkt3/fw.hpp>
#include <GLFW/glfw3.h>
#include <vma/vk_mem_alloc.h>
#include <JiviX/JiviX.hpp>
#include <misc/tinyexr.h>

//
namespace vkx {

};
