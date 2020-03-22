#pragma once

#include "./core.hpp"
#include "./JVI/mesh.hpp"

namespace jvx { 
    class Mesh : public Wrap<jvi::Mesh> { public: using T = jvi::Mesh;
        Mesh(const Context& context, vk::DeviceSize AllocationUnitCount = 32768, vk::DeviceSize MaxStride = sizeof(glm::vec4)) { object = std::make_shared<jvi::Mesh>(context, AllocationUnitCount, MaxStride); };
    };
};
