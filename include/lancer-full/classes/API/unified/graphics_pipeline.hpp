#pragma once

#include "./classes/API/types.hpp"
#include "./classes/API/unified/device.hpp"
#include "./factory/API/unified/pipeline_layout.hpp"
#include "./factory/API/unified/graphics_pipeline.hpp"

namespace svt {
    namespace api {
        namespace classes {

            // TODO: push bindings, full version
            // TODO: complete descriptor layout entries
            // TODO: add implementation into `.cpp`
            class graphics_pipeline { public: 
                graphics_pipeline(const graphics_pipeline& graphics_pipeline) : device_(graphics_pipeline), pipeline_(graphics_pipeline) {};
                graphics_pipeline(                              const stu::graphics_pipeline& pipeline_ = {}) : pipeline_(pipeline_) {};
                graphics_pipeline(const stu::device_t& device_, const stu::graphics_pipeline& pipeline_ = {}) : pipeline_(pipeline_), device_(device_) {};

                // 
                graphics_pipeline& operator=(const graphics_pipeline &graphics_pipeline) { 
                    this->pipeline_ = graphics_pipeline;
                    this->device_ = graphics_pipeline;
                    return *this;
                };

                // 
                operator stu::graphics_pipeline&() { return pipeline_; };
                operator stu::device&() { return device_; };
                operator stu::device_t&() { return device_; };
                operator const stu::graphics_pipeline&() const { return pipeline_; };
                operator const stu::device&() const { return device_; };
                operator const stu::device_t&() const { return device_; };

                // 
                api::factory::graphics_pipeline_t* operator->() { return &(*this->pipeline_); };
                const api::factory::graphics_pipeline_t* operator->() const { return &(*this->pipeline_); };
                api::factory::graphics_pipeline_t& operator*() { return (*this->pipeline_); };
                const api::factory::graphics_pipeline_t& operator*() const { return (*this->pipeline_); };

                // 
                svt::core::handle_ref<graphics_pipeline,core::api::result_t> create(const graphics_pipeline_create_info& info = {});

                // 
                operator uintptr_t&() { return (*pipeline_); };
                operator const uintptr_t&() const { return (*pipeline_); };
                
                operator core::api::graphics_pipeline_t&() { return (*pipeline_); };
                operator const core::api::graphics_pipeline_t&() const { return (*pipeline_); };

                operator core::api::device_t&() { return device_; };
                operator const core::api::device_t&() const { return device_; };

                operator core::api::physical_device_t&() { return device_; };
                operator const core::api::physical_device_t&() const { return device_; };

                operator core::api::pipeline_layout_t&() { return *layout_; };
                operator const core::api::pipeline_layout_t&() const { return *layout_; };

            // 
            protected: friend graphics_pipeline;
                stu::graphics_pipeline pipeline_ = {};
                stu::pipeline_layout layout_ = {};
                stu::device_t device_ = {};
            };

        };
    };
};
