#pragma once

#include "./core.hpp"
#include "./JVI/renderer.hpp"

namespace jvx { 
    class Renderer : public Wrap<jvi::Renderer> { public: using T = jvi::Renderer;
        Renderer(const Context& context) { object = std::make_shared<jvi::Renderer>(context); };
    };
};
