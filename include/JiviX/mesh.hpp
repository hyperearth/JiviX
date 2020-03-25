#pragma once

#include "./core.hpp"
#include "./JVI/mesh-binding.hpp"
#include "./JVI/mesh-input.hpp"

namespace jvx { 
    class MeshBinding : public Wrap<jvi::MeshBinding> { 
    public: using T = jvi::MeshBinding;
        MeshBinding(const Context& context, vk::DeviceSize AllocationUnitCount = 32768, vk::DeviceSize MaxStride = jvi::DEFAULT_STRIDE) { object = std::make_shared<jvi::MeshBinding>(context, AllocationUnitCount, MaxStride); };
    };

    class MeshInput : public Wrap<jvi::MeshInput> {
    public: using T = jvi::MeshInput;
        MeshInput(const Context& context) { object = std::make_shared<jvi::MeshInput>(context); };
    };
};
