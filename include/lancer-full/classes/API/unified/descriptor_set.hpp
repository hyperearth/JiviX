#pragma once

#include "./classes/API/types.hpp"
#include "./classes/API/unified/device.hpp"
#include "./classes/API/unified/descriptor_set_layout.hpp"
#include "./factory/API/unified/descriptor_set.hpp"

namespace svt {
    namespace api {
        namespace classes {

            class descriptor_set {
                protected: 
                    stu::descriptor_set descriptor_set_t = {};
                    stu::descriptor_set_layout descriptor_set_layout_t = {};
                    stu::device device_t = {};
                    std::vector<uint8_t> heap = {};
                    std::vector<vk::DescriptorUpdateTemplateEntry> entries = {};
                    
                public: 

                    enum class type : uint32_t {
                        t_sampler = 0u,
                        t_combined_image_sampler = 1u,
                        t_sampled_image = 2u,
                        t_storage_image = 3u,
                        t_uniform_texel_buffer = 4u,
                        t_storage_texel_buffer = 5u,
                        t_uniform_buffer = 6u,
                        t_storage_buffer = 7u,
                        t_uniform_buffer_dynamic = 8u,
                        t_storage_buffer_dynamic = 9u,
                        t_input_attachment = 10u
                    };

                    struct entry {
                        type type = type::t_sampler;
                        uint32_t dst_binding = 0u;
                        uint32_t dst_item_id = 0u;
                        uint32_t descs_count = 1u;
                    };

                    // TODO: typing control, add into `.cpp` file
                    struct cpu_handle {
                        entry* entry_t = nullptr;
                        void* field_t = nullptr;

                        // any buffers and images can `write` into types
                        template<class T = uint8_t> operator T&() { return (T*)field_t; };
                        template<class T = uint8_t> operator const T&() const { return (T*)field_t; };
                        template<class T = uint8_t> T& handle() { return (T*)field_t; };
                        template<class T = uint8_t> const T& handle() const { return (T*)field_t; };
                        uint32_t& size() { return entry_t->descs_count; };
                    };

                    // TODO: add into `.cpp` file
                    //using T = vk::DescriptorImageInfo;
                    template<class T>
                    inline T* _add_desc( const entry& entry_t ) { // Un-Safe API again
                        const uintptr_t pt0 = heap.size();
                        heap.resize(pt0+sizeof(T)*entry_t.descriptor_count, 0u);
                        entries.push_back(vk::DescriptorUpdateTemplateEntry{entry_t.dst_binding,entry_t.dst_array_element,entry_t.descs_count,(vk::DescriptorType)(entry_t),pt0,sizeof(T)});
                        return (T*)(&heap[pt0]);
                    };

                    // TODO: add finally apply method support
                    // TODO: add into `.cpp` file
                    // TODO: add RTX description_write support
                    void apply() {
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

                        device_t->device.createDescriptorUpdateTemplate(&info,nullptr,&descriptor_set_t->temp); // TODO: destroy previous template 
                        device_t->device.updateDescriptorSetWithTemplate(descriptor_set_t->set,descriptor_set_t->temp,heap.data()); // 
                    };
                    
            };
        };
    };
};
