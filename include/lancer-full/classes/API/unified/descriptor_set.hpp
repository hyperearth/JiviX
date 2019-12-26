#pragma once

#include "./classes/API/types.hpp"
#include "./classes/API/unified/device.hpp"
#include "./factory/API/unified/descriptor_set.hpp"
#include "./factory/API/unified/descriptor_set_layout.hpp"
#include "./classes/API/unified/descriptor_set_layout.hpp"

namespace svt {
    namespace api {
        namespace classes {

            class descriptor_set { public: 
                descriptor_set(const descriptor_set& descriptor_set) : device_(descriptor_set), descriptor_set_(descriptor_set) {};
                descriptor_set(                              const stu::descriptor_set& descriptor_set_ = {}) : descriptor_set_(descriptor_set_) {};
                descriptor_set(const stu::device_t& device_, const stu::descriptor_set& descriptor_set_ = {}) : descriptor_set_(descriptor_set_), device_(device_) {};

                // TODO: add assigment by core types and shared_ptr types
                descriptor_set& operator=(const descriptor_set &descriptor_set) { 
                    this->descriptor_set_layout_ = descriptor_set;
                    this->descriptor_set_ = descriptor_set;
                    this->device_ = descriptor_set;
                    return *this;
                };

                // TODO: move into `.cpp` file
                operator stu::descriptor_set_layout&() { return descriptor_set_layout_; };
                operator stu::descriptor_set&() { return descriptor_set_; };
                operator stu::descriptor_pool&() { return descriptor_pool_; };
                operator stu::device&() { return device_; };
                operator stu::device_t&() { return device_; };
                operator const stu::descriptor_set_layout&() const { return descriptor_set_layout_; };
                operator const stu::descriptor_set&() const { return descriptor_set_; };
                operator const stu::descriptor_pool&() const { return descriptor_pool_; };
                operator const stu::device&() const { return device_; };
                operator const stu::device_t&() const { return device_; };

                // 
                api::factory::descriptor_set_t* operator->() { return &(*this->descriptor_set_); };
                const api::factory::descriptor_set_t* operator->() const { return &(*this->descriptor_set_); };
                api::factory::descriptor_set_t& operator*() { return (*this->descriptor_set_); };
                const api::factory::descriptor_set_t& operator*() const { return (*this->descriptor_set_); };

                // TODO: create descriptor set method
                svt::core::handle_ref<descriptor_set,core::api::result_t> update(const descriptor_set_update_info& info = {});
                svt::core::handle_ref<descriptor_set,core::api::result_t> create(const descriptor_set_create_info& info = {});

                // 
                operator uintptr_t&() { return (*descriptor_set_); };
                operator const uintptr_t&() const { return (*descriptor_set_); };

                operator core::api::descriptor_set_t&() { return (*descriptor_set_); };
                operator const core::api::descriptor_set_t&() const { return (*descriptor_set_); };

                operator core::api::descriptor_set_layout_t&() { return (*descriptor_set_layout_); };
                operator const core::api::descriptor_set_layout_t&() const { return (*descriptor_set_layout_); };

                operator core::api::device_t&() { return device_; };
                operator const core::api::device_t&() const { return device_; };
                
                operator core::api::physical_device_t&() { return device_; };
                operator const core::api::physical_device_t&() const { return device_; };

            protected: friend descriptor_set;
                stu::descriptor_set descriptor_set_ = {};
                stu::descriptor_pool descriptor_pool_ = {};
                stu::descriptor_set_layout descriptor_set_layout_ = {};
                stu::device_t device_ = {};
            };
        };
    };
};
