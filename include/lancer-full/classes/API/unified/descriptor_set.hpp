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
                        template<class T = uint8_t> operator T&() { return (*field_t); };
                        template<class T = uint8_t> operator const T&() const { return (*field_t); };
                        template<class T = uint8_t> T& offset(const uint32_t& idx = 0u) { return cpu_handle{entry_t+idx,(T*)field_t}; };
                        template<class T = uint8_t> const T& offset(const uint32_t& idx = 0u) const { return cpu_handle{entry_t+idx,(T*)field_t}; };
                        const uint32_t& size() const { return entry_t->descs_count; };
                        
                        // 
                        
                    };

                    // TODO: add into `.cpp` file
                    template<class T>
                    inline cpu_handle add_description( const entry& entry_t );

                    // 
                    svt::core::handle_ref<descriptor_set,core::api::result_t> update();
                    
            };
        };
    };
};
