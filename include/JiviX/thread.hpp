#pragma once

#include "./core.hpp"
#include "./JVI/thread.hpp"
//#include "./JVI/context.hpp"

namespace jvx { 
    class Thread : public Wrap<jvi::Thread> { public: using T = jvi::Thread;
        Thread(Context context) { object = std::make_shared<jvi::Thread>(context); };
    };
};
