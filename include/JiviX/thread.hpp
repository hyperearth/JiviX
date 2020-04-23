#pragma once

#include "./core.hpp"
#include "./JVI/thread.hpp"
//#include "./JVI/context.hpp"

namespace jvx { 
    class Thread : public Wrap<jvi::Thread> { public: using T = jvi::Thread;
        Thread(Context context) { object = std::make_shared<jvi::Thread>(context); };

        CALLIFY(sharedPtr);

        CALLIFY(createThreadPool);
        CALLIFY(setDriver);
        CALLIFY(createQueue);
        CALLIFY(createCommandPool);
        CALLIFY(createDescriptorPool);
        CALLIFY(getCommandPool);
        CALLIFY(getDescriptorPool);
        CALLIFY(getQueue);
        CALLIFY(getDevice);
        CALLIFY(getDriverPtr);
        CALLIFY(getDriver);
        CALLIFY(submitOnce);
        CALLIFY(submitOnceAsync);
        CALLIFY(submitCmd);
        CALLIFY(submitCmdAsync);
    };
};
