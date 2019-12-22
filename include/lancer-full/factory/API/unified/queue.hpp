#pragma once

#include "./core/unified/core.hpp"

namespace svt {
    namespace api {
        namespace factory {

            class queue_t { public: 
                core::api::queue_t queue_ = API_NULL_HANDLE;
                //std::vector<uint32_t> queue_family_indices_ = {};

                queue_t() {};
                queue_t(const queue_t& queue) : queue_(queue) {};
                queue_t(const core::api::queue_t& queue_) : queue_(queue_) {};
                queue_t& operator=(const queue_t& queue) { this->queue_ = queue; return *this; };

                operator uintptr_t&() { return (uintptr_t&)(queue_); };
                operator const uintptr_t&() const { return (uintptr_t&)(queue_); };
                operator core::api::queue_t&() { return queue_; };
                operator const core::api::queue_t&() const { return queue_; };

                core::api::queue_t* operator->() { return &(this->queue_); };
                const core::api::queue_t* operator->() const { return &(this->queue_); };
                core::api::queue_t& operator*() { return (this->queue_); };
                const core::api::queue_t& operator*() const { return (this->queue_); };
            };

        };
    };
};
