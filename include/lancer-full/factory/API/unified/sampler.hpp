#pragma once

#include "./core/unified/core.hpp"
#include "./factory/API/types.hpp"

namespace svt {
    namespace api {
        namespace factory {
            class sampler_t : public std::enable_shared_from_this<sampler_t> { public: 
                core::api::sampler_t sampler_ = API_NULL_HANDLE;

                operator uintptr_t&() { return (uintptr_t&)(this->sampler_); };
                operator const uintptr_t&() const { return (uintptr_t&)(this->sampler_); };

                sampler_t() {};
                sampler_t(const sampler_t& sampler) : sampler_(sampler) {};
                sampler_t(const core::api::sampler_t& sampler_) : sampler_(sampler_) {};
                sampler_t& operator=(const sampler_t& sampler) { this->sampler_ = sampler; return *this; };

                operator uintptr_t&() { return (uintptr_t&)(sampler_); };
                operator const uintptr_t&() const { return (uintptr_t&)(sampler_); };
                operator core::api::sampler_t&() { return sampler_; };
                operator const core::api::sampler_t&() const { return sampler_; };

                core::api::sampler_t* operator->() { return &(this->sampler_); };
                const core::api::sampler_t* operator->() const { return &(this->sampler_); };
                core::api::sampler_t& operator*() { return (this->sampler_); };
                const core::api::sampler_t& operator*() const { return (this->sampler_); };
            };
        };
    };
};
