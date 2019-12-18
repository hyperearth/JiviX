#pragma once

#include "./core/unified/core.hpp"

namespace svt {
    namespace api {
        namespace factory {

            class device_t { public: 
                core::api::device_t device_;
                std::vector<uint32_t> queue_family_indices_ = {};

                device_t(const device_t& device) : device_(device) {};
                device_t(const core::api::device_t& device_) : device_(device_) {};
                device_t& operator=(const device_t& device) { this->device_ = device; return *this; };

                operator core::api::device_t&() { return device_; };
                operator const core::api::device_t&() const { return device_; };

                core::api::device_t* operator->() { return &(this->device_); };
                const core::api::device_t* operator->() const { return &(this->device_); };
            };

        };
    };
};
