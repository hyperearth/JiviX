#pragma once

#include "./classes/API/types.hpp"
#include "./factory/API/unified/allocator.hpp"
#include "./factory/API/unified/buffer.hpp"
#include "./factory/API/unified/vector.hpp"
#include "./factory/API/unified/allocator.hpp"
#include "./classes/API/unified/allocator.hpp"

namespace svt {
    namespace api {
        namespace classes {
            class buffer { public: 
                buffer(const stu::buffer& buffer_t = {}) : buffer_t(buffer_t) {};
                buffer(const stu::device& device_t, const stu::buffer& buffer_t = {}) : buffer_t(buffer_t), device_t(device_t) {};
                buffer(const buffer& buffer) : buffer_t(buffer), device_t(buffer) {};

                // TODO: move into `.cpp` file
                operator stu::buffer&() { return buffer_t; };
                operator stu::device&() { return device_t; };
                operator const stu::buffer&() const { return buffer_t; };
                operator const stu::device&() const { return device_t; };

                // 
                stu::vector vector(uintptr_t offset = 0u, size_t size = 4u);
                svt::core::handle_ref<buffer,core::api::result_t> create(const allocator& allocator = {}, const buffer_create_info& info = {});

                // UN-safe (Debug) API, always should begin from `_`
                svt::core::api::buffer_t _get_buffer_t();
                
                // TODO: move into `.cpp` file
                buffer& operator=(const buffer &buffer) { 
                    this->buffer_t = buffer;
                    this->device_t = buffer;
                    return *this;
                };
                
                // TODO: move into `.cpp` file
                api::factory::buffer_t* operator->() { return &(*this->buffer_t); };
                const api::factory::buffer_t* operator->() const { return &(*this->buffer_t); };

            protected: friend buffer;
                stu::buffer buffer_t = {};
                stu::device device_t = {};
            };
        };
    };
};
