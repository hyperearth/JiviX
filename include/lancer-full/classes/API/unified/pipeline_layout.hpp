#pragma once

#include "./classes/API/types.hpp"
#include "./classes/API/unified/device.hpp"
#include "./factory/API/unified/descriptor_set_layout.hpp"
#include "./factory/API/unified/pipeline_layout.hpp"


namespace svt {
    namespace api {
        namespace classes {

            class pipeline_layout { public: 
                pipeline_layout(const pipeline_layout& pipeline_layout) : device_(pipeline_layout), pipeline_layout_(pipeline_layout) {};
                pipeline_layout(                              const stu::pipeline_layout& pipeline_layout_ = {}) : pipeline_layout_(pipeline_layout_) {};
                pipeline_layout(const stu::device_t& device_, const stu::pipeline_layout& pipeline_layout_ = {}) : pipeline_layout_(pipeline_layout_), device_(device_) {};

                // TODO: add assigment by core types and shared_ptr types
                pipeline_layout& operator=(const pipeline_layout &pipeline_layout) { 
                    this->pipeline_layout_ = pipeline_layout;
                    this->device_ = (stu::device_t&)pipeline_layout;
                    return *this;
                };

                // TODO: move into `.cpp` file
                operator stu::pipeline_layout&() { return pipeline_layout_; };
                operator stu::device&() { return device_; };
                operator stu::device_t&() { return device_; };
                operator const stu::pipeline_layout&() const { return pipeline_layout_; };
                operator const stu::device&() const { return device_; };
                operator const stu::device_t&() const { return device_; };

                // 
                api::factory::pipeline_layout_t* operator->() { return &(*this->pipeline_layout_); };
                const api::factory::pipeline_layout_t* operator->() const { return &(*this->pipeline_layout_); };
                api::factory::pipeline_layout_t& operator*() { return (*this->pipeline_layout_); };
                const api::factory::pipeline_layout_t& operator*() const { return (*this->pipeline_layout_); };

                // 
                svt::core::handle_ref<pipeline_layout,core::api::result_t> create(const pipeline_layout_create_info& info = {});
                
                // 
                operator uintptr_t&() { return (*pipeline_layout_); };
                operator const uintptr_t&() const { return (*pipeline_layout_); };
                operator core::api::pipeline_layout_t&() { return (*pipeline_layout_); };
                operator const core::api::pipeline_layout_t&() const { return (*pipeline_layout_); };
                operator core::api::device_t&() { return device_; };
                operator const core::api::device_t&() const { return device_; };
                operator core::api::physical_device_t&() { return device_; };
                operator const core::api::physical_device_t&() const { return device_; };

            // 
            protected: friend descriptor_set; friend pipeline_layout;
                stu::pipeline_layout pipeline_layout_ = {};
                stu::device_t device_ = {};
            };

        };
    };
};
