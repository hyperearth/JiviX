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
                // TODO: add assigment by core types and shared_ptr types
                allocator& operator=(const allocator &allocator) { 
                    this->allocator_ = allocator;
                    this->device_ = allocator;
                    return *this;
                };

                // TODO: move into `.cpp` file
                operator stu::allocator&() { return allocator_; };
                operator stu::device&() { return device_; };
                operator stu::device_t&() { return device_; };
                operator const stu::allocator&() const { return allocator_; };
                operator const stu::device&() const { return device_; };
                operator const stu::device_t&() const { return device_; };

                // TODO: move into `.cpp` file
                svt::core::handle_ref<buffer,core::api::result_t> create_buffer(const buffer_create_info& info, const uintptr_t& info_ptr = 0u, const buffer_modifier& modifier = buffer_modifier::t_unknown) { auto buffer_ = buffer(device_); return buffer_.create(allocator_, info, info_ptr, modifier);        };
                svt::core::handle_ref<image ,core::api::result_t> create_image (const  image_create_info& info, const uintptr_t& info_ptr = 0u, const image_layout& initial_layout = image_layout::t_undefined) { auto image_ = image(device_); return  image_.create(allocator_, info, info_ptr, initial_layout);  };

                // TODO: move into `.cpp` file
                api::factory::allocator_t* operator->() { return &(*this->allocator_); };
                const api::factory::allocator_t* operator->() const { return &(*this->allocator_); };

                api::factory::allocator_t& operator*() { return (*this->allocator_); };
                const api::factory::allocator_t& operator*() const { return (*this->allocator_); };

                operator core::api::device_t&() { return device_; };
                operator const core::api::device_t&() const { return device_; };

                operator core::api::physical_device_t&() { return device_; };
                operator const core::api::physical_device_t&() const { return device_; };

            protected: friend allocator;
                stu::allocator allocator_ = {};
                stu::device_t device_ = {};
            };
        };
    };
};
