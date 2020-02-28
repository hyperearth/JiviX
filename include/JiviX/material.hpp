#pragma once

#include <memory>
#include "JVI/material.hpp"

namespace jvx { 
    class Material {
    public:
        Material() {};
        Material(const Material& material) : object(material.object) {};
        Material(const std::shared_ptr<jvi::Material>& object) : object(object) {};

        // 
        virtual Material& operator=(const std::shared_ptr<jvi::Material>& object) { this->object = object; return *this; };
        virtual Material& operator=(const Material& object) { this->object = object.object; return *this; };

        // 
        virtual operator std::shared_ptr<jvi::Material>& () { return object; };
        virtual operator const std::shared_ptr<jvi::Material>& () const { return object; };

        // 
        virtual jvi::Material* operator->() { return &(*object); };
        virtual jvi::Material& operator*() { return (*object); };
        virtual const jvi::Material* operator->() const { return &(*object); };
        virtual const jvi::Material& operator*() const { return (*object); };

    protected: friend jvx::Material; friend jvi::Material; // 
             std::shared_ptr<jvi::Material> object = {};
    };
};
