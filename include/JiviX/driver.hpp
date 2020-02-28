#pragma once

#include "core.hpp"
#include "JVI/driver.hpp"

namespace jvx { 
    class Driver : public Wrap<jvi::Driver> { public:
        Driver() { object = std::make_shared<jvi::Driver>(); };
    };
};
