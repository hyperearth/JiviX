#pragma once

#include "./core/unified/core.hpp"

namespace svt {
    namespace api {
        namespace factory {

            class physical_device_t : public std::enable_shared_from_this<physical_device_t> { public: 
                core::api::physical_device_t physical_device_ = API_NULL_HANDLE;
                core::api::physical_device_properties_t properties_ = {};
                core::api::physical_device_features_t features_ = {};

                physical_device_t() {};
                physical_device_t(const physical_device_t& physical_device) : physical_device_(physical_device) {};
                physical_device_t(const core::api::physical_device_t& physical_device_) : physical_device_(physical_device_) {};
                physical_device_t& operator=(const physical_device_t& physical_device) { this->physical_device_ = physical_device; return *this; };

                operator uintptr_t&() { return (uintptr_t&)(physical_device_); };
                operator const uintptr_t&() const { return (uintptr_t&)(physical_device_); };
                operator core::api::physical_device_t&() { return physical_device_; };
                operator const core::api::physical_device_t&() const { return physical_device_; };

                core::api::physical_device_t* operator->() { return &(this->physical_device_); };
                const core::api::physical_device_t* operator->() const { return &(this->physical_device_); };
                core::api::physical_device_t& operator*() { return (this->physical_device_); };
                const core::api::physical_device_t& operator*() const { return (this->physical_device_); };
            };

        };
    };
};
