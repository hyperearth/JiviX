#pragma once

#include "./core/unified/core.hpp"
#include "./factory/API/types.hpp"

namespace svt {
    namespace api {
        namespace factory {
            class compute_pipeline_t : public std::enable_shared_from_this<compute_pipeline_t> { public: 
                core::api::compute_pipeline_t pipeline_ = API_NULL_HANDLE;

                compute_pipeline_t() {};
                compute_pipeline_t(const compute_pipeline_t& compute_pipeline_t) : pipeline_(compute_pipeline_t) {};
                compute_pipeline_t(core::api::compute_pipeline_t pipeline_ = API_NULL_HANDLE) : pipeline_(pipeline_) {};
                compute_pipeline_t& operator=(const compute_pipeline_t& compute_pipeline) { this->pipeline_ = compute_pipeline; return *this; };
                compute_pipeline_t& operator=(const std::shared_ptr<compute_pipeline_t>& compute_pipeline) { this->pipeline_ = *compute_pipeline; return *this; };
                compute_pipeline_t& operator=(const core::api::compute_pipeline_t& pipeline_){ this->pipeline_ = pipeline_; return *this; };
                

                operator uintptr_t&() { return (uintptr_t&)(pipeline_); };
                operator const uintptr_t&() const { return (uintptr_t&)(pipeline_); };
                operator core::api::compute_pipeline_t&() { return pipeline_; };
                operator const core::api::compute_pipeline_t&() const { return pipeline_; };

                core::api::compute_pipeline_t* operator->() { return &(this->pipeline_); };
                const core::api::compute_pipeline_t* operator->() const { return &(this->pipeline_); };
                core::api::compute_pipeline_t& operator*() { return (this->pipeline_); };
                const core::api::compute_pipeline_t& operator*() const { return (this->pipeline_); };
            };
        };
    };
};
