#pragma once

#include "./core.hpp"
#include "./context.hpp"
#include "./JVI/context.hpp"
#include "./JVI/mesh-binding.hpp"
#include "./JVI/mesh-input.hpp"

namespace jvx { 
    class BufferViewSet : public Wrap<jvi::BufferViewSet> {
    public: using T = jvi::BufferViewSet;
        BufferViewSet(Context context) { object = std::make_shared<jvi::BufferViewSet>(context); };
    };

    class MeshBinding : public Wrap<jvi::MeshBinding> {
    public: using T = jvi::MeshBinding;
        MeshBinding(Context context, vk::DeviceSize MaxPrimitiveCount = jvi::MAX_PRIM_COUNT, vk::DeviceSize MaxStride = jvi::DEFAULT_STRIDE) { object = std::make_shared<jvi::MeshBinding>(context, MaxPrimitiveCount, MaxStride); };
    };

    class MeshInput : public Wrap<jvi::MeshInput> {
    public: using T = jvi::MeshInput;
        MeshInput(Context context) { object = std::make_shared<jvi::MeshInput>(context); };
    };
};
