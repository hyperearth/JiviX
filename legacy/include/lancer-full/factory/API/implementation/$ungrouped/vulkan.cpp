//#pragma once

#include "./factory/API/types.hpp"
#include "./factory/API/unified/physical_device.hpp"
#include "./factory/API/unified/device.hpp"

namespace svt {
    namespace api {
        namespace factory {
#ifdef USE_VULKAN

                // TODO: Sorting
                surface_capabilities physical_device_t::get_surface_capabilities(const core::api::surface_t& surface) const {
                    return (surface_capabilities&)(physical_device_.getSurfaceCapabilitiesKHR(surface));
                };
                
                std::vector<present_mode> physical_device_t::get_present_modes(const core::api::surface_t& surface) const {
                    auto present_modes = physical_device_.getSurfacePresentModesKHR(surface);
                    return std::vector<present_mode>(present_modes.begin(),present_modes.end());
                };

                format_properties physical_device_t::get_format_properties(const format& format) const {
                    return (format_properties&)physical_device_.getFormatProperties(vk::Format(format));
                };

                surface_format physical_device_t::get_surface_formats(const core::api::surface_t& surface) const {
                    return (surface_format&)physical_device_.getSurfaceFormatsKHR(surface);
                };

                // TODO: Sorting
                core::api::swapchain_t device_t::create_swapchain(const swapchain_create_info& info) const {
                    vk::SwapchainCreateInfoKHR vk_info = {};
                    vk_info.flags = vk::SwapchainCreateFlagsKHR(info.flags);
                    vk_info.surface = vk::SurfaceKHR(info.surface);
                    vk_info.minImageCount = info.min_image_count;
                    vk_info.imageFormat = vk::Format(info.image_format);
                    vk_info.imageColorSpace = vk::ColorSpaceKHR(info.image_color_space);
                    vk_info.imageExtent = vk::Extent2D(info.image_extent.x,info.image_extent.y);
                    vk_info.imageArrayLayers = info.image_array_layers;
                    vk_info.imageUsage = vk::ImageUsageFlags(info.image_usage_32u);
                    vk_info.imageSharingMode = vk::SharingMode(info.image_sharing_mode);
                    vk_info.queueFamilyIndexCount = queue_family_indices_.size();
                    vk_info.pQueueFamilyIndices = queue_family_indices_.data();
                    vk_info.preTransform = vk::SurfaceTransformFlagBitsKHR(info.pre_transform_32u);
                    vk_info.compositeAlpha = vk::CompositeAlphaFlagBitsKHR(info.composite_alpha_flags_32u);
                    vk_info.presentMode = vk::PresentModeKHR(info.present_mode);
                    vk_info.clipped = info.clipped;
                    vk_info.oldSwapchain = info.old_swapchain;
                    return device_.createSwapchainKHR(vk_info);
                };


#endif
        };
    };
};
