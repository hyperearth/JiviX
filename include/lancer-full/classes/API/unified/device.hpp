#pragma once

#include "./classes/API/types.hpp"
#include "./factory/API/unified/device.hpp"

namespace svt {
    namespace api {
        namespace classes {
            class device {
                protected: 
                    stu::device device_t = {};
                    
                public: 
                    device(const device& device) : device_t(device) {};
                    device(const stu::device& device_t = {}) : device_t(device_t) {};
                    device& operator=(const device &device) { this->device_t = device; return *this; };

                    operator stu::device&() { return device_t; };
                    operator const stu::device&() const { return device_t; };

                    api::factory::device_t* operator->() { return &(*this->device_t); };
                    const api::factory::device_t* operator->() const { return &(*this->device_t); };
            };
        };
    };
};
