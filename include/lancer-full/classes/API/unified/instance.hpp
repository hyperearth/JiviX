#pragma once

#include "./classes/API/types.hpp"
#include "./factory/API/unified/instance.hpp"

namespace svt {
    namespace api {
        namespace classes {
            class instance { public: 
                instance(const instance& instance) : instance_(instance) {};
                instance(const stu::instance_t& instance_ = {}) : instance_(instance_) {};
                instance& operator=(const instance &instance) { this->instance_ = instance; return *this; };

                // TODO: move into `.cpp` file
                // TODO: add assigment by core types and shared_ptr types
                operator stu::instance&() { return instance_; };
                operator stu::instance_t&() { return instance_; };
                operator const stu::instance&() const { return instance_; };
                operator const stu::instance_t&() const { return instance_; };

                api::factory::instance_t* operator->() { return &(*this->instance_); };
                const api::factory::instance_t* operator->() const { return &(*this->instance_); };
                api::factory::instance_t& operator*() { return (*this->instance_); };
                const api::factory::instance_t& operator*() const { return (*this->instance_); };

                // 
                operator uintptr_t&() { return instance_; };
                operator const uintptr_t&() const { return instance_; };

                operator core::api::instance_t&() { return instance_; };
                operator const core::api::instance_t&() const { return instance_; };

            protected: friend instance;
                stu::instance_t instance_ = {};
            };
        };
    };
};
