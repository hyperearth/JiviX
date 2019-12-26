#pragma once

#include "./classes/API/types.hpp"
#include "./classes/API/unified/device.hpp"
#include "./factory/API/unified/pipeline_layout.hpp"
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

                // TODO: add assigment by core types and shared_ptr types
                compute_pipeline& operator=(const compute_pipeline &compute_pipeline) { 
                    this->pipeline_ = compute_pipeline;
                    this->device_ = (stu::device_t&)compute_pipeline;
                    return *this;
                };

                // TODO: move into `.cpp` file
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

                operator core::api::device_t&() { return device_; };
                operator const core::api::device_t&() const { return device_; };

                operator core::api::physical_device_t&() { return device_; };
                operator const core::api::physical_device_t&() const { return device_; };
                
                operator core::api::pipeline_layout_t&() { return *layout_; };
                operator const core::api::pipeline_layout_t&() const { return *layout_; };

            // 
            protected: friend compute_pipeline;
                stu::compute_pipeline pipeline_ = {};
                stu::pipeline_layout layout_ = {};
                stu::device_t device_ = {};
            };

        };
    };
};
