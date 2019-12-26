#pragma once

#include "./core/unified/core.hpp"

namespace svt {
    namespace api {
        namespace factory {

            class framebuffer_t : public std::enable_shared_from_this<framebuffer_t> { public: 
                core::api::framebuffer_t framebuffer_ = API_NULL_HANDLE;
                //core::api::pipeline_cache_t pipeline_cache_ = API_NULL_HANDLE;
                //std::vector<uint32_t> queue_family_indices_ = {};

                framebuffer_t() {};
                framebuffer_t(const framebuffer_t& framebuffer) : framebuffer_(framebuffer) {};
                framebuffer_t(const core::api::framebuffer_t& framebuffer_) : framebuffer_(framebuffer_) {};
                framebuffer_t& operator=(const framebuffer_t& framebuffer) { this->framebuffer_ = framebuffer; return *this; };
                framebuffer_t& operator=(const std::shared_ptr<framebuffer_t>& framebuffer_) { this->framebuffer_ = *framebuffer_; return *this; };
                framebuffer_t& operator=(const core::api::framebuffer_t& device_) { this->framebuffer_ = framebuffer_; return *this; };

                operator uintptr_t&() { return (uintptr_t&)(framebuffer_); };
                operator const uintptr_t&() const { return (uintptr_t&)(framebuffer_); };
                operator core::api::framebuffer_t&() { return framebuffer_; };
                operator const core::api::framebuffer_t&() const { return framebuffer_; };

                core::api::framebuffer_t* operator->() { return &(this->framebuffer_); };
                const core::api::framebuffer_t* operator->() const { return &(this->framebuffer_); };
                core::api::framebuffer_t& operator*() { return (this->framebuffer_); };
                const core::api::framebuffer_t& operator*() const { return (this->framebuffer_); };
            };

        };
    };
};
