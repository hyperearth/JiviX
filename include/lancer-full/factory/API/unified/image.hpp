#pragma once

#include "./core/unified/core.hpp"
#include "./factory/API/types.hpp"

namespace svt {
    namespace api {
        namespace factory {

            // Can Be Extended By VMA Allocators
            class image_t : public std::enable_shared_from_this<image_t> { public: 
                //std::vector<uint32_t> queueFamilyIndices = {};
                core::api::image_t image_ = API_NULL_HANDLE;
                image_layout layout_ = image_layout::t_undefined;

                image_t(const image_t& image) : image_(image) {};
                image_t(const core::api::image_t& image_) : image_(image_) {};
                image_t& operator=(const image_t& image) { this->image_ = image; return *this; };

                operator core::api::image_t&() { return image_; };
                operator const core::api::image_t&() const { return image_; };

                core::api::image_t* operator->() { return &(this->image_); };
                const core::api::image_t* operator->() const { return &(this->image_); };


                virtual ~image_t(){};
            };
        };
    };
};
