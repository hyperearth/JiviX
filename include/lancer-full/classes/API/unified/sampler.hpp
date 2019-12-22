#pragma once

#include "./classes/API/types.hpp"
#include "./factory/API/unified/sampler.hpp"

namespace svt {
    namespace api {
        namespace classes {
            class sampler { public: 
                sampler(                              const stu::sampler& sampler_ = {}) : sampler_(sampler_) {};
                sampler(const stu::device_t& device_, const stu::sampler& sampler_ = {}) : sampler_(sampler_), device_(device_) {};
                sampler(const sampler& sampler = {}) : device_(sampler), sampler_(sampler) {};

                // TODO: merge into `.cpp`
                operator stu::sampler&() { return sampler_; };
                operator stu::device&() { return device_; };
                operator stu::device_t&() { return device_; };
                operator const stu::sampler&() const { return sampler_; };
                operator const stu::device&() const { return device_; };
                operator const stu::device_t&() const { return device_; };

                // TODO: Sampler Aggregator
                // svt::core::handle_ref<sampler, core::api::result_t> create(const sampler_create_info& info = {});

                // TODO: move into `.cpp` file
                sampler& operator=(const sampler &sampler) { 
                    this->sampler_ = sampler;
                    this->device_ = sampler;
                    return *this;
                };

                // TODO: move into `.cpp` file
                api::factory::sampler_t* operator->() { return &(*this->sampler_); };
                const api::factory::sampler_t* operator->() const { return &(*this->sampler_); };
                api::factory::sampler_t& operator*() { return (*this->sampler_); };
                const api::factory::sampler_t& operator*() const { return (*this->sampler_); };

                // 
                operator uintptr_t&() { return (uintptr_t&)(*sampler_->sampler_); };
                operator const uintptr_t&() const { return (uintptr_t&)(*sampler_->sampler_); };
                operator core::api::sampler_t&() { return (*sampler_); };
                operator const core::api::sampler_t&() const { return (*sampler_); };

                // UN-safe (Debug) API, always should begin from `_`
                svt::core::api::sampler_t _get_sampler_t();

            protected: friend sampler;
                stu::sampler sampler_ = {};
                stu::allocation allocation_ = {};
                stu::allocator allocator_ = {};
                stu::device_t device_ = {};
            };
        };
    };
};
