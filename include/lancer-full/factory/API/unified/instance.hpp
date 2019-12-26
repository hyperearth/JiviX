#pragma once

#include "./core/unified/core.hpp"

namespace svt {
    namespace api {
        namespace factory {

            class instance_t : public std::enable_shared_from_this<instance_t> { public: 
                core::api::instance_t instance_ = API_NULL_HANDLE;
                //core::api::pipeline_cache_t pipeline_cache_ = API_NULL_HANDLE;
                //std::vector<uint32_t> queue_family_indices_ = {};

                instance_t() {};
                instance_t(const instance_t& instance) : instance_(instance) {};
                instance_t(const core::api::instance_t& instance_) : instance_(instance_) {};
                instance_t& operator=(const instance_t& instance) { this->instance_ = instance; return *this; };
                instance_t& operator=(const std::shared_ptr<instance_t>& instance) { this->instance_ = *instance; return *this; };
                instance_t& operator=(const core::api::instance_t& instance_) { this->instance_ = instance_; return *this; };

                operator uintptr_t&() { return (uintptr_t&)(instance_); };
                operator const uintptr_t&() const { return (uintptr_t&)(instance_); };
                operator core::api::instance_t&() { return instance_; };
                operator const core::api::instance_t&() const { return instance_; };

                core::api::instance_t* operator->() { return &(this->instance_); };
                const core::api::instance_t* operator->() const { return &(this->instance_); };
                core::api::instance_t& operator*() { return (this->instance_); };
                const core::api::instance_t& operator*() const { return (this->instance_); };
            };

        };
    };
};
