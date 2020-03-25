#pragma once

#include "./core.hpp"
#include "./JVI/mesh-binding.hpp"

namespace jvx { 
    class MeshBinding : public Wrap<jvi::MeshBinding> { 
    public: using T = jvi::MeshBinding;
        MeshBinding(const Context& context, vk::DeviceSize AllocationUnitCount = 32768, vk::DeviceSize MaxStride = 80u) { object = std::make_shared<jvi::MeshBinding>(context, AllocationUnitCount, MaxStride); };
    };

    class MeshInput : public Wrap<jvi::MeshInput> {
    public: using T = jvi::MeshInput;
        MeshInput(const Context& context, vk::DeviceSize AllocationUnitCount = 32768, vk::DeviceSize MaxStride = 80u) { object = std::make_shared<jvi::MeshInput>(context, AllocationUnitCount, MaxStride); };
    };
};
