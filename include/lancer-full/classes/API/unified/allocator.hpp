#pragma once

#include "./classes/API/types.hpp"
#include "./factory/API/unified/allocator.hpp"

namespace svt {
    namespace api {
        namespace classes {
            // Agregated type from `allocator_t`, and can be created by dedicated utils (such as VMA)
            class allocator {
                protected: friend allocator;
                    stu::allocator allocator_ = {};
                    stu::device device_ = {};

                public: 
                    allocator(const allocator& allocator) : device_(allocator), allocator_(allocator) {};
                    allocator(                            const stu::allocator& allocator_ = {}) : allocator_(allocator_) {};
                    allocator(const stu::device& device_, const stu::allocator& allocator_ = {}) : allocator_(allocator_), device_(device_) {};

                    // TODO: move into `.cpp` file
                    operator stu::allocator&() { return allocator_; };
                    operator stu::device&() { return device_; };
                    operator const stu::allocator&() const { return allocator_; };
                    operator const stu::device&() const { return device_; };

                    // TODO: move into `.cpp` file
                    allocator& operator=(const allocator &allocator) { 
                        this->allocator_ = allocator;
                        this->device_ = allocator;
                        return *this;
                    };

                    // TODO: move into `.cpp` file
                    api::factory::allocator_t* operator->() { return &(*this->allocator_); };
                    const api::factory::allocator_t* operator->() const { return &(*this->allocator_); };
            };
        };
    };
};
