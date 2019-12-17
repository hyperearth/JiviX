#pragma once

#include "./core/unified/core.hpp"
#include "./factory/API/types.hpp"

namespace svt {
    namespace api {
        namespace factory {
            class buffer_view_t : public std::enable_shared_from_this<buffer_view_t> { public: 
                core::api::buffer_view_t buffer_view;

                buffer_view_t(const buffer_view_t& buffer_view) : buffer_view(buffer_view) {};
                buffer_view_t(const core::api::buffer_view_t& buffer_view) : buffer_view(buffer_view) {};
                buffer_view_t& operator=(const buffer_view_t& buffer_view) { this->buffer_view = buffer_view; return *this; };

                operator core::api::buffer_view_t&() { return buffer_view; };
                operator const core::api::buffer_view_t&() const { return buffer_view; };

                core::api::buffer_view_t* operator->() { return &(this->buffer_view); };
                const core::api::buffer_view_t* operator->() const { return &(this->buffer_view); };
            };
        };
    };
};
