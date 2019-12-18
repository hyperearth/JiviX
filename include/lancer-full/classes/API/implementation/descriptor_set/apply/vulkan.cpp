#include "./factory/API/unified/device.hpp"
#include "./factory/API/unified/descriptor_set.hpp"
#include "./classes/API/unified/descriptor_set.hpp"


namespace svt {
    namespace api {
        namespace classes {
#ifdef USE_VULKAN
            // TODO: add finally apply method support
            // TODO: add RTX description_write support
            void descriptor_set::apply() {
                vk::DescriptorUpdateTemplateCreateInfo info{};
                info.templateType = vk::DescriptorUpdateTemplateType::eDescriptorSet;
                info.flags = {};
                info.descriptorUpdateEntryCount = entries.size();
                info.pDescriptorUpdateEntries = entries.data();
                info.descriptorSetLayout = descriptor_set_layout_t->layout;

                // IGNORE due isn't push descriptor 
                //info.pipelineBindPoint = 0u;
                info.pipelineLayout = nullptr;
                info.set = {};

                // 
                (*device_t)->createDescriptorUpdateTemplate(&info,nullptr,&descriptor_set_t->temp); // TODO: destroy previous template 
                (*device_t)->updateDescriptorSetWithTemplate(descriptor_set_t->set,descriptor_set_t->temp,heap.data()); // 
            };
#endif
        };
    };
};
