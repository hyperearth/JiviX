#pragma once

#include <memory>
#include "./JVI/JiviX.hpp"

namespace jvx { 

    template<class T>
    class Wrap { public:
        Wrap() {};
        Wrap(const Wrap<T>& wrap) : object(wrap.object) {};
        Wrap(std::shared_ptr<T> object) : object(object) {};
        Wrap(T* object) : object(object) {};
        Wrap(T& object) : object(&object) {};

        // 
        virtual Wrap<T>& operator=(vkt::uni_ptr<T> object) { this->object = object; return *this; };
        virtual Wrap<T>& operator=(std::shared_ptr<T> object) { this->object = object; return *this; };
        virtual Wrap<T>& operator=(Wrap<T> object) { this->object = object.object; return *this; };

        // 
        virtual operator T& () { return object.ref(); };
        virtual operator const T& () const { return object.ref(); };

        // 
        virtual operator T* () { return object.ptr(); };
        virtual operator const T* () const { return object.ptr(); };

        // 
        virtual operator std::shared_ptr<T> () { return object.get_shared(); };
        virtual operator std::shared_ptr<T> () const { return object.get_shared(); };

        // 
        virtual operator vkt::uni_ptr<T> () { return object; };
        virtual operator vkt::uni_ptr<T> () const { return object; };

        // 
        virtual T* operator->() { return object.ptr(); };
        virtual T& operator*() { return object.ref(); };
        virtual const T* operator->() const { return object.ptr(); };
        virtual const T& operator*() const { return object.ref(); };

    protected: friend Wrap; friend T; // 
        vkt::uni_ptr<T> object = {};
    };

#define CALLIFY(NAME)\
     template<class... A> inline decltype(auto) NAME(A... args) { return object->NAME(); };\
     template<class... A> inline decltype(auto) NAME(A... args) const { return object->NAME(); }

    // 
    class Material;
    class Renderer;
    class Context;
    class Thread;
    class Driver;
    class Mesh;
    class Node;

};
