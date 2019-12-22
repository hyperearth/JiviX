#pragma once

#include "./classes/API/types.hpp"
#include "./factory/API/unified/render_pass.hpp"

namespace svt {
    namespace api {
        namespace classes {
            class render_pass { public: 
                render_pass(const render_pass& render_pass) : render_pass_(render_pass) {};
                render_pass(const stu::render_pass_t& render_pass_ = {}) : render_pass_(render_pass_) {};
                render_pass& operator=(const render_pass &render_pass) { this->render_pass_ = render_pass; return *this; };

                operator stu::render_pass&() { return render_pass_; };
                operator stu::render_pass_t&() { return render_pass_; };
                operator const stu::render_pass&() const { return render_pass_; };
                operator const stu::render_pass_t&() const { return render_pass_; };

                api::factory::render_pass_t* operator->() { return &(*this->render_pass_); };
                const api::factory::render_pass_t* operator->() const { return &(*this->render_pass_); };
                api::factory::render_pass_t& operator*() { return (*this->render_pass_); };
                const api::factory::render_pass_t& operator*() const { return (*this->render_pass_); };

                // 
                operator uintptr_t&() { return render_pass_; };
                operator const uintptr_t&() const { return render_pass_; };
                operator core::api::render_pass_t&() { return render_pass_; };
                operator const core::api::render_pass_t&() const { return render_pass_; };

            protected: 
                stu::render_pass_t render_pass_ = {};
            };
        };
    };
};
