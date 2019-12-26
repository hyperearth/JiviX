#include "./factory/API/unified/device.hpp"
#include "./factory/API/unified/descriptor_pool.hpp"
#include "./factory/API/unified/descriptor_set.hpp"
#include "./classes/API/unified/descriptor_set.hpp"


namespace svt {
    namespace api {
        namespace classes {
#ifdef USE_VULKAN
            svt::core::handle_ref<descriptor_set,core::api::result_t> descriptor_set::create(const descriptor_set_create_info& info) {
                vk::DescriptorSetAllocateInfo vk_info = {};
                vk_info.descriptorPool = descriptor_pool_ ? vk::DescriptorPool(*descriptor_pool_) : vk::DescriptorPool(*device_);
                vk_info.pSetLayouts = &(vk::DescriptorSetLayout&)(descriptor_set_layout_);
                vk_info.descriptorSetCount = 1u;
                descriptor_set_ = std::make_shared<api::factory::descriptor_set_t>((*device_)->allocateDescriptorSets(vk_info)[0]);
                return {*this,core::api::result_t(0u)};
            };
#endif
        };
    };
};
