#pragma once

#include "./core/unified/core.hpp"
#include "./factory/API/types.hpp"

namespace svt {
    namespace api {
        namespace factory {
            class image_view_t : public std::enable_shared_from_this<image_view_t> { public: 
                core::api::image_view_t image_view_ = API_NULL_HANDLE;

                image_view_t(const image_view_t& image_view) : image_view_(image_view) {};
                image_view_t(const core::api::image_view_t& image_view_) : image_view_(image_view_) {};
                image_view_t& operator=(const image_view_t& image_view) { this->image_view_ = image_view; return *this; };

                operator core::api::image_view_t&() { return image_view_; };
                operator const core::api::image_view_t&() const { return image_view_; };

                core::api::image_view_t* operator->() { return &(this->image_view_); };
                const core::api::image_view_t* operator->() const { return &(this->image_view_); };
            };
        };
    };
};
