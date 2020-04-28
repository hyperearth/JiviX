#pragma once

#include "./core.hpp"
#include "./JVI/context.hpp"
#include "./JVI/material.hpp"

namespace jvx { 
    class Material : public Wrap<jvi::Material> { public: using T = jvi::Material;
        Material() {};
        Material(const vkt::uni_ptr<jvi::Material>& obj) { this->object = obj; };
        Material(const vkt::uni_arg<jvx::Context>& context) { object = std::make_shared<jvi::Material>(*context); };
        Material(const jvx::Context& context) { object = std::make_shared<jvi::Material>(context); };

        CALLIFY(sharedPtr);
        CALLIFY(setContext);
        CALLIFY(setRawMaterials);
        CALLIFY(setGpuMaterials);
        CALLIFY(pushMaterial);
        CALLIFY(resetMaterials);
        CALLIFY(resetSampledImages);
        CALLIFY(pushSampledImage);
        CALLIFY(copyBuffers);
        CALLIFY(createDescriptorSet);
    };
};
