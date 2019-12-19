#pragma once

#include "./classes/API/types.hpp"
#include "./factory/API/unified/allocator.hpp"
#include "./factory/API/unified/allocation.hpp"

namespace svt {
    namespace api {
        namespace classes {
            class allocation { public: 
            protected: friend allocation;
                stu::allocation allocation_ = {};
                stu::device device_ = {};
                stu::allocator allocator_ = {};
            };
        };
    };
};
