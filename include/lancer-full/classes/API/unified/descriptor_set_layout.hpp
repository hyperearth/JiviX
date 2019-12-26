#pragma once

#include "./classes/API/types.hpp"
#include "./classes/API/unified/device.hpp"
#include "./factory/API/unified/descriptor_set_layout.hpp"
#include "./factory/API/unified/descriptor_set.hpp"

namespace svt {
    namespace api {
        namespace classes {

            // TODO: move some implementations into `.cpp`
            class descriptor_set_layout { public: 
                descriptor_set_layout(const descriptor_set_layout& descriptor_set_layout) : device_(descriptor_set_layout), descriptor_set_layout_(descriptor_set_layout) {};
                descriptor_set_layout(                              const stu::descriptor_set_layout& descriptor_set_layout_ = {}) : descriptor_set_layout_(descriptor_set_layout_) {};
                descriptor_set_layout(const stu::device_t& device_, const stu::descriptor_set_layout& descriptor_set_layout_ = {}) : descriptor_set_layout_(descriptor_set_layout_), device_(device_) {};

                // TODO: add assigment by core types and shared_ptr types
                descriptor_set_layout& operator=(const descriptor_set_layout &descriptor_set_layout) { 
                    this->descriptor_set_layout_ = descriptor_set_layout;
                    this->device_ = (stu::device_t&)descriptor_set_layout;
                    return *this;
                };

                // TODO: move into `.cpp` file
                operator stu::descriptor_set_layout&() { return descriptor_set_layout_; };
                operator stu::device&() { return device_; };
                operator stu::device_t&() { return device_; };
                operator const stu::descriptor_set_layout&() const { return descriptor_set_layout_; };
                operator const stu::device&() const { return device_; };
                operator const stu::device_t&() const { return device_; };

                // 
                api::factory::descriptor_set_layout_t* operator->() { return &(*this->descriptor_set_layout_); };
                const api::factory::descriptor_set_layout_t* operator->() const { return &(*this->descriptor_set_layout_); };
                api::factory::descriptor_set_layout_t& operator*() { return (*this->descriptor_set_layout_); };
                const api::factory::descriptor_set_layout_t& operator*() const { return (*this->descriptor_set_layout_); };

                // TODO: create_descriptor_set
                svt::core::handle_ref<descriptor_set_layout,core::api::result_t> create(const descriptor_set_layout_create_info& info = {});
                svt::core::handle_ref<descriptor_set,core::api::result_t> create_descriptor_set(const descriptor_set_create_info& info = {});

                // 
                operator uintptr_t&() { return (*descriptor_set_layout_); };
                operator const uintptr_t&() const { return (*descriptor_set_layout_); };
                operator core::api::descriptor_set_layout_t&() { return (*descriptor_set_layout_); };
                operator const core::api::descriptor_set_layout_t&() const { return (*descriptor_set_layout_); };
                operator core::api::device_t&() { return device_; };
                operator const core::api::device_t&() const { return device_; };
                operator core::api::physical_device_t&() { return device_; };
                operator const core::api::physical_device_t&() const { return device_; };

            // 
            protected: friend descriptor_set; friend descriptor_set_layout;
                stu::descriptor_set_layout descriptor_set_layout_ = {};
                stu::device_t device_ = {};
            };


        };
    };
};
