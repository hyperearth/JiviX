#pragma once

#ifdef ENABLE_EXTENSION_RTX
#define EXTENSION_RTX
#endif

#ifdef ENABLE_EXTENSION_VMA
#define EXTENSION_VMA
#endif

#include <lancer/lib/core.hpp>
#include <lancer/API/memory.hpp>
#include <lancer/API/device.hpp>
#include <lancer/API/image.hpp>
#include <lancer/API/constructors.hpp>
#include <lancer/API/utils.hpp>

// Currently Only Supported
#ifdef ENABLE_EXTENSION_VMA
#include <lancer/EXT/VMA.hpp>
#endif

// TODO: Extension System for Ray-Tracers
#ifdef EXTENSION_RTX
#include <lancer/EXT/RTX.hpp>
#endif
