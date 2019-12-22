#pragma once

#include "./classes/API/types.hpp"
#include "./factory/API/unified/sampler.hpp"

namespace svt {
    namespace api {
        namespace classes {
            class sampler { public: 
                sampler(                              const stu::sampler& sampler_ = {}) : sampler_(sampler_) {};
                sampler(const stu::device_t& device_, const stu::sampler& sampler_ = {}) : sampler_(sampler_), device_(device_) {};
                sampler(const sampler& image = {}) : device_(image), sampler_(image) {};

                // TODO: merge into `.cpp`
                operator stu::sampler&() { return sampler_; };
                operator stu::device&() { return device_; };
                operator const stu::sampler&() const { return sampler_; };
                operator const stu::device&() const { return device_; };

                // Currently Aggregator
                svt::core::handle_ref<sampler, core::api::result_t> create(const sampler_create_info& info = {});

                // TODO: move into `.cpp` file
                sampler& operator=(const sampler &image) { 
                    this->sampler_ = image;
                    this->device_ = image;
                    return *this;
                };

                // TODO: move into `.cpp` file
                api::factory::sampler_t* operator->() { return &(*this->sampler_); };
                const api::factory::sampler_t* operator->() const { return &(*this->sampler_); };

                // 
                operator uintptr_t&() { return (*sampler_); };
                operator const uintptr_t&() const { return (*sampler_); };
                operator core::api::sampler_t&() { return (*sampler_); };
                operator const core::api::sampler_t&() const { return (*sampler_); };

                // UN-safe (Debug) API, always should begin from `_`
                svt::core::api::sampler_t _get_sampler_t();

            protected: friend image;
                stu::sampler sampler_ = {};
                stu::allocation allocation_ = {};
                stu::allocator allocator_ = {};
                stu::device_t device_ = {};
            };
        };
    };
};
