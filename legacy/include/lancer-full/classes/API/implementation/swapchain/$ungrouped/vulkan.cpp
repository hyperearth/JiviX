//#pragma once

#include "./classes/API/types.hpp"
#include "./classes/API/unified/device.hpp"
#include "./classes/API/unified/swapchain.hpp"

namespace svt {
    namespace api {
        namespace classes {
#ifdef USE_VULKAN
            svt::core::handle_ref<swapchain,core::api::result_t> swapchain::create(const swapchain_create_info& info = {}) {
                swapchain_ = std::make_shared<api::factory::swapchain_t>(device_->create_swapchain(info));
                return {*this,core::api::result_t(0u)};
            };
#endif
        };
    };
};
