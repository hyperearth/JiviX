#pragma once

#include "./core/unified/core.hpp"

namespace svt {
    namespace api {
        namespace factory {

            class physical_device_t : public std::enable_shared_from_this<physical_device_t> { public: 
                core::api::physical_device_t physical_device_ = API_NULL_HANDLE;
                //core::api::pipeline_cache_t pipeline_cache_ = API_NULL_HANDLE;
                //std::vector<uint32_t> queue_family_indices_ = {};

                // TODO: unified features and properties
                #if defined(USE_VULKAN)
                    // main features
                    vk::PhysicalDeviceFeatures2 features_ = {};
                    vk::PhysicalDeviceSubgroupSizeControlFeaturesEXT subgroup_size_control_features_ = {};

                    // main properties
                    vk::PhysicalDeviceProperties2 properties_ = {};
                    vk::PhysicalDeviceConservativeRasterizationPropertiesEXT conservative_rasterization_properties_ = {};
                    vk::PhysicalDeviceSubgroupSizeControlPropertiesEXT subgroup_size_control_properties_ = {};
                    vk::PhysicalDeviceSampleLocationsPropertiesEXT sample_locations_properties = {};
                    vk::PhysicalDeviceRayTracingPropertiesNV ray_tracing_properties_ = {};
                #endif

                physical_device_t() {};
                physical_device_t(const physical_device_t& physical_device) : physical_device_(physical_device) {};
                physical_device_t(const core::api::physical_device_t& physical_device_) : physical_device_(physical_device_) {};
                physical_device_t& operator=(const physical_device_t& physical_device) { this->physical_device_ = physical_device; return *this; };

                operator uintptr_t&() { return (uintptr_t&)(physical_device_); };
                operator const uintptr_t&() const { return (uintptr_t&)(physical_device_); };
                operator core::api::physical_device_t&() { return physical_device_; };
                operator const core::api::physical_device_t&() const { return physical_device_; };

                core::api::physical_device_t* operator->() { return &(this->physical_device_); };
                const core::api::physical_device_t* operator->() const { return &(this->physical_device_); };
                core::api::physical_device_t& operator*() { return (this->physical_device_); };
                const core::api::physical_device_t& operator*() const { return (this->physical_device_); };
            };

        };
    };
};
