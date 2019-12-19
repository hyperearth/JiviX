#pragma once

#include "./classes/API/types.hpp"
#include "./factory/API/unified/buffer.hpp"

namespace svt {
    namespace api {
        namespace classes {
            class buffer { public: 
                buffer(                            const stu::buffer& buffer_ = {}) : buffer_(buffer_) {};
                buffer(const stu::device& device_, const stu::buffer& buffer_ = {}) : buffer_(buffer_), device_(device_) {};
                buffer(const buffer& buffer) : buffer_(buffer), device_(buffer) {};

                // TODO: move into `.cpp` file
                operator stu::buffer&() { return buffer_; };
                operator stu::device&() { return device_; };
                operator const stu::buffer&() const { return buffer_; };
                operator const stu::device&() const { return device_; };

                // TODO: vector construction
                stu::vector vector(uintptr_t offset = 0u, size_t size = 4u);

                // Currently Aggregator
                svt::core::handle_ref<buffer,core::api::result_t> create(const stu::allocator& allocator_ = {}, const buffer_create_info& info = {}, const uintptr_t& info_ptr = 0u);

                // UN-safe (Debug) API, always should begin from `_`
                svt::core::api::buffer_t _get_buffer_t();
                
                // TODO: move into `.cpp` file
                buffer& operator=(const buffer &buffer) { 
                    this->buffer_ = buffer;
                    this->device_ = buffer;
                    return *this;
                };
                
                // TODO: move into `.cpp` file
                api::factory::buffer_t* operator->() { return &(*this->buffer_); };
                const api::factory::buffer_t* operator->() const { return &(*this->buffer_); };

            protected: friend buffer;
                stu::buffer buffer_ = {};
                stu::allocation allocation_ = {};
                stu::allocator allocator_ = {};
                stu::device device_ = {};
            };
        };
    };
};
