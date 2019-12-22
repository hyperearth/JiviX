#pragma once

#include "./classes/API/types.hpp"
#include "./factory/API/unified/device.hpp"

namespace svt {
    namespace api {
        namespace classes {
            class device { public: 
                device(const device& device) : device_(device) {};
                device(const stu::device_t& device_ = {}) : device_(device_) {};
                device& operator=(const device &device) { this->device_ = device; return *this; };

                operator stu::device&() { return device_; };
                operator stu::device_t&() { return device_; };
                operator const stu::device&() const { return device_; };
                operator const stu::device_t&() const { return device_; };

                api::factory::device_t* operator->() { return &(*this->device_); };
                const api::factory::device_t* operator->() const { return &(*this->device_); };
                api::factory::device_t& operator*() { return (*this->device_); };
                const api::factory::device_t& operator*() const { return (*this->device_); };

                // 
                operator uintptr_t&() { return device_; };
                operator const uintptr_t&() const { return device_; };
                operator core::api::device_t&() { return device_; };
                operator const core::api::device_t&() const { return device_; };

            protected: 
                stu::device_t device_ = {};
            };
        };
    };
};
