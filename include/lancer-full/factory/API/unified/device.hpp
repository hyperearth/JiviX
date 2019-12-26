#pragma once

#include "./core/unified/core.hpp"

namespace svt {
    namespace api {
        namespace factory {

            class device_t : public std::enable_shared_from_this<device_t> { public: 
                // Native Device Related
                core::api::device_t device_ = API_NULL_HANDLE;
                core::api::dispatch_t dispatch_ = API_NULL_HANDLE;

                // Native Pipeline Cache
                core::api::pipeline_cache_t pipeline_cache_ = API_NULL_HANDLE;
                
                // Mainline Thread
                core::api::descriptor_pool_t descriptor_pool_ = API_NULL_HANDLE;
                core::api::command_pool_t command_pool_ = API_NULL_HANDLE;

                // Native Queue Family Indices
                std::vector<uint32_t> queue_family_indices_ = {};

                device_t() {};
                device_t(const device_t& device) : device_(device) {};
                device_t(const core::api::device_t& device_) : device_(device_) {};

                // TODO: extend by types above
                device_t& operator=(const device_t& device) { this->device_ = device; return *this; };
                device_t& operator=(const std::shared_ptr<device_t>& device) { this->device_ = *device; return *this; };
                device_t& operator=(const core::api::device_t& device_) { this->device_ = device_; return *this; };

                operator uintptr_t&() { return (uintptr_t&)(device_); };
                operator const uintptr_t&() const { return (uintptr_t&)(device_); };
                operator core::api::device_t&() { return device_; };
                operator const core::api::device_t&() const { return device_; };
                operator core::api::dispatch_t&() { return dispatch_; };
                operator const core::api::dispatch_t&() const { return dispatch_; };
                operator core::api::descriptor_pool_t&() { return descriptor_pool_; };
                operator const core::api::descriptor_pool_t&() const { return descriptor_pool_; };
                operator core::api::command_pool_t&() { return command_pool_; };
                operator const core::api::command_pool_t&() const { return command_pool_; };
                operator core::api::pipeline_cache_t&() { return pipeline_cache_; };
                operator const core::api::pipeline_cache_t&() const { return pipeline_cache_; };
                operator std::vector<uint32_t>&() { return queue_family_indices_; };
                operator const std::vector<uint32_t>&() const { return queue_family_indices_; };

                core::api::device_t* operator->() { return &(this->device_); };
                const core::api::device_t* operator->() const { return &(this->device_); };
                core::api::device_t& operator*() { return (this->device_); };
                const core::api::device_t& operator*() const { return (this->device_); };

                // 
                core::api::swapchain_t create_swapchain(const swapchain_create_info& info) const;
            };

        };
    };
};
