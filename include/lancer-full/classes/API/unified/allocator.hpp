#pragma once

#include "./classes/API/types.hpp"
#include "./factory/API/unified/allocator.hpp"
#include "./classes/API/unified/buffer.hpp"
#include "./classes/API/unified/image.hpp"

namespace svt {
    namespace api {
        namespace classes {
            // Agregated type from `allocator_t`, and can be created by dedicated utils (such as VMA)
            class allocator { public: 
                allocator(const allocator& allocator) : device_(allocator), allocator_(allocator) {};
                allocator(                              const stu::allocator& allocator_ = {}) : allocator_(allocator_) {};
                allocator(const stu::device_t& device_, const stu::allocator& allocator_ = {}) : allocator_(allocator_), device_(device_) {};

                // TODO: move into `.cpp` file
                operator stu::allocator&() { return allocator_; };
                operator stu::device&() { return device_; };
                operator stu::device_t&() { return device_; };
                operator const stu::allocator&() const { return allocator_; };
                operator const stu::device&() const { return device_; };
                operator const stu::device_t&() const { return device_; };

                // TODO: move into `.cpp` file
                allocator& operator=(const allocator &allocator) { 
                    this->allocator_ = allocator;
                    this->device_ = allocator;
                    return *this;
                };

                // TODO: move into `.cpp` file
                virtual buffer& create_buffer(const buffer_create_info& info, const uintptr_t& info_ptr = 0u) { auto buffer_ = buffer(device_); buffer_.create(allocator_, info, info_ptr); return buffer_; };
                virtual image& create_image(const image_create_info& info, const uintptr_t& info_ptr = 0u, const image_layout& initial_layout = image_layout::t_undefined) { auto image_ = image(device_); image_.create(allocator_, info, info_ptr, initial_layout); return image_; };

                // TODO: move into `.cpp` file
                api::factory::allocator_t* operator->() { return &(*this->allocator_); };
                const api::factory::allocator_t* operator->() const { return &(*this->allocator_); };
                
            protected: friend allocator;
                stu::allocator allocator_ = {};
                stu::device_t device_ = {};
            };
        };
    };
};
