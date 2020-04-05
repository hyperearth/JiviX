#pragma once

#include "./core.hpp"
#include "./driver.hpp"
#include "./JVI/context.hpp"
#include "./JVI/driver.hpp"

namespace jvx { 
    class Context : public Wrap<jvi::Context> { public: using T = jvi::Context;
        Context(Driver driver) { object = std::make_shared<jvi::Context>(driver); };
    };
};
