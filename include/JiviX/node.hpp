#pragma once

#include "./core.hpp"
#include "./JVI/node.hpp"

namespace jvx { 
    class Node : public Wrap<jvi::Node> { public: using T = jvi::Node;
        Node() {};
        Node(const vkt::uni_ptr<jvi::Node>& obj) { this->object = obj; };
        Node(const vkt::uni_arg<jvx::Context>& context) { object = std::make_shared<jvi::Node>(*context); };
        Node(const jvx::Context& context) { object = std::make_shared<jvi::Node>(context); };

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
    };
};
