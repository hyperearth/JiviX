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
        BufferViewSet(std::shared_ptr<jvi::BufferViewSet> obj) { this->object = obj; };

        CALLIFY(sharedPtr);
        CALLIFY(setContext);
        CALLIFY(resetBufferViews);
        CALLIFY(pushBufferView);
        CALLIFY(get);
        CALLIFY(getBufferViewList);
        CALLIFY(getDescriptorSet);
        CALLIFY(getDescriptorLayout);
        CALLIFY(getBufferCount);
    };

    class MeshInput : public Wrap<jvi::MeshInput> {
    public: using T = jvi::MeshInput;
        MeshInput() {};
        MeshInput(vkt::uni_ptr<jvi::MeshInput> obj) { this->object = obj; };
        MeshInput(vkt::uni_arg<jvx::Context> context) { this->object = std::make_shared<jvi::MeshInput>(*context); };
        MeshInput(std::shared_ptr<jvi::MeshInput> obj) { this->object = obj; };

        CALLIFY(sharedPtr);
        CALLIFY(makeQuad);
        CALLIFY(copyMeta);
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
        //CALLIFY(getOffsetMeta);

        vkt::uni_ptr<jvi::MeshInput> linkBViewSet(vkt::uni_arg<jvx::BufferViewSet> node) {
            return object->linkBViewSet(node->sharedPtr());
        };
        
    };

    class MeshBinding : public Wrap<jvi::MeshBinding> {
    public: using T = jvi::MeshBinding;
          MeshBinding() {};
          MeshBinding(vkt::uni_ptr<jvi::MeshBinding> obj) { this->object = obj; };
          MeshBinding(vkt::uni_arg<jvx::Context> context, vk::DeviceSize MaxPrimitiveCount = jvi::MAX_PRIM_COUNT, std::vector<vk::DeviceSize> GeometryInitial = {}) { this->object = std::make_shared<jvi::MeshBinding>(*context, MaxPrimitiveCount, GeometryInitial); };
          MeshBinding(vkt::uni_arg<jvx::Context> context, vk::DeviceSize MaxPrimitiveCount, std::vector<int64_t> GeometryInitial) { this->object = std::make_shared<jvi::MeshBinding>(*context, MaxPrimitiveCount, GeometryInitial); };
          MeshBinding(std::shared_ptr<jvi::MeshBinding> obj) { this->object = obj; };

          CALLIFY(sharedPtr);
          CALLIFY(setThread);
          CALLIFY(setDriver);
          CALLIFY(getBindingBuffer);
          CALLIFY(getIndexBuffer);
#ifdef ENABLE_OPENGL_INTEROP
          CALLIFY(getBindingMemoryHandle);
          CALLIFY(getIndexMemoryHandle);
#endif
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
          CALLIFY(addMeshInput);
          CALLIFY(createAccelerationStructure);
          CALLIFY(createRasterizePipeline);
          CALLIFY(createRasterizeCommand);
          CALLIFY(addRangeInput);
          CALLIFY(resetGeometry);

#ifdef ENABLE_OPENGL_INTEROP //
          CALLIFY(getIndexBufferGL);
          CALLIFY(getBindingBufferGL);
#endif

          vkt::uni_ptr<jvi::MeshBinding> addMeshInput(vkt::uni_arg<jvx::MeshInput> mesh) {
              return object->addMeshInput(mesh->sharedPtr());
          };
    };

};
