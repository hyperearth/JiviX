#pragma once
#include "JVI/mesh.hpp"

namespace jvx { 
    class Mesh { public: 
        Mesh() {};
        Mesh(const Mesh& mesh): object(mesh.object) {};
        Mesh(const std::shared_ptr<jvi::Mesh>& object): object(object) {};

        // 
        virtual Mesh& operator=(const std::shared_ptr<jvi::Mesh>& object) { this->object = object; return *this; };
        virtual Mesh& operator=(const Mesh& object) { this->object = object.object; return *this; };

        // 
        virtual operator std::shared_ptr<jvi::Mesh>&() { return object; };
        virtual operator const std::shared_ptr<jvi::Mesh>&() const { return object; };

        // 
        virtual jvi::Mesh* operator->() { return &(*object); };
        virtual jvi::Mesh& operator*() { return (*object); };
        virtual const jvi::Mesh* operator->() const { return &(*object); };
        virtual const jvi::Mesh& operator*() const { return (*object); };

    protected: friend jvx::Mesh; friend jvi::Mesh; // 
        std::shared_ptr<jvi::Mesh> object = {};
    }
};
