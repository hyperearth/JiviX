#pragma once

#include "./classes/API/types.hpp"
#include "./dynamic/API/unified/image.hpp"

namespace svt {
    namespace api {

        class image {
            protected: 
                std::shared_ptr<data::dynamic::image_t> image_t = {};
                std::shared_ptr<data::device_t> device_t = {};

            public: 
                // 
                image(const image& image) : device_t(image.device_t), image_t(image.image_t) {};
                image(const std::shared_ptr<data::dynamic::image_t>& image_t = {}) : image_t(image_t) {};
                image(const std::shared_ptr<data::device_t>& device_t, const std::shared_ptr<data::dynamic::image_t>& image_t = {}) : image_t(image_t), device_t(device_t) {};

                // UN-safe (Debug) API, always should begin from `_`
                svt::api::core::image_t _get_image_t();
        };
    };
};
