#pragma once

#include "./classes/API/types.hpp"
#include "./classes/API/unified/device.hpp"
#include "./factory/API/unified/compute_pipeline.hpp"

namespace svt {
    namespace api {
        namespace classes {

            // TODO: push bindings, full version
            // TODO: complete descriptor layout entries
            // TODO: add implementation into `.cpp`
            class compute_pipeline { public: 
                compute_pipeline(const compute_pipeline& compute_pipeline) : device_(compute_pipeline), pipeline_(compute_pipeline) {};
                compute_pipeline(                              const stu::compute_pipeline& pipeline_ = {}) : pipeline_(pipeline_) {};
                compute_pipeline(const stu::device_t& device_, const stu::compute_pipeline& pipeline_ = {}) : pipeline_(pipeline_), device_(device_) {};

                // 
                compute_pipeline& operator=(const compute_pipeline &compute_pipeline) { 
                    this->pipeline_ = compute_pipeline;
                    this->device_ = compute_pipeline;
                    return *this;
                };

                // 
                operator stu::compute_pipeline&() { return pipeline_; };
                operator stu::device&() { return device_; };
                operator stu::device_t&() { return device_; };
                operator const stu::compute_pipeline&() const { return pipeline_; };
                operator const stu::device&() const { return device_; };
                operator const stu::device_t&() const { return device_; };

                // 
                api::factory::compute_pipeline_t* operator->() { return &(*this->pipeline_); };
                const api::factory::compute_pipeline_t* operator->() const { return &(*this->pipeline_); };
                api::factory::compute_pipeline_t& operator*() { return (*this->pipeline_); };
                const api::factory::compute_pipeline_t& operator*() const { return (*this->pipeline_); };

                // 
                svt::core::handle_ref<compute_pipeline,core::api::result_t> create(const compute_pipeline_create_info& info = {});

                // 
                operator uintptr_t&() { return (*pipeline_); };
                operator const uintptr_t&() const { return (*pipeline_); };
                operator core::api::compute_pipeline_t&() { return (*pipeline_); };
                operator const core::api::compute_pipeline_t&() const { return (*pipeline_); };


            protected: friend compute_pipeline;
                stu::compute_pipeline pipeline_ = {};
                stu::device_t device_ = {};
            };

        };
    };
};
