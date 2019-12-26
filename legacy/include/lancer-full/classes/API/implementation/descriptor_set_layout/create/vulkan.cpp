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
            svt::core::handle_ref<descriptor_set_layout,core::api::result_t> descriptor_set_layout::create(const descriptor_set_layout_create_info& info) {
                // 
                std::vector<vk::DescriptorSetLayoutBinding> bindings_vk = {};
                std::vector<vk::DescriptorBindingFlagsEXT> bindings_flags_vk = {};

                // 
                for (auto& bi : info.bindings_) {
                    bindings_vk.push_back(vk::DescriptorSetLayoutBinding{bi.binding,vk::DescriptorType(bi.type),bi.count,vk::ShaderStageFlags(bi.shader_stages_32u),(vk::Sampler*)(bi.samplers)});
                    bindings_flags_vk.push_back(vk::DescriptorBindingFlagsEXT(bi.flags_ext_32u));
                };

                // re-assign bindings (full version)
                vk::DescriptorSetLayoutCreateInfo vk_info = {};
                vk::DescriptorSetLayoutBindingFlagsCreateInfoEXT f_info = {};
                vk_info.flags = vk::DescriptorSetLayoutCreateFlags(info.flags);
                vk_info.pNext = &f_info;
                vk_info.pBindings = bindings_vk.data();
                vk_info.bindingCount = bindings_vk.size();
                f_info.pBindingFlags = bindings_flags_vk.data();
                f_info.bindingCount = bindings_flags_vk.size();
                
                // 
                (core::api::descriptor_set_layout_t&)(*descriptor_set_layout_) = (*device_)->createDescriptorSetLayout(vk_info);
                return {*this,core::api::result_t(0u)};
            };
#endif
        };
    };
};
