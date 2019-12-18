#pragma once

#include "./classes/API/types.hpp"
#include "./factory/API/unified/allocator.hpp"
#include "./factory/API/unified/image.hpp"
#include "./factory/API/unified/allocator.hpp"
#include "./classes/API/unified/allocator.hpp"

namespace svt {
    namespace api {
        namespace classes {
            class image { public: 
                image(                            const stu::image& image_ = {}) : image_(image_) {};
                image(const stu::device& device_, const stu::image& image_ = {}) : image_(image_), device_(device_) {};
                image(const image& image = {}) : device_(image), image_(image) {};

                // TODO: merge into `.cpp`
                operator stu::image&() { return image_; };
                operator stu::device&() { return device_; };
                operator const stu::image&() const { return image_; };
                operator const stu::device&() const { return device_; };

                // 
                svt::core::handle_ref<image, core::api::result_t> create(const allocator& allocator = {}, const image_create_info& info = {}, const image_layout& initial_layout = image_layout::t_undefined);

                // TODO: move into `.cpp` file
                image& operator=(const image &image) { 
                    this->image_ = image;
                    this->device_ = image;
                    return *this;
                };

                // TODO: move into `.cpp` file
                api::factory::image_t* operator->() { return &(*this->image_); };
                const api::factory::image_t* operator->() const { return &(*this->image_); };

                // UN-safe (Debug) API, always should begin from `_`
                svt::core::api::image_t _get_image_t();

            protected: 
                stu::image image_ = {};
                stu::device device_ = {};
                //image_layout layout_ = {};
                
            };
        };
    };
};
