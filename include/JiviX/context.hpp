#pragma once

#include "core.hpp"
#include "JVI/context.hpp"
#include "JVI/driver.hpp"

namespace jvx { 
    class Context : public Wrap<jvi::Context> { public:
        Context(const Driver& driver) { object = std::make_shared<jvi::Context>(driver); };
    };
};
