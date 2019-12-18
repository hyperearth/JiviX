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
                image(                             const stu::image& image_t = {}) : image_t(image_t) {};
                image(const stu::device& device_t, const stu::image& image_t = {}) : image_t(image_t), device_t(device_t) {};
                image(const image& image = {}) : device_t(image), image_t(image) {};

                // TODO: merge into `.cpp`
                operator stu::image&() { return image_t; };
                operator stu::device&() { return device_t; };
                operator const stu::image&() const { return image_t; };
                operator const stu::device&() const { return device_t; };

                // 
                svt::core::handle_ref<image, core::api::result_t> create(const allocator& allocator = {}, const image_create_info& info = {}, const image_layout& initial_layout = image_layout::t_undefined);

                // TODO: move into `.cpp` file
                image& operator=(const image &image) { 
                    this->image_t = image;
                    this->device_t = image;
                    return *this;
                };

                // TODO: move into `.cpp` file
                api::factory::image_t* operator->() { return &(*this->image_t); };
                const api::factory::image_t* operator->() const { return &(*this->image_t); };

                // UN-safe (Debug) API, always should begin from `_`
                svt::core::api::image_t _get_image_t();

            protected: 
                stu::image image_t = {};
                stu::device device_t = {};
                //image_layout layout_t = {};
                
            };
        };
    };
};
