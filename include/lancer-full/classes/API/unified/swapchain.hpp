#pragma once

#include "./classes/API/types.hpp"
#include "./factory/API/unified/device.hpp"
#include "./factory/API/unified/swapchain.hpp"

namespace svt {
    namespace api {
        namespace classes {
            class swapchain { public: 
                swapchain(const swapchain& swapchain) : swapchain_(swapchain) {};
                swapchain(const stu::swapchain& swapchain_ = {}) : swapchain_(swapchain_) {};
                swapchain& operator=(const swapchain &swapchain) { this->swapchain_ = swapchain; return *this; };

                operator stu::swapchain&() { return swapchain_; };
                operator stu::device&() { return device_; };
                operator stu::device_t&() { return device_; };
                operator const stu::swapchain&() const { return swapchain_; };
                operator const stu::device&() const { return device_; };
                operator const stu::device_t&() const { return device_; };

                api::factory::swapchain_t* operator->() { return &(*this->swapchain_); };
                const api::factory::swapchain_t* operator->() const { return &(*this->swapchain_); };
                api::factory::swapchain_t& operator*() { return (*this->swapchain_); };
                const api::factory::swapchain_t& operator*() const { return (*this->swapchain_); };

                // 
                operator uintptr_t&() { return (*swapchain_); };
                operator const uintptr_t&() const { return (*swapchain_); };
                
                operator core::api::device_t&() { return device_; };
                operator const core::api::device_t&() const { return device_; };

                operator core::api::physical_device_t&() { return device_; };
                operator const core::api::physical_device_t&() const { return device_; };

                operator core::api::swapchain_t&() { return *swapchain_; };
                operator const core::api::swapchain_t&() const { return *swapchain_; };
                
            //
            protected: friend swapchain;
                stu::swapchain swapchain_ = {};
                stu::device_t device_ = {};
            };
        };
    };
};
