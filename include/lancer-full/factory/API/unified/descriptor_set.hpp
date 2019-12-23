#pragma once

#include "./core/unified/core.hpp"
#include "./factory/API/types.hpp"

namespace svt {
    namespace api {
        namespace factory {
            class descriptor_set_t : public std::enable_shared_from_this<descriptor_set_t> { public: 
                core::api::descriptor_set_t set_ = API_NULL_HANDLE;
                core::api::descriptor_set_template_t temp_ = API_NULL_HANDLE;

                descriptor_set_t() {};
                descriptor_set_t(const descriptor_set_t& descriptor_set_t) : set_(descriptor_set_t), temp_(descriptor_set_t) {};
                descriptor_set_t(const core::api::descriptor_set_t& set_ = API_NULL_HANDLE, core::api::descriptor_set_template_t temp_ = API_NULL_HANDLE) : set_(set_), temp_(temp_) {};
                descriptor_set_t& operator=(const descriptor_set_t& descriptor_set_t) { set_ = descriptor_set_t, temp_ = descriptor_set_t; return *this; };

                operator uintptr_t&() { return (uintptr_t&)(set_); };
                operator const uintptr_t&() const { return (uintptr_t&)(set_); };
                operator core::api::descriptor_set_t&() {return set_; };
                operator core::api::descriptor_set_template_t&() {return temp_; };
                operator const core::api::descriptor_set_t&() const {return set_; };
                operator const core::api::descriptor_set_template_t&() const {return temp_; };

                core::api::descriptor_set_t* operator->() { return &(this->set_); };
                const core::api::descriptor_set_t* operator->() const { return &(this->set_); };
                core::api::descriptor_set_t& operator*() { return (this->set_); };
                const core::api::descriptor_set_t& operator*() const { return (this->set_); };
            };
        };
    };
};
