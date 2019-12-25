#pragma once

#include "./core/unified/core.hpp"

namespace svt {
    namespace api {
        namespace factory {

            class swapchain_t : public std::enable_shared_from_this<swapchain_t> { public: 
                core::api::swapchain_t swapchain_ = API_NULL_HANDLE;
                //std::vector<uint32_t> swapchain_family_indices_ = {};

                swapchain_t() {};
                swapchain_t(const swapchain_t& swapchain) : swapchain_(swapchain) {};
                swapchain_t(const core::api::swapchain_t& swapchain_) : swapchain_(swapchain_) {};
                swapchain_t(core::api::swapchain_t swapchain_ = API_NULL_HANDLE) : swapchain_(swapchain_) {};
                swapchain_t& operator=(const swapchain_t& swapchain) { this->swapchain_ = swapchain; return *this; };

                operator uintptr_t&() { return (uintptr_t&)(swapchain_); };
                operator const uintptr_t&() const { return (uintptr_t&)(swapchain_); };
                operator core::api::swapchain_t&() { return swapchain_; };
                operator const core::api::swapchain_t&() const { return swapchain_; };

                core::api::swapchain_t* operator->() { return &(this->swapchain_); };
                const core::api::swapchain_t* operator->() const { return &(this->swapchain_); };
                core::api::swapchain_t& operator*() { return (this->swapchain_); };
                const core::api::swapchain_t& operator*() const { return (this->swapchain_); };
            };

        };
    };
};
