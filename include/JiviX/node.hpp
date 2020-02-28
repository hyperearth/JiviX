#pragma once

#include "core.hpp"
#include "JVI/node.hpp"

namespace jvx { 
    class Node : public Wrap<jvi::Node> { public:
        Node(const Context& context) { object = std::make_shared<jvi::Node>(context); };
    };
};
