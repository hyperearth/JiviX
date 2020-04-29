#pragma once

#include "./core.hpp"
#include "./JVI/renderer.hpp"

namespace jvx { 
    class Renderer : public Wrap<jvi::Renderer> { public: using T = jvi::Renderer;
        Renderer() {};
        Renderer(vkt::uni_ptr<jvi::Renderer> obj) { this->object = obj; };
        Renderer(vkt::uni_arg<jvx::Context> context) { this->object = std::make_shared<jvi::Renderer>(*context); };
        //Renderer(jvx::Context context) { this->object = std::make_shared<jvi::Renderer>(context); };
        Renderer(std::shared_ptr<jvi::Renderer> obj) { this->object = obj; };

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
