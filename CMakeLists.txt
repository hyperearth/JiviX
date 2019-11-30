cmake_minimum_required (VERSION 3.10)
set (PROJECT_NAME LancER)
project (${PROJECT_NAME} C CXX)

# Architecture specific
if (COMPILER_ID MATCHES "MSVC")
    set(MSVC_${lang}_ARCHITECTURE_ID "${ARCHITECTURE_ID}")
endif()

# Import modules 
set (DEP_DIR "${CMAKE_CURRENT_SOURCE_DIR}")
set (CMAKE_PREFIX_PATH ${CMAKE_PREFIX_PATH} "${DEP_DIR}" "${DEP_DIR}/lib/cmake/")

# Set language specific
set (CMAKE_CXX_EXTENSIONS 1)
set (CMAKE_CXX_STANDARD 20)
set (CMAKE_C_STANDARD 11)

# Set source directories
set(SOURCES_LIST 
    "./include/*.h"
    "./include/*.hpp"
    "./include/lancer/*"
    "./include/lancer/*/*"
    "./include/lancer/*/*/*"
    "./include/radx/*"
    "./include/vulkan/*"
    "./include/vma/*"
    "./include/vku/*"
    "./include/vkt/*"
    "./include/volk/*"
    "./include/spirv/*"
    "./include/spirv/*/*"
    "./src/*.hpp"
    "./src/*.h"
    "./src/*/*.hpp"
    "./src/*/*.h"
    "./src/include/*.hpp"
    "./src/include/*.h"
    "./src/include/*/*.hpp"
    "./src/include/*/*.h"
)

# Find packages (Linux Prior)
find_package (Vulkan QUIET)
find_package (GLFW3 QUIET)
find_package (PkgConfig QUIET)
pkg_search_module(GLFW QUIET glfw3)
pkg_search_module(GLFW QUIET renderdoc)

# Set libs and include dirs of projects
set (LIBS glfw3)
set (LIB_DIR ${PROJECT_SOURCE_DIR} "${DEP_DIR}/lib")
set (INC_DIR 
    "${DEP_DIR}/include" 
    "${DEP_DIR}/src/" 
    "${DEP_DIR}/src/base/" 
    "${DEP_DIR}")

# Set definition
set (DEFINES
    -D_CRT_SECURE_NO_WARNINGS
    -D_USE_MATH_DEFINES
    -DGLM_FORCE_SWIZZLE
    -DGLM_ENABLE_EXPERIMENTAL
    -DGLFW_INCLUDE_NONE
    -DEXPERIMENTAL_GLTF
)

# Set MSVC/Windows specific 
if (MSVC OR MSVS OR MSYS OR MINGW)
    set(DEFINES ${DEFINES} -DOS_WIN -DWIN32) 
endif()

# Set Linux specific
if (UNIX OR LINUX)
    set(DEFINES ${DEFINES} -DOS_LNX) 
endif()

# Vulkan Directories 
if (Vulkan_INCLUDE_DIR)
    set (INC_DIR ${INC_DIR} ${Vulkan_INCLUDE_DIR})
endif()

# Vulkan Library 
if (Vulkan_LIBRARY)
    set (LIBS ${LIBS} ${Vulkan_LIBRARY})
endif()

# GLFW 
if (GLFW_LIBRARIES)
    set (LIBS ${LIBS} ${GLFW_LIBRARIES})
    set (INC_DIR ${INC_DIR} ${GLFW_INCLUDE_DIRS})
endif()

# set GCC8 (Linux) libs
# MingW-W64 TODO
if (UNIX OR LINUX)
    find_package(X11 QUIET)
    if (X11_FOUND)
        set (LIBS ${LIBS} ${X11_LIBRARIES})
        set (INC_DIR ${INC_DIR} ${X11_INCLUDE_DIR})
    endif()
    set (LIBS ${LIBS} -static-libgcc -static-libstdc++ -lX11 -lXrandr -lpthread -ldl)
endif()

# Compiler specific options
if (MSVC) # Clang in Visual Studio 2017
    set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /std:c++latest /arch:AVX2 /tune:skylake /fp:strict /permissive- /Qspectre /JMC /Oy /Oi /Ot /Ox /GT /GL /Gy /Qpar /W3 /NODEFAULTLIB:library") # Native compiler of Visual Studio 2017
    set (CMAKE_DOTNET_TARGET_FRAMEWORK_VERSION "v4.7.2")
else()
    set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -m64 -std=c++2a -march=skylake -mtune=skylake -fpermissive -w -Ofast")
    set (CMAKE_C_FLAGS   "${CMAKE_C_FLAGS}   -m64 -std=c1x   -march=skylake -mtune=skylake -fpermissive -w -Ofast")
    if (WIN32)
        set (CMAKE_CXX_FLAGS "${CMAK_CXX_FLAGS} -Wa,-mbig-obj")
        set (CMAKE_C_FLAGS "${CMAK_C_FLAGS} -Wa,-mbig-obj")
    endif()
endif()

# Set optimizations for build types
set (CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O0")
set (CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -O0")

# Set default compiler options
link_directories(${LIB_DIR})
include_directories(${INC_DIR})
file (GLOB RSOURCES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} ${SOURCES_LIST})

# Compute test 
set (APPLICATION_NAME compute-test)
add_definitions(${DEFINES})
add_executable(${APPLICATION_NAME} src/examples/rtvg.cpp src/examples/main.cpp ${RSOURCES})
target_link_libraries(${APPLICATION_NAME} PRIVATE ${LIBS})

# Add includes to MSVC explorer 
foreach(source IN LISTS RSOURCES)
    get_filename_component(source_path "${source}" PATH)
    string(REPLACE "/" "\\" source_path_msvc "${source_path}")
    source_group("${source_path_msvc}" FILES "${source}")
endforeach()
