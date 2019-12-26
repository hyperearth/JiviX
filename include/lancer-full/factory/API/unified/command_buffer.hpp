#pragma once

#include "./core/unified/core.hpp"
#include "./factory/API/types.hpp"

namespace svt {
    namespace api {
        namespace factory {
            class command_buffer_t : public std::enable_shared_from_this<command_buffer_t> { public: 
                core::api::command_buffer_t cmdbuf_ = API_NULL_HANDLE;

                command_buffer_t() {};
                command_buffer_t(const command_buffer_t& command_buffer_t) : cmdbuf_(cmdbuf_) {};
                command_buffer_t(core::api::command_buffer_t cmdbuf_ = API_NULL_HANDLE) : cmdbuf_(cmdbuf_) {};
                command_buffer_t& operator=(const command_buffer_t& command_buffer) { this->cmdbuf_ = command_buffer; return *this; };
                command_buffer_t& operator=(const std::shared_ptr<command_buffer_t>& command_buffer) { this->cmdbuf_ = *command_buffer; return *this; };
                command_buffer_t& operator=(const core::api::command_buffer_t& cmdbuf_){ this->cmdbuf_ = cmdbuf_; return *this; };

                operator uintptr_t&() { return (uintptr_t&)(cmdbuf_); };
                operator const uintptr_t&() const { return (uintptr_t&)(cmdbuf_); };
                operator core::api::command_buffer_t&() { return cmdbuf_; };
                operator const core::api::command_buffer_t&() const { return cmdbuf_; };

                core::api::command_buffer_t* operator->() { return &(this->cmdbuf_); };
                const core::api::command_buffer_t* operator->() const { return &(this->cmdbuf_); };
                core::api::command_buffer_t& operator*() { return (this->cmdbuf_); };
                const core::api::command_buffer_t& operator*() const { return (this->cmdbuf_); };
            };
        };
    };
};
