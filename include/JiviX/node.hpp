#pragma once

#include "./core.hpp"
#include "./JVI/node.hpp"

namespace jvx { 
    class Node : public Wrap<jvi::Node> { public: using T = jvi::Node;
        Node(Context context) { object = std::make_shared<jvi::Node>(context); };

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
