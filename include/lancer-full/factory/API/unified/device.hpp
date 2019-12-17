#pragma once

#include "./core/unified/core.hpp"

namespace svt {
    namespace api {
        namespace factory {

            class device_t { public: 
                core::api::device_t device;
                std::vector<uint32_t> queueFamilyIndices = {};

                device_t(const device_t& device_t) : device(device_t) {};
                device_t(const core::api::device_t& device) : device(device) {};
                device_t& operator=(const device_t& device) { this->device = device; return *this; };

                operator core::api::device_t&() { return device; };
                operator const core::api::device_t&() const { return device; };

                core::api::device_t* operator->() { return &(this->device); };
                const core::api::device_t* operator->() const { return &(this->device); };
            };

        };
    };
};
