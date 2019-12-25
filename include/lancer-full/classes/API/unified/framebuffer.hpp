#pragma once

#include "./classes/API/types.hpp"
#include "./factory/API/unified/framebuffer.hpp"

namespace svt {
    namespace api {
        namespace classes {
            class framebuffer { public: 
                framebuffer(const framebuffer& framebuffer) : framebuffer_(framebuffer) {};
                framebuffer(const stu::device_t& device_ = {}, const stu::framebuffer& framebuffer_ = {}) : framebuffer_(framebuffer_), device_(device_) {};
                framebuffer(                                   const stu::framebuffer& framebuffer_ = {}) : framebuffer_(framebuffer_) {};
                framebuffer& operator=(const framebuffer &framebuffer) { this->framebuffer_ = framebuffer; return *this; };

                operator stu::framebuffer&() { return framebuffer_; };
                operator stu::device_t&() { return device_; };
                operator const stu::framebuffer&() const { return framebuffer_; };
                operator const stu::device_t&() const { return device_; };

                api::factory::framebuffer_t* operator->() { return &(*this->framebuffer_); };
                const api::factory::framebuffer_t* operator->() const { return &(*this->framebuffer_); };
                api::factory::framebuffer_t& operator*() { return (*this->framebuffer_); };
                const api::factory::framebuffer_t& operator*() const { return (*this->framebuffer_); };

                // 
                svt::core::handle_ref<framebuffer, core::api::result_t> create(const framebuffer_create_info& info = {});

                // 
                operator uintptr_t&() { return *framebuffer_; };
                operator const uintptr_t&() const { return *framebuffer_; };

                operator core::api::framebuffer_t&() { return *framebuffer_; };
                operator const core::api::framebuffer_t&() const { return *framebuffer_; };

                operator core::api::device_t&() { return device_; };
                operator const core::api::device_t&() const { return device_; };

                operator core::api::physical_device_t&() { return device_; };
                operator const core::api::physical_device_t&() const { return device_; };

            protected: 
                stu::framebuffer framebuffer_ = {};
                stu::device_t device_ = {};
            };
        };
    };
};
