#pragma once

#include "./core/unified/core.hpp"
#include "./factory/API/types.hpp"

namespace svt {
    namespace api {
        namespace factory {
            class shader_module_t : public std::enable_shared_from_this<shader_module_t> { public: 
                core::api::shader_module_t module_ = API_NULL_HANDLE;

                operator uintptr_t&() { return (uintptr_t&)(this->module_); };
                operator const uintptr_t&() const { return (uintptr_t&)(this->module_); };

                shader_module_t() {};
                shader_module_t(const shader_module_t& shader_module) : module_(shader_module) {};
                shader_module_t(const core::api::shader_module_t& module_) : module_(module_) {};
                shader_module_t& operator=(const shader_module_t& shader_module) { this->module_ = module_; return *this; };

                operator uintptr_t&() { return (uintptr_t&)(module_); };
                operator const uintptr_t&() const { return (uintptr_t&)(module_); };
                operator core::api::shader_module_t&() { return module_; };
                operator const core::api::shader_module_t&() const { return module_; };

                core::api::shader_module_t* operator->() { return &(this->module_); };
                const core::api::shader_module_t* operator->() const { return &(this->module_); };
                core::api::shader_module_t& operator*() { return (this->module_); };
                const core::api::shader_module_t& operator*() const { return (this->module_); };
            };
        };
    };
};
