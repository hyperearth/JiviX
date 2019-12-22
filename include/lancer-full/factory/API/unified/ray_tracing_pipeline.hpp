#pragma once

#include "./core/unified/core.hpp"
#include "./factory/API/types.hpp"

namespace svt {
    namespace api {
        namespace factory {
            class ray_tracing_pipeline_t : public std::enable_shared_from_this<ray_tracing_pipeline_t> { public: 
                core::api::ray_tracing_pipeline_t pipeline_ = API_NULL_HANDLE;

                ray_tracing_pipeline_t() {};
                ray_tracing_pipeline_t(const ray_tracing_pipeline_t& ray_tracing_pipeline_t) : pipeline_(ray_tracing_pipeline_t) {};
                ray_tracing_pipeline_t& operator=(const ray_tracing_pipeline_t& ray_tracing_pipeline_t) { pipeline_ = ray_tracing_pipeline_t; return *this; };

                operator uintptr_t&() { return (uintptr_t&)(pipeline_); };
                operator const uintptr_t&() const { return (uintptr_t&)(pipeline_); };
                operator core::api::ray_tracing_pipeline_t&() { return pipeline_; };
                operator const core::api::ray_tracing_pipeline_t&() const { return pipeline_; };

                core::api::ray_tracing_pipeline_t* operator->() { return &(this->pipeline_); };
                const core::api::ray_tracing_pipeline_t* operator->() const { return &(this->pipeline_); };
                
            };
        };
    };
};
