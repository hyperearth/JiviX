#pragma once

#include "./core.hpp"
#include "./JVI/node.hpp"

namespace jvx { 
    class Node : public Wrap<jvi::Node> { public: using T = jvi::Node;
        Node() {};
        Node(vkt::uni_ptr<jvi::Node> obj) { this->object = obj; };
        Node(vkt::uni_arg<jvx::Context> context, const uint32_t& MaxInstanceCount = 64u) { this->object = std::make_shared<jvi::Node>(*context, MaxInstanceCount); };
        Node(std::shared_ptr<jvi::Node> obj) { this->object = obj; };

        CALLIFY(sharedPtr);
        CALLIFY(setContext);
        CALLIFY(setThread);
        CALLIFY(setRawInstance);
        CALLIFY(setGpuInstance);
        CALLIFY(pushInstance);
        CALLIFY(mapMeshData);
        CALLIFY(pushMesh);
        CALLIFY(copyMeta);
        CALLIFY(createDescriptorSet);
        CALLIFY(buildAccelerationStructure);
        CALLIFY(createAccelerationStructure);

        uintptr_t pushMesh(vkt::uni_arg<jvx::MeshBinding> binding) {
            return object->pushMesh(binding->sharedPtr());
        };
    };
};
