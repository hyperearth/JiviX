#pragma once

#include "./core.hpp"
#include "./context.hpp"
#include "./JVI/context.hpp"
#include "./JVI/mesh-binding.hpp"
#include "./JVI/mesh-input.hpp"

namespace jvx { 
    class BufferViewSet : public Wrap<jvi::BufferViewSet> {
    public: using T = jvi::BufferViewSet;
        BufferViewSet() {};
        BufferViewSet(vkt::uni_ptr<jvi::BufferViewSet> obj) { this->object = obj; };
        BufferViewSet(vkt::uni_arg<jvx::Context> context) { this->object = std::make_shared<jvi::BufferViewSet>(*context); };
        //BufferViewSet(jvx::Context context) { this->object = std::make_shared<jvi::BufferViewSet>(context); };

        CALLIFY(sharedPtr);
        CALLIFY(setContext);
        CALLIFY(resetBufferViews);
        CALLIFY(pushBufferView);
        CALLIFY(get);
        CALLIFY(getBufferViewList);
        CALLIFY(getDescriptorSet);
        CALLIFY(createDescriptorSet);
        CALLIFY(getDescriptorLayout);
        CALLIFY(getBufferCount);
    };

    class MeshBinding : public Wrap<jvi::MeshBinding> {
    public: using T = jvi::MeshBinding;
        MeshBinding() {};
        MeshBinding(vkt::uni_ptr<jvi::MeshBinding> obj) { this->object = obj; };
        MeshBinding(vkt::uni_arg<jvx::Context> context, vk::DeviceSize MaxPrimitiveCount = jvi::MAX_PRIM_COUNT, std::vector<vk::DeviceSize> GeometryInitial = {}) { this->object = std::make_shared<jvi::MeshBinding>(*context, MaxPrimitiveCount, GeometryInitial); };
        //MeshBinding(jvx::Context context, vk::DeviceSize MaxPrimitiveCount = jvi::MAX_PRIM_COUNT, std::vector<vk::DeviceSize> GeometryInitial = {}) { this->object = std::make_shared<jvi::MeshBinding>(context, MaxPrimitiveCount, GeometryInitial); };

        CALLIFY(sharedPtr);
        CALLIFY(setThread);
        CALLIFY(setDriver);
        CALLIFY(getBindingBuffer);
        CALLIFY(getIndexBuffer);
        CALLIFY(getBindingMemoryHandle);
        CALLIFY(getIndexMemoryHandle);
        CALLIFY(manifestIndex);
        CALLIFY(setIndexCount);
        CALLIFY(setPrimitiveCount);
        CALLIFY(linkWithInstance);
        CALLIFY(increaseGeometryCount);
        CALLIFY(setGeometryCount);
        CALLIFY(setTransformData);
        CALLIFY(setBinding);
        CALLIFY(setAttribute);
        CALLIFY(copyBuffers);
        CALLIFY(buildGeometry);
        CALLIFY(buildAccelerationStructure);
        CALLIFY(addMeshInput);
        CALLIFY(createAccelerationStructure);
        CALLIFY(createRasterizePipeline);
        CALLIFY(createRasterizeCommand);

#ifdef ENABLE_OPENGL_INTEROP //
        CALLIFY(getIndexBufferGL);
        CALLIFY(getBindingBufferGL);
#endif

    };

    class MeshInput : public Wrap<jvi::MeshInput> {
    public: using T = jvi::MeshInput;
        MeshInput() {};
        MeshInput(vkt::uni_ptr<jvi::MeshInput> obj) { this->object = obj; };
        MeshInput(vkt::uni_arg<jvx::Context> context) { this->object = std::make_shared<jvi::MeshInput>(*context); };
        //MeshInput(jvx::Context context) { this->object = std::make_shared<jvi::MeshInput>(context); };

        CALLIFY(sharedPtr);
        CALLIFY(makeQuad);
        CALLIFY(createDescriptorSet);
        CALLIFY(copyMeta);
        CALLIFY(buildGeometry);
        //CALLIFY(getOffsetMeta);
        CALLIFY(addBinding);
        CALLIFY(manifestIndex);
        CALLIFY(addAttribute);
        CALLIFY(setIndexOffset);
        CALLIFY(setIndexCount);
        CALLIFY(setPrimitiveCount);
        CALLIFY(setIndexData);
        CALLIFY(createRasterizePipeline);
        CALLIFY(linkCounterBuffer);
        CALLIFY(linkBViewSet);
        CALLIFY(getIndexCount);
        
    };
};
