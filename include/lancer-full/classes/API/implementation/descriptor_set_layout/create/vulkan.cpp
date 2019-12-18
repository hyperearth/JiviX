#include "./factory/API/unified/device.hpp"
#include "./factory/API/unified/allocator.hpp"
#include "./factory/API/unified/descriptor_set_layout.hpp"
#include "./classes/API/unified/descriptor_set_layout.hpp"
#include "./classes/API/unified/descriptor_set.hpp"


namespace svt {
    namespace api {
        namespace classes {
#ifdef USE_VULKAN
            // 
            svt::core::handle_ref<descriptor_set_layout,core::api::result_t> descriptor_set_layout::create(const uint32_t& flags){
                vk::DescriptorSetLayoutCreateInfo info = {};
                vk::DescriptorSetLayoutBindingFlagsCreateInfoEXT f_info = {};
                info.flags = vk::DescriptorSetLayoutCreateFlags(flags);
                info.pNext = &f_info;

                // 
                std::vector<vk::DescriptorSetLayoutBinding> bindings_vk = {};
                std::vector<vk::DescriptorBindingFlagsEXT> bindings_flags_vk = {};

                // 
                for (auto& bi : this->bindings_) {
                    bindings_vk.push_back(vk::DescriptorSetLayoutBinding{bi.binding,vk::DescriptorType(bi.type),bi.count,vk::ShaderStageFlags(bi.shader_stages),(vk::Sampler*)(bi.samplers)});
                    bindings_flags_vk.push_back(vk::DescriptorBindingFlagsEXT{bi.flags_ext});
                };

                // re-assign bindings (full version)
                f_info.pBindingFlags = bindings_flags_vk.data();
                f_info.bindingCount = bindings_flags_vk.size();
                info.pBindings = bindings_vk.data();
                info.bindingCount = bindings_vk.size();

                (core::api::descriptor_set_layout_t&)(*descriptor_set_layout_) = (*device_)->createDescriptorSetLayout(info);
                return {*this,core::api::result_t(0u)};
            };
#endif
        };
    };
};
