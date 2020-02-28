#pragma once

#include <memory>
#include "JVI/renderer.hpp"

namespace jvx { 
    class Renderer {
    public:
        Renderer() {};
        Renderer(const Renderer& renderer) : object(renderer.object) {};
        Renderer(const std::shared_ptr<jvi::Renderer>& object) : object(object) {};

        // 
        virtual Renderer& operator=(const std::shared_ptr<jvi::Renderer>& object) { this->object = object; return *this; };
        virtual Renderer& operator=(const Renderer& object) { this->object = object.object; return *this; };

        // 
        virtual operator std::shared_ptr<jvi::Renderer>& () { return object; };
        virtual operator const std::shared_ptr<jvi::Renderer>& () const { return object; };

        // 
        virtual jvi::Renderer* operator->() { return &(*object); };
        virtual jvi::Renderer& operator*() { return (*object); };
        virtual const jvi::Renderer* operator->() const { return &(*object); };
        virtual const jvi::Renderer& operator*() const { return (*object); };

    protected: friend jvx::Renderer; friend jvi::Renderer; // 
             std::shared_ptr<jvi::Renderer> object = {};
    };
};
