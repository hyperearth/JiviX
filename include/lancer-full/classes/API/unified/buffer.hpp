#pragma once

#include "./classes/API/types.hpp"
#include "./factory/API/unified/buffer.hpp"
#include "./factory/API/unified/vector.hpp"
#include "./factory/API/classes/vector.hpp"

namespace svt {
    namespace api {
        namespace classes {
            class buffer { public: 
                buffer(                              const stu::buffer& buffer_ = {}) : buffer_(buffer_) {};
                buffer(const stu::device_t& device_, const stu::buffer& buffer_ = {}) : buffer_(buffer_), device_(device_) {};
                buffer(const buffer& buffer) : allocator_(buffer.allocator_), buffer_(buffer), device_(buffer) {};

                // TODO: move into `.cpp` file
                operator stu::buffer&() { return buffer_; };
                operator stu::device&() { return device_; };
                operator stu::device_t&() { return device_; };
                operator stu::allocator&() { return allocator_; };
                operator const stu::buffer&() const { return buffer_; };
                operator const stu::device&() const { return device_; };
                operator const stu::device_t&() const { return device_; };
                operator const stu::allocator&() const { return allocator_; };

                // TODO: move into `.cpp` file
                stu::vector _vector(uintptr_t offset = 0u, size_t size = 4u, size_t stride = 1u) {
                    return std::make_shared<api::factory::vector_t>(*buffer_,offset,size,stride);
                };
                template<class T = uint8_t>
                vector<T> vector(uintptr_t offset = 0u, size_t size = 1u) {
                    return api::classes::vector<T>(device_, buffer_, _vector(offset,size,sizeof(T)));
                };

                // Currently Aggregator
                svt::core::handle_ref<buffer,core::api::result_t> create(const stu::allocator& allocator_ = {}, const buffer_create_info& info = {}, const uintptr_t& info_ptr = 0u, const buffer_modifier& modifier = buffer_modifier::t_unknown);

                // UN-safe (Debug) API, always should begin from `_`
                svt::core::api::buffer_t _get_buffer_t();

                // TODO: move into `.cpp` file
                buffer& operator=(const buffer &buffer) { 
                    this->buffer_ = buffer;
                    this->allocator_ = buffer.allocator_;
                    this->device_ = buffer;
                    return *this;
                };

                // TODO: move into `.cpp` file
                api::factory::buffer_t* operator->() { return &(*this->buffer_); };
                const api::factory::buffer_t* operator->() const { return &(*this->buffer_); };
                api::factory::buffer_t& operator*() { return (*this->buffer_); };
                const api::factory::buffer_t& operator*() const { return (*this->buffer_); };

                // 
                operator uintptr_t&() { return (uintptr_t&)(buffer_->buffer_); };
                operator const uintptr_t&() const { return (uintptr_t&)(buffer_->buffer_); };

                operator core::api::buffer_t&() { return (*buffer_); };
                operator const core::api::buffer_t&() const { return (*buffer_); };

                operator core::api::device_t&() { return device_; };
                operator const core::api::device_t&() const { return device_; };
                
                operator core::api::physical_device_t&() { return device_; };
                operator const core::api::physical_device_t&() const { return device_; };

            // 
            protected: friend buffer; friend allocator;
                stu::buffer buffer_ = {};
                //stu::allocation allocation_ = {};
                stu::allocator allocator_ = {};
                stu::device_t device_ = {};
            };
        };
    };
};
