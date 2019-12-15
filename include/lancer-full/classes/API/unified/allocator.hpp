#pragma once

#include "./classes/API/types.hpp"
#include "./factory/API/unified/allocator.hpp"

namespace svt {
    namespace api {
        namespace classes {
            // Agregated type from `allocator_t`, and can be created by dedicated utils (such as VMA)
            class allocator {
                protected: friend allocator;
                    stu::allocator allocator_t = {};
                    stu::device device_t = {};

                public: 
                    allocator(const allocator& allocator_t) : device_t(allocator_t), allocator_t(allocator_t) {};
                    allocator(                           const stu::allocator& allocator_t = {}) : allocator_t(allocator_t) {};
                    allocator(const stu::device& device, const stu::allocator& allocator_t = {}) : allocator_t(allocator_t), device_t(device) {};

                    // TODO: merge into `.cpp`
                    operator stu::allocator&() { return allocator_t; };
                    operator stu::device&() { return device_t; };
                    operator const stu::allocator&() const { return allocator_t; };
                    operator const stu::device&() const { return device_t; };

                    // assign mode 
                    // TODO: move into `.cpp` file
                    allocator& operator=(const allocator &allocator) { 
                        this->allocator_t = allocator;
                        this->device_t = allocator;
                        return *this;
                    };

                    // TODO: move into `.cpp` file
                    api::factory::allocator_t* operator->() { return &(*this->allocator_t); };
                    const api::factory::allocator_t* operator->() const { return &(*this->allocator_t); };
            };
        };
    };
};
