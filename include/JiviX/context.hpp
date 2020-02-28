#pragma once
#include "JVI/context.hpp"

namespace jvx { 
    class Context { public: 
        Context() {};
        Context(const Context& context): object(context.object) {};
        Context(const std::shared_ptr<jvi::Context>& object): object(object) {};
        
        // 
        virtual Context& operator=(const std::shared_ptr<jvi::Context>& object) { this->object = object; return *this; };
        virtual Context& operator=(const Context& object) { this->object = object.object; return *this; };

        // 
        virtual operator std::shared_ptr<jvi::Context>&() { return object; };
        virtual operator const std::shared_ptr<jvi::Context>&() const { return object; };

        // 
        virtual jvi::Context* operator->() { return &(*object); };
        virtual jvi::Context& operator*() { return (*object); };
        virtual const jvi::Context* operator->() const { return &(*object); };
        virtual const jvi::Context& operator*() const { return (*object); };

    protected: friend jvx::Context; friend jvi::Context; // 
        std::shared_ptr<jvi::Context> object = {};
    }
};
