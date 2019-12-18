#pragma once

#include "./core/unified/core.hpp"
#include "./factory/API/types.hpp"

namespace svt {
    namespace api {
        namespace factory {

            // Can Be Extended By VMA Allocators
            class image_t : public std::enable_shared_from_this<image_t> { public: 
                //std::vector<uint32_t> queueFamilyIndices = {};
                core::api::image_t image = (core::api::image_t)(nullptr);
                image_layout layout = image_layout::t_undefined;


                image_t(const image_t& image) : image(image) {};
                image_t(const core::api::image_t& image) : image(image) {};
                image_t& operator=(const image_t& image) { this->image = image; return *this; };

                operator core::api::image_t&() { return image; };
                operator const core::api::image_t&() const { return image; };

                core::api::image_t* operator->() { return &(this->image); };
                const core::api::image_t* operator->() const { return &(this->image); };


                virtual ~image_t(){};
            };
        };
    };
};
