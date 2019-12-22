#pragma once

#include "./classes/API/types.hpp"
#include "./factory/API/unified/image.hpp"
#include "./factory/API/unified/image_view.hpp"

namespace svt {
    namespace api {
        namespace classes {

            // TODO: create image_view method
            class image_view { public: 

                // TODO: merge into `.cpp`
                operator stu::image_view&() { return image_view_; };
                operator stu::device&() { return device_; };
                operator stu::device_t&() { return device_; };
                operator const stu::image_view&() const { return image_view_; };
                operator const stu::device&() const { return device_; };
                operator const stu::device_t&() const { return device_; };

                // Currently Aggregator
                //svt::core::handle_ref<image, core::api::result_t> create(const stu::allocator& allocator_ = {}, const image_create_info& info = {}, const uintptr_t& info_ptr = 0u, const image_layout& initial_layout = image_layout::t_undefined);

                // TODO: move into `.cpp` file
                image_view& operator=(const image_view &image_view) { 
                    this->image_view_ = image_view;
                    this->device_ = image_view;
                    return *this;
                };

                // TODO: move into `.cpp` file
                api::factory::image_view_t* operator->() { return &(*this->image_view_); };
                const api::factory::image_view_t* operator->() const { return &(*this->image_view_); };
                api::factory::image_view_t& operator*() { return (*this->image_view_); };
                const api::factory::image_view_t& operator*() const { return (*this->image_view_); };

                // UN-safe (Debug) API, always should begin from `_`
                svt::core::api::image_view_t _get_image_view_t();

                // 
                operator uintptr_t&() { return (*image_view_); };
                operator const uintptr_t&() const { return (*image_view_); };
                operator core::api::image_view_t&() { return (*image_view_); };
                operator const core::api::image_view_t&() const { return (*image_view_); };


            protected: 
                stu::image_view image_view_ = {};
                stu::image image_ = {};
                stu::sampler sampler_ = {};
                stu::device_t device_ = {};
            };

        };
    };
};
