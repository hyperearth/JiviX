#pragma once // #

#include <memory>
#include <chrono>

#ifdef ENABLE_OPENGL_INTEROP
//#include <glad/glad.h>
//#include <glbinding/glbinding.h>
#endif

#include <vkt3/core.hpp>
#include <vkt3/fw.hpp>

namespace jvi {

    //class Driver;
    class Thread;
    //class Mesh;
    class Node;
    class Renderer;
    class Material;
    class Context;
    class MeshInput;
    class MeshBinding;
    class BufferViewSet;

#define uPTR(NAME) vkt::uni_ptr<NAME>
#ifdef NATIVE_SHARED_PTR
//#define uPTR(NAME) std::shared_ptr<NAME>
#define uTHIS shared_from_this()
#else // For Java Applications compatibility
//#define uPTR(NAME) NAME*
#define uTHIS this
#endif

    struct MeshInfo {
        //uint32_t materialID = 0u;
        uint32_t geometryCount = 0u;
        uint32_t indexType = 0u;
        union {
            uint32_t primitiveCount = 0u;
            uint32_t indexID; // for geometry build
        };

        uint32_t
            hasTransform : 1,
            hasNormal : 1,
            hasTexcoord : 1,
            hasTangent : 1;
    };

    struct CommandOptions {
        uint8_t
            eEnableCopyMeta: 1,
            eEnableBuildGeometry : 1,
            eEnableBuildAccelerationStructure : 1,
            eEnableDenoise : 1,
            eEnableRasterization : 1,
            eEnableRayTracing : 1,
            eEnableResampling: 1
            ;
    };

    constexpr uint32_t DEFAULT_STRIDE = 80u;
    constexpr uint32_t MAX_VERT_COUNT = 65536u;
    constexpr uint32_t MAX_PRIM_COUNT = 65536u * 3u;

};
