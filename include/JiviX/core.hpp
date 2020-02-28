#pragma once

#include <memory>

namespace jvx { 

    template<class T>
    class Wrap { public:
        Wrap() {};
        Wrap(const Wrap<T>& wrap) : object(wrap.object) {};
        Wrap(const std::shared_ptr<T>& object) : object(object) {};

        // 
        virtual Wrap<T>& operator=(const std::shared_ptr<T>& object) { this->object = object; return *this; };
        virtual Wrap<T>& operator=(const Wrap<T>& object) { this->object = object.object; return *this; };

        // 
        virtual operator std::shared_ptr<T>& () { return object; };
        virtual operator const std::shared_ptr<T>& () const { return object; };

        // 
        virtual T* operator->() { return &(*object); };
        virtual T& operator*() { return (*object); };
        virtual const T* operator->() const { return &(*object); };
        virtual const T& operator*() const { return (*object); };

    protected: friend Wrap; friend T; // 
        std::shared_ptr<T> object = {};
    };
    
    // 
    class Material;
    class Renderer;
    class Context;
    class Thread;
    class Driver;
    class Mesh;
    class Node;

};
