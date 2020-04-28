#pragma once

#include "./core.hpp"
#include "./JVI/renderer.hpp"

namespace jvx { 
    class Renderer : public Wrap<jvi::Renderer> { public: using T = jvi::Renderer;
        Renderer() {};
        Renderer(const vkt::uni_ptr<jvi::Renderer>& obj) { this->object = obj; };
        Renderer(const vkt::uni_arg<jvx::Context>& context) { this->object = std::make_shared<jvi::Renderer>(*context); };
        Renderer(const jvx::Context& context) { this->object = std::make_shared<jvi::Renderer>(context); };

        CALLIFY(sharedPtr);
        CALLIFY(linkMaterial);
        CALLIFY(linkNode);
        CALLIFY(setupSkyboxedCommand);
        CALLIFY(saveDiffuseColor);
        CALLIFY(setupResamplingPipeline);
        CALLIFY(setupResampleCommand);
        CALLIFY(setupRenderer);
        CALLIFY(setupCommands);
        CALLIFY(refCommandBuffer);
    };
};
