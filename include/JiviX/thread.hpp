#pragma once

#include "./core.hpp"
#include "./JVI/thread.hpp"
//#include "./JVI/context.hpp"

namespace jvx { 
    class Thread : public Wrap<jvi::Thread> { public: using T = jvi::Thread;
        Thread() {};
        Thread(vkt::uni_ptr<jvi::Thread> obj) { this->object = obj; };
        //Thread(vkt::uni_arg<jvx::Thread> obj) { this->object = *obj; };
        Thread(vkt::uni_arg<jvx::Driver> driver) { this->object = std::make_shared<jvi::Thread>(*driver); };
        //Thread(const jvx::Driver& driver) { this->object = std::make_shared<jvi::Thread>(driver); };
        Thread(std::shared_ptr<jvi::Thread> obj) { this->object = obj; };
        Thread(std::shared_ptr<jvi::Driver> driver) { this->object = std::make_shared<jvi::Thread>(driver); };

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
