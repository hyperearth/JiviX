#pragma once

#include "./classes/API/types.hpp"
#include "./classes/API/unified/device.hpp"
#include "./factory/API/unified/command_buffer.hpp"

namespace svt {
    namespace api {
        namespace classes {

            // TODO: full command buffer
            // TODO: add implementation into `.cpp`
            class command_buffer { public: 
                command_buffer(const command_buffer& command_buffer) : device_(command_buffer), cmdbuf_(command_buffer) {};
                command_buffer(                              const stu::command_buffer& cmdbuf_ = {}) : cmdbuf_(cmdbuf_) {};
                command_buffer(const stu::device_t& device_, const stu::command_buffer& cmdbuf_ = {}) : cmdbuf_(cmdbuf_), device_(device_) {};

                // TODO: add assigment by core types and shared_ptr types 
                command_buffer& operator=(const command_buffer &command_buffer) { 
                    this->cmdbuf_ = command_buffer;
                    this->device_ = command_buffer;
                    return *this;
                };

                // TODO: move into `.cpp` file
                operator stu::command_buffer&() { return cmdbuf_; };
                operator stu::device&() { return device_; };
                operator stu::device_t&() { return device_; };
                operator const stu::command_buffer&() const { return cmdbuf_; };
                operator const stu::device&() const { return device_; };
                operator const stu::device_t&() const { return device_; };

                // 
                api::factory::command_buffer_t* operator->() { return &(*this->cmdbuf_); };
                const api::factory::command_buffer_t* operator->() const { return &(*this->cmdbuf_); };
                api::factory::command_buffer_t& operator*() { return (*this->cmdbuf_); };
                const api::factory::command_buffer_t& operator*() const { return (*this->cmdbuf_); };

                // TODO: Command Pool Support
                //svt::core::handle_ref<command_buffer,core::api::result_t> create(const command_buffer_create_info& info = {});

                // 
                operator uintptr_t&() { return (*cmdbuf_); };
                operator const uintptr_t&() const { return (*cmdbuf_); };

                operator core::api::command_buffer_t&() { return (*cmdbuf_); };
                operator const core::api::command_buffer_t&() const { return (*cmdbuf_); };

                operator core::api::device_t&() { return device_; };
                operator const core::api::device_t&() const { return device_; };
                
                operator core::api::physical_device_t&() { return device_; };
                operator const core::api::physical_device_t&() const { return device_; };

            // 
            protected: friend command_buffer;
                stu::command_buffer cmdbuf_ = {};
                stu::device_t device_ = {};
            };

        };
    };
};
