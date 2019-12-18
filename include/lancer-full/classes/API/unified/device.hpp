#pragma once

#include "./classes/API/types.hpp"
#include "./factory/API/unified/device.hpp"

namespace svt {
    namespace api {
        namespace classes {
            class device { public: 
                device(const device& device) : device_(device) {};
                device(const stu::device& device_ = {}) : device_(device_) {};
                device& operator=(const device &device) { this->device_ = device; return *this; };

                operator stu::device&() { return device_; };
                operator const stu::device&() const { return device_; };

                api::factory::device_t* operator->() { return &(*this->device_); };
                const api::factory::device_t* operator->() const { return &(*this->device_); };

            protected: 
                stu::device device_ = {};
            };
        };
    };
};
