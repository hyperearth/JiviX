#pragma once

#include "./core/unified/core.hpp"

namespace svt {
    namespace api {
        namespace factory {

            class surface_t : public std::enable_shared_from_this<surface_t> { public: 
                core::api::surface_t surface_ = API_NULL_HANDLE;
                //std::vector<uint32_t> surface_family_indices_ = {};

                surface_t() {};
                surface_t(const surface_t& surface) : surface_(surface) {};
                surface_t(const core::api::surface_t& surface_) : surface_(surface_) {};
                surface_t(core::api::surface_t surface_ = API_NULL_HANDLE) : surface_(surface_) {};
                surface_t& operator=(const surface_t& surface) { this->surface_ = surface; return *this; };
                surface_t& operator=(const std::shared_ptr<surface_t>& surface) { this->surface_ = *surface; return *this; };
                surface_t& operator=(const core::api::surface_t& surface_) { this->surface_ = surface_; return *this; };

                operator uintptr_t&() { return (uintptr_t&)(surface_); };
                operator const uintptr_t&() const { return (uintptr_t&)(surface_); };
                operator core::api::surface_t&() { return surface_; };
                operator const core::api::surface_t&() const { return surface_; };

                core::api::surface_t* operator->() { return &(this->surface_); };
                const core::api::surface_t* operator->() const { return &(this->surface_); };
                core::api::surface_t& operator*() { return (this->surface_); };
                const core::api::surface_t& operator*() const { return (this->surface_); };
            };

        };
    };
};
