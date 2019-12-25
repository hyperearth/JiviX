#pragma once

#include "./classes/API/types.hpp"
#include "./factory/API/unified/image.hpp"
#include "./factory/API/unified/sampler.hpp"
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
                // TODO: image layout support and image layout barrier
                inline std::pair<image_view&, description_handle&> write_into_description(description_handle& handle, const uint32_t& idx = 0u) {
                    auto& handle_ = handle.offset<core::api::image_desc_t>(idx);
                    handle_.sampler = core::api::sampler_t(*sampler_);
                    handle_.imageView = core::api::image_view_t(*image_view_);
                    return {*this, handle};
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
                operator core::api::image_t&() { return (*image_); };
                operator const core::api::image_t&() const { return (*image_); };
                operator core::api::device_t&() { return device_; };
                operator const core::api::device_t&() const { return device_; };
                operator core::api::physical_device_t&() { return device_; };
                operator const core::api::physical_device_t&() const { return device_; };

            // 
            protected: friend image_view;
                stu::image_view image_view_ = {};
                stu::image image_ = {};
                stu::sampler sampler_ = {};
                stu::device_t device_ = {};
            };

        };
    };
};
