#pragma once
#include "JVI/thread.hpp"

namespace jvx { 
    class Thread {
    public:
        Thread() {};
        Thread(const Thread& thread) : object(thread.object) {};
        Thread(const std::shared_ptr<jvi::Thread>& object) : object(object) {};

        // 
        virtual Thread& operator=(const std::shared_ptr<jvi::Thread>& object) { this->object = object; return *this; };
        virtual Thread& operator=(const Thread& object) { this->object = object.object; return *this; };

        // 
        virtual operator std::shared_ptr<jvi::Thread>& () { return object; };
        virtual operator const std::shared_ptr<jvi::Thread>& () const { return object; };

        // 
        virtual jvi::Thread* operator->() { return &(*object); };
        virtual jvi::Thread& operator*() { return (*object); };
        virtual const jvi::Thread* operator->() const { return &(*object); };
        virtual const jvi::Thread& operator*() const { return (*object); };

    protected: friend jvx::Thread; friend jvi::Thread; // 
             std::shared_ptr<jvi::Thread> object = {};
    };
};
