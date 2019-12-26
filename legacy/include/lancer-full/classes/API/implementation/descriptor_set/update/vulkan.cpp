#include "./factory/API/unified/device.hpp"
#include "./factory/API/unified/descriptor_set.hpp"
#include "./classes/API/unified/descriptor_set.hpp"


namespace svt {
    namespace api {
        namespace classes {
#ifdef USE_VULKAN
            // TODO: add RTX description_write support
            svt::core::handle_ref<descriptor_set,core::api::result_t> descriptor_set::update(const descriptor_set_update_info& info) {
                // If not exists, create it
                if (!this->descriptor_set_) { this->create(info); };

                vk::DescriptorUpdateTemplateCreateInfo vk_info{};
                vk_info.templateType = vk::DescriptorUpdateTemplateType::eDescriptorSet;
                vk_info.flags = {};
                vk_info.descriptorUpdateEntryCount = info.entries_.size();
                vk_info.pDescriptorUpdateEntries = (vk::DescriptorUpdateTemplateEntry*)info.entries_.data();
                vk_info.descriptorSetLayout = *descriptor_set_layout_;

                // IGNORE due isn't push descriptor 
                //info.pipelineBindPoint = 0u;
                vk_info.pipelineLayout = nullptr;
                vk_info.set = {};

                // TODO: destroy previous template 
                (*device_)->updateDescriptorSetWithTemplate(*descriptor_set_,descriptor_set_->temp_=(*device_)->createDescriptorUpdateTemplate(vk_info,nullptr,*device_),info.heap_.data(),*device_); // 
                return {*this,core::api::result_t(0u)};
            };
#endif
        };
    };
};
