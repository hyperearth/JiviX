#pragma once

#include "./core.hpp"
#include "./JVI/context.hpp"
#include "./JVI/material.hpp"

namespace jvx { 
    class Material : public Wrap<jvi::Material> { public: using T = jvi::Material;
        Material() {};
        Material(vkt::uni_ptr<jvi::Material> obj) { this->object = obj; };
        Material(vkt::uni_arg<jvx::Context> context) { this->object = std::make_shared<jvi::Material>(*context); };
        //Material(jvx::Context context) { this->object = std::make_shared<jvi::Material>(context); };
        Material(std::shared_ptr<jvi::Material> obj) { this->object = obj; };

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
