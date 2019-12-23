#pragma once

#include "./core/unified/core.hpp"
#include "./factory/API/types.hpp"

namespace svt {
    namespace api {
        namespace factory {
            class descriptor_pool_t : public std::enable_shared_from_this<descriptor_pool_t> { public: 
                core::api::descriptor_pool_t pool_ = API_NULL_HANDLE;

                descriptor_pool_t() {};
                descriptor_pool_t(const descriptor_pool_t& descriptor_pool_t) : pool_(descriptor_pool_t) {};
                descriptor_pool_t(core::api::descriptor_pool_t pool_ = API_NULL_HANDLE) : pool_(pool_) {};
                descriptor_pool_t& operator=(const descriptor_pool_t& descriptor_pool_t) { pool_ = descriptor_pool_t; return *this; };

                operator uintptr_t&() { return (uintptr_t&)(pool_); };
                operator const uintptr_t&() const { return (uintptr_t&)(pool_); };
                operator core::api::descriptor_pool_t&() { return pool_; };
                operator const core::api::descriptor_pool_t&() const { return pool_; };

                core::api::descriptor_pool_t* operator->() { return &(this->pool_); };
                const core::api::descriptor_pool_t* operator->() const { return &(this->pool_); };
                core::api::descriptor_pool_t& operator*() { return (this->pool_); };
                const core::api::descriptor_pool_t& operator*() const { return (this->pool_); };
            };
        };
    };
};
