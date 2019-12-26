#pragma once

#include "./classes/API/types.hpp"
#include "./factory/API/unified/render_pass.hpp"

namespace svt {
    namespace api {
        namespace classes {
            class render_pass { public: 
                render_pass(const render_pass& render_pass) : render_pass_(render_pass) {};
                render_pass(const stu::device_t& device_ = {}, const stu::render_pass& render_pass_ = {}) : render_pass_(render_pass_), device_(device_) {};
                render_pass(                                   const stu::render_pass& render_pass_ = {}) : render_pass_(render_pass_) {};

                // TODO: add assigment by core types and shared_ptr types
                render_pass& operator=(const render_pass &render_pass) { this->device_ = (stu::device_t&)render_pass; this->render_pass_ = render_pass; return *this; };

                // TODO: move into `.cpp` file
                operator stu::render_pass&() { return render_pass_; };
                operator stu::device_t&() { return device_; };
                operator const stu::render_pass&() const { return render_pass_; };
                operator const stu::device_t&() const { return device_; };

                api::factory::render_pass_t* operator->() { return &(*this->render_pass_); };
                const api::factory::render_pass_t* operator->() const { return &(*this->render_pass_); };
                api::factory::render_pass_t& operator*() { return (*this->render_pass_); };
                const api::factory::render_pass_t& operator*() const { return (*this->render_pass_); };

                // 
                svt::core::handle_ref<render_pass, core::api::result_t> create(const render_pass_create_info& info = {});

                // 
                operator uintptr_t&() { return *render_pass_; };
                operator const uintptr_t&() const { return *render_pass_; };

                operator core::api::render_pass_t&() { return *render_pass_; };
                operator const core::api::render_pass_t&() const { return *render_pass_; };

                operator core::api::device_t&() { return device_; };
                operator const core::api::device_t&() const { return device_; };

                operator core::api::physical_device_t&() { return device_; };
                operator const core::api::physical_device_t&() const { return device_; };

            protected: 
                stu::render_pass render_pass_ = {};
                stu::device_t device_ = {};
            };
        };
    };
};
