#pragma once

#include "./classes/API/types.hpp"
#include "./factory/API/unified/shader_module.hpp"

namespace svt {
    namespace api {
        namespace classes {
            class shader_module { public: 
                shader_module(                              const stu::shader_module& module_ = {}) : module_(module_) {};
                shader_module(const stu::device_t& device_, const stu::shader_module& module_ = {}) : module_(module_), device_(device_) {};
                shader_module(const shader_module& shader_module = {}) : device_(shader_module), module_(shader_module) {};

                // TODO: merge into `.cpp`
                operator stu::shader_module&() { return module_; };
                operator stu::device&() { return device_; };
                operator stu::device_t&() { return device_; };
                operator const stu::shader_module&() const { return module_; };
                operator const stu::device&() const { return device_; };
                operator const stu::device_t&() const { return device_; };

                // 
                svt::core::handle_ref<shader_module, core::api::result_t> create(const shader_module_create_info& info = {});

                // TODO: move into `.cpp` file
                shader_module& operator=(const shader_module &shader_module) { 
                    this->module_ = shader_module;
                    this->device_ = shader_module;
                    return *this;
                };

                // TODO: move into `.cpp` file
                api::factory::shader_module_t* operator->() { return &(*this->module_); };
                const api::factory::shader_module_t* operator->() const { return &(*this->module_); };
                api::factory::shader_module_t& operator*() { return (*this->module_); };
                const api::factory::shader_module_t& operator*() const { return (*this->module_); };

                // 
                operator uintptr_t&() { return (uintptr_t&)(*module_->module_); };
                operator const uintptr_t&() const { return (uintptr_t&)(*module_->module_); };
                operator core::api::shader_module_t&() { return (*module_); };
                operator const core::api::shader_module_t&() const { return (*module_); };

                // UN-safe (Debug) API, always should begin from `_`
                svt::core::api::shader_module_t _get_shader_module_t();

            protected: friend shader_module;
                stu::shader_module module_ = {};
                stu::device_t device_ = {};
            };
        };
    };
};
