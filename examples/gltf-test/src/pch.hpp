#pragma once 

//
#define VKT_ENABLE_GLFW_LINKED
#define VKT_ENABLE_GLFW_SURFACE

//
#if defined(_WIN32) && !defined(WIN32)
#define WIN32
#endif

//
#ifdef WIN32
#define VK_USE_PLATFORM_WIN32_KHR
#endif

//
#define VKT_ENABLE_GLFW_SUPPORT

//
#include <string>
#include <iostream>

//
#include <vkt3/fw.hpp>
#include <GLFW/glfw3.h>
#include <vma/vk_mem_alloc.h>
#include <JiviX/JiviX.hpp>
#include <misc/tinyexr.h>

