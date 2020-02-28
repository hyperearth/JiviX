#pragma once

#include "core.hpp"
#include "JVI/context.hpp"
#include "JVI/material.hpp"

namespace jvx { 
    class Material : public Wrap<jvi::Material> { public:
        Material(const Context& context) { object = std::make_shared<jvi::Material>(context); };
    };
};
