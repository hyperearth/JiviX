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

# 
set(DCMAKE_SH="CMAKE_SH-NOTFOUND")
if (MINGW)
    set(CMAKE_SHARED_LIBRARY_PREFIX "")
    set(CMAKE_STATIC_LIBRARY_PREFIX "")
    set(GNUtoMS TRUE)
endif()

# Set source directories
set (SOURCES_LIST 
    "./include/JiviX/*.hpp"
    "./include/JiviX/*/*.hpp"
    "./include/JiviX/*/vulkan.*"
    "./include/JiviX/*/*/*.hpp"
    "./include/JiviX/*/*/vulkan.*"
    "./include/JiviX/*/*/*/*.hpp"
    "./include/JiviX/*/*/*/vulkan.*"
    "./include/JiviX/*/*/*/*/*.hpp"
    "./include/JiviX/*/*/*/*/vulkan.*"

    "./include/misc/*.hpp"
    "./include/misc/*.h"
    "./include/nlohmann/*.hpp"
    "./include/nlohmann/*.h"
    "./include/vulkan/*.hpp"
    "./include/vulkan/*.h"
    "./include/vma/*.hpp"
    "./include/vma/*.h"
    "./include/vkh/*.hpp"
    "./include/vkh/*.h"
    "./include/xvk/*.hpp"
    "./include/xvk/*.h"
    "./include/vkt3/*.hpp"
    "./include/vkt3/*.h"
    "./include/volk/*.hpp"
    "./include/volk/*.h"
    "./include/volk/*.c"
    "./include/*.hpp"
    "./include/*.h"
    "./include/tinyobjloader/*.h"
    "./include/tinyobjloader/tiny_obj_loader.cc"

    "./src/*.cpp"
    "./src/*.c"
    "./src/*/*.hpp"
    "./src/*/*.h"
    "./src/*/*/*.cpp"
    "./src/*/*/*.c"
    "./src/*/*.cpp"
    "./src/*/*.c"
    
    #"./src/include/*/*.hpp"
    #"./src/include/*/*.h"
    #"./src/include/*.hpp"
    #"./src/include/*.h"
    #"./src/*/*/*.hpp"
    #"./src/*/*/*.h"
    #"./src/*.hpp"
    #"./src/*.h"
)

# Find packages (Linux Prior)
find_package(GLFW QUIET)
find_package(Vulkan REQUIRED)
find_package(PkgConfig QUIET)
#find_package(glbinding REQUIRED) # if it is really required in your project

set (LIBS )

# Set libs and include dirs of projects
set (LIB_DIR ${PROJECT_SOURCE_DIR} "${DEP_DIR}/lib")
set (INC_DIR "../../../vkt/include/" "../../../vkt/include/rapidjson"
    "${DEP_DIR}/include/JiviX/"
    "${DEP_DIR}/include/*/*/"
    "${DEP_DIR}/include/*/"
    "${DEP_DIR}/include/" 
    "${DEP_DIR}/include/volk/"
    "${DEP_DIR}/include/vkt3/"
    "${DEP_DIR}/include/misc/"
    "${DEP_DIR}/include/nlohmann/"
    "${DEP_DIR}/src/" 
    "${DEP_DIR}/src/base/"
    "${DEP_DIR}")

# Set definition
set (DEFINES
    -DNOMINMAX
    #-DENABLE_OPENGL_INTEROP
    -DTINYGLTF_NOEXCEPTION
    -DVK_ENABLE_BETA_EXTENSIONS
    -D_CRT_SECURE_NO_WARNINGS
    -D_USE_MATH_DEFINES
    -DGLM_FORCE_SWIZZLE
    -DGLM_FORCE_RADIANS
    -DGLM_ENABLE_EXPERIMENTAL
    -DVMA_DEDICATED_ALLOCATION
    -DGLFW_INCLUDE_NONE
    -DEXPERIMENTAL_GLTF
    -DUSE_VULKAN
    #-DTINYGLTF_USE_RAPIDJSON
    -DTINYGLTF_USE_CPP14
    -DENABLE_VULKAN_HPP
    -DVKT_CORE_ENABLE_XVK
    -DVKT_CORE_ENABLE_VMA
    -DWIN32_LEAN_AND_MEAN 
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
#if (Vulkan_INCLUDE_DIR)
#    set (INC_DIR ${INC_DIR} ${Vulkan_INCLUDE_DIR})
#endif()

# Vulkan Library 
if (Vulkan_LIBRARY)
    #set (LIBS ${LIBS} ${Vulkan_LIBRARY}) #Da poshlo ono... 
endif()


# GLFW 
if (GLFW_LIBRARIES)
    #set (LIBS ${LIBS} ${GLFW_LIBRARIES})
    set (INC_DIR ${INC_DIR} ${GLFW_INCLUDE_DIRS})
endif()

# glbinding
#if (glbinding_DIR)
#    set (INC_DIR ${INC_DIR} ${glbinding_DIR} "${glbinding_DIR}/include")
#endif()

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
    #set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /diagnostics:caret /std:c++latest /arch:AVX2 /tune:skylake /fp:strict /permissive- /Qspectre /JMC /Oy /Oi /Ot /Ox /GT /GL /Gy /Qpar /W3") # Native compiler of Visual Studio 2017
    set (LIBS ${LIBS} glfw3)
    set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /diagnostics:caret /std:c++latest /arch:AVX2 /fp:strict /permissive- /Qspectre /JMC /Ox /Qpar /W3")
    set (CMAKE_DOTNET_TARGET_FRAMEWORK_VERSION "v4.7.2")
else()
    set (COMMON_LIBS glfw X11 Xrandr rt dl)
    set (LIBS ${LIBS} ${COMMON_LIBS})

    set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wmissing-field-initializers -stdlib=libc++ -lc++ -lc++abi -lm -lc -lgcc_s -lgcc -g -Wall -std=c++2a -march=skylake -mtune=skylake -fpermissive")
    set (CMAKE_C_FLAGS   "${CMAKE_C_FLAGS}   -Wmissing-field-initializers -stdlib=libc                                                            -march=skylake -mtune=skylake -fpermissive")
endif()

# Set optimizations for build types
set (CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O0 -g")
set (CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -O0 -g")

# Set default compiler options
link_directories(${LIB_DIR})
include_directories(${INC_DIR})
file (GLOB RSOURCES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} ${SOURCES_LIST})

# Compute test 
set (APPLICATION_NAME test-app)
add_definitions(${DEFINES})
add_executable(${APPLICATION_NAME} ${RSOURCES} src/main.cpp src/pch.cpp)
target_link_libraries(${APPLICATION_NAME} PRIVATE ${LIBS} Vulkan::Vulkan)
target_precompile_headers(${APPLICATION_NAME} PRIVATE src/pch.hpp src/pch.hpp src/pch.cpp)

# Add includes to MSVC explorer 
foreach(source IN LISTS RSOURCES)
    get_filename_component(source_path "${source}" PATH)
    string(REPLACE "/" "\\" source_path_msvc "${source_path}")
    source_group("${source_path_msvc}" FILES "${source}")
endforeach()

