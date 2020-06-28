#pragma once 

#if defined(_WIN32) && !defined(WIN32)
#define WIN32
#endif

#ifdef WIN32
#define VK_USE_PLATFORM_WIN32_KHR
#endif

//
#define VKT_ENABLE_GLFW_SUPPORT
#define VKT_ENABLE_GLFW_LINKED
#define VKT_ENABLE_GLFW_SURFACE

//
#include <string>
#include <iostream>

//
#include <vma/vk_mem_alloc.h>
#include <vkt3/fw.hpp>
#include <JiviX/JiviX.hpp>
#include <misc/tiny_gltf.h>
#include <misc/tinyexr.h>
#include <tinyobjloader/tiny_obj_loader.h>
