#pragma once

#include "./core/unified/core.hpp"
#include "./factory/API/types.hpp"

namespace svt {
    namespace api {
        namespace factory {
            class sampler_t : public std::enable_shared_from_this<sampler_t> { public: 
                core::api::sampler_t sampler = (core::api::sampler_t)(nullptr);


                operator uintptr_t&() { return (uintptr_t&)(this->sampler); };
                operator const uintptr_t&() const { return (uintptr_t&)(this->sampler); };

                sampler_t(const sampler_t& buffer) : sampler(sampler) {};
                sampler_t(const core::api::sampler_t& buffer) : sampler(sampler) {};
                sampler_t& operator=(const sampler_t& buffer) { this->sampler = sampler; return *this; };

                operator core::api::sampler_t&() { return sampler; };
                operator const core::api::sampler_t&() const { return sampler; };

                core::api::sampler_t* operator->() { return &(this->sampler); };
                const core::api::sampler_t* operator->() const { return &(this->sampler); };
            };
        };
    };
};
