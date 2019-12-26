#pragma once

#include "./core/unified/core.hpp"
#include "./factory/API/types.hpp"

namespace svt {
    namespace api {
        namespace factory {
            class command_pool_t : public std::enable_shared_from_this<command_pool_t> { public: 
                core::api::command_pool_t cmdpool_ = API_NULL_HANDLE;

                command_pool_t() {};
                command_pool_t(const command_pool_t& command_pool_t) : cmdpool_(cmdpool_) {};
                command_pool_t(core::api::command_pool_t cmdbuf_ = API_NULL_HANDLE) : cmdpool_(cmdpool_) {};
                command_pool_t& operator=(const command_pool_t& command_pool) { cmdpool_ = command_pool; return *this; };
                command_pool_t& operator=(const std::shared_ptr<command_pool_t>& command_pool) { this->cmdpool_ = *command_pool; return *this; };
                command_pool_t& operator=(const core::api::command_pool_t& cmdpool_){ this->cmdpool_ = cmdpool_; return *this; };

                operator uintptr_t&() { return (uintptr_t&)(cmdpool_); };
                operator const uintptr_t&() const { return (uintptr_t&)(cmdpool_); };
                operator core::api::command_pool_t&() { return cmdpool_; };
                operator const core::api::command_pool_t&() const { return cmdpool_; };

                core::api::command_pool_t* operator->() { return &(this->cmdpool_); };
                const core::api::command_pool_t* operator->() const { return &(this->cmdpool_); };
                core::api::command_pool_t& operator*() { return (this->cmdpool_); };
                const core::api::command_pool_t& operator*() const { return (this->cmdpool_); };
            };
        };
    };
};
