#pragma once // #

#include <memory>
#include <chrono>

#ifdef ENABLE_OPENGL_INTEROP
#include <glad/glad.h>
#endif

#include <vkt2/fw.hpp>

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

#define uPTR(NAME) vkt::uni_ptr<NAME>
#ifdef NATIVE_SHARED_PTR
//#define uPTR(NAME) std::shared_ptr<NAME>
#define uTHIS shared_from_this()
#else // For Java Applications compatibility
//#define uPTR(NAME) NAME*
#define uTHIS this
#endif

    struct MeshInfo {
        uint32_t materialID = 0u;
        uint32_t indexType = 0u;
        uint32_t prmCount = 0u;
        //uint32_t flags = 0u;
        uint32_t
            hasTransform : 1,
            hasNormal : 1,
            hasTexcoord : 1,
            hasTangent : 1;
    };

    constexpr uint32_t DEFAULT_STRIDE = 80u;

};
