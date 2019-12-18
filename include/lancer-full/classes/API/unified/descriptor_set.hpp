#pragma once

#include "./classes/API/types.hpp"
#include "./classes/API/unified/device.hpp"
#include "./factory/API/unified/descriptor_set.hpp"
#include "./factory/API/unified/descriptor_set_layout.hpp"
#include "./classes/API/unified/descriptor_set_layout.hpp"

namespace svt {
    namespace api {
        namespace classes {

            class descriptor_set { public: 

                template<class T>
                inline description_handle push_description( const description_entry& entry_t );

                // TODO: create descriptor set method
                svt::core::handle_ref<descriptor_set,core::api::result_t> update();
                svt::core::handle_ref<descriptor_set,core::api::result_t> create();

            protected: 
                stu::descriptor_set descriptor_set_t = {};
                stu::descriptor_set_layout descriptor_set_layout_t = {};
                stu::device device_t = {};
                std::vector<uint8_t> heap = {};
                std::vector<vk::DescriptorUpdateTemplateEntry> entries = {};
            };
        };
    };
};
