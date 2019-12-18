#include "./factory/API/unified/device.hpp"
#include "./factory/API/unified/descriptor_set.hpp"
#include "./classes/API/unified/descriptor_set.hpp"


namespace svt {
    namespace api {
        namespace classes {
#ifdef USE_VULKAN
            // TODO: add finally apply method support
            // TODO: add RTX description_write support
            svt::core::handle_ref<descriptor_set,core::api::result_t> descriptor_set::update() {
                vk::DescriptorUpdateTemplateCreateInfo info{};
                info.templateType = vk::DescriptorUpdateTemplateType::eDescriptorSet;
                info.flags = {};
                info.descriptorUpdateEntryCount = entries_.size();
                info.pDescriptorUpdateEntries = entries_.data();
                info.descriptorSetLayout = descriptor_set_layout_->layout;

                // IGNORE due isn't push descriptor 
                //info.pipelineBindPoint = 0u;
                info.pipelineLayout = nullptr;
                info.set = {};

                // 
                (*device_)->createDescriptorUpdateTemplate(&info,nullptr,&descriptor_set_->temp); // TODO: destroy previous template 
                (*device_)->updateDescriptorSetWithTemplate(descriptor_set_->set,descriptor_set_->temp,heap_.data()); // 
                return {*this,core::api::result_t(0u)};
            };
#endif
        };
    };
};
