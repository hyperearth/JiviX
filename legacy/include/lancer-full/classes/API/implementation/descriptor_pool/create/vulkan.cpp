// 
#include "./core/unified/core.hpp"
#include "./classes/API/unified/device.hpp"

// 
#include "./factory/API/unified/descriptor_pool.hpp"
#include "./classes/API/unified/descriptor_pool.hpp"

namespace svt {
    namespace api {
        namespace classes {
#ifdef USE_VULKAN
            // TODO: Extensions for descriptor_pools
            svt::core::handle_ref<descriptor_pool, core::api::result_t> descriptor_pool::create(const descriptor_pool_create_info& info) {
                vk::DescriptorPoolCreateInfo vk_info = {};
                vk_info.flags = vk::DescriptorPoolCreateFlags(info.flags);
                vk_info.maxSets = info.max_sets;
                vk_info.poolSizeCount = info.pool_sizes.size();
                vk_info.pPoolSizes = (vk::DescriptorPoolSize*)info.pool_sizes.data();
                
                // 
                this->descriptor_pool_ = std::make_shared<api::factory::descriptor_pool_t>((*device_)->createDescriptorPool(vk_info));
                return { *this, core::api::result_t(0u) };
            };
#endif
        };
    };
};
