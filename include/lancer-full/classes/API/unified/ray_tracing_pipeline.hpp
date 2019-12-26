#pragma once

#include "./classes/API/types.hpp"
#include "./classes/API/unified/device.hpp"
#include "./factory/API/unified/pipeline_layout.hpp"
#include "./factory/API/unified/ray_tracing_pipeline.hpp"

namespace svt {
    namespace api {
        namespace classes {

            // TODO: push bindings, full version
            // TODO: complete descriptor layout entries
            // TODO: add implementation into `.cpp`
            class ray_tracing_pipeline { public: 
                ray_tracing_pipeline(const ray_tracing_pipeline& ray_tracing_pipeline) : device_(ray_tracing_pipeline), pipeline_(ray_tracing_pipeline) {};
                ray_tracing_pipeline(                              const stu::ray_tracing_pipeline& pipeline_ = {}) : pipeline_(pipeline_) {};
                ray_tracing_pipeline(const stu::device_t& device_, const stu::ray_tracing_pipeline& pipeline_ = {}) : pipeline_(pipeline_), device_(device_) {};

                // TODO: add assigment by core types and shared_ptr types
                ray_tracing_pipeline& operator=(const ray_tracing_pipeline &ray_tracing_pipeline) { 
                    this->pipeline_ = ray_tracing_pipeline;
                    this->device_ = ray_tracing_pipeline;
                    return *this;
                };

                // TODO: move into `.cpp` file
                operator stu::ray_tracing_pipeline&() { return pipeline_; };
                operator stu::device&() { return device_; };
                operator stu::device_t&() { return device_; };
                operator const stu::ray_tracing_pipeline&() const { return pipeline_; };
                operator const stu::device&() const { return device_; };
                operator const stu::device_t&() const { return device_; };

                // 
                api::factory::ray_tracing_pipeline_t* operator->() { return &(*this->pipeline_); };
                const api::factory::ray_tracing_pipeline_t* operator->() const { return &(*this->pipeline_); };
                api::factory::ray_tracing_pipeline_t& operator*() { return (*this->pipeline_); };
                const api::factory::ray_tracing_pipeline_t& operator*() const { return (*this->pipeline_); };

                // 
                svt::core::handle_ref<ray_tracing_pipeline,core::api::result_t> create(const ray_tracing_pipeline_create_info& info = {});

                // 
                operator uintptr_t&() { return (*pipeline_); };
                operator const uintptr_t&() const { return (*pipeline_); };

                operator core::api::ray_tracing_pipeline_t&() { return (*pipeline_); };
                operator const core::api::ray_tracing_pipeline_t&() const { return (*pipeline_); };

                operator core::api::device_t&() { return device_; };
                operator const core::api::device_t&() const { return device_; };

                operator core::api::physical_device_t&() { return device_; };
                operator const core::api::physical_device_t&() const { return device_; };

                operator core::api::pipeline_layout_t&() { return *layout_; };
                operator const core::api::pipeline_layout_t&() const { return *layout_; };


            protected: friend ray_tracing_pipeline;
                stu::ray_tracing_pipeline pipeline_ = {};
                stu::pipeline_layout layout_ = {};
                stu::device_t device_ = {};
            };

        };
    };
};
