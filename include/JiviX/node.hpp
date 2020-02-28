#pragma once
#include "JVI/node.hpp"

namespace jvx { 
    class Node {
    public:
        Node() {};
        Node(const Node& node) : object(node.object) {};
        Node(const std::shared_ptr<jvi::Node>& object) : object(object) {};

        // 
        virtual Node& operator=(const std::shared_ptr<jvi::Node>& object) { this->object = object; return *this; };
        virtual Node& operator=(const Node& object) { this->object = object.object; return *this; };

        // 
        virtual operator std::shared_ptr<jvi::Node>& () { return object; };
        virtual operator const std::shared_ptr<jvi::Node>& () const { return object; };

        // 
        virtual jvi::Node* operator->() { return &(*object); };
        virtual jvi::Node& operator*() { return (*object); };
        virtual const jvi::Node* operator->() const { return &(*object); };
        virtual const jvi::Node& operator*() const { return (*object); };

    protected: friend jvx::Node; friend jvi::Node; // 
             std::shared_ptr<jvi::Node> object = {};
    };
};
