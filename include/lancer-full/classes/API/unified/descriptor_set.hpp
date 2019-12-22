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
                descriptor_set(const descriptor_set& descriptor_set) : device_(descriptor_set), descriptor_set_(descriptor_set) {};
                descriptor_set(                              const stu::descriptor_set& descriptor_set_ = {}) : descriptor_set_(descriptor_set_) {};
                descriptor_set(const stu::device_t& device_, const stu::descriptor_set& descriptor_set_ = {}) : descriptor_set_(descriptor_set_), device_(device_) {};

                // 
                operator stu::descriptor_set_layout&() { return descriptor_set_layout_; };
                operator stu::descriptor_set&() { return descriptor_set_; };
                operator stu::device&() { return device_; };
                operator stu::device_t&() { return device_; };
                operator const stu::descriptor_set_layout&() const { return descriptor_set_layout_; };
                operator const stu::descriptor_set&() const { return descriptor_set_; };
                operator const stu::device&() const { return device_; };
                operator const stu::device_t&() const { return device_; };

                // 
                api::factory::descriptor_set_layout_t* operator->() { return &(*this->descriptor_set_layout_); };
                const api::factory::descriptor_set_layout_t* operator->() const { return &(*this->descriptor_set_layout_); };

                // 
                descriptor_set& operator=(const descriptor_set &descriptor_set) { 
                    this->descriptor_set_layout_ = descriptor_set;
                    this->descriptor_set_ = descriptor_set;
                    this->device_ = descriptor_set;
                    this->heap_ = descriptor_set.heap_;
                    this->entries_ = descriptor_set.entries_;
                    return *this;
                };


                template<class T>
                inline description_handle _push_description( const description_entry& entry_ = {} );

                // official function (not template)
                description_handle  push_description( const description_entry& entry_ = {} );


                // TODO: create descriptor set method
                svt::core::handle_ref<descriptor_set,core::api::result_t> update();
                svt::core::handle_ref<descriptor_set,core::api::result_t> create();

                // 
                operator uintptr_t&() { return (*descriptor_set_); };
                operator const uintptr_t&() const { return (*descriptor_set_); };
                operator core::api::descriptor_set_t&() { return (*descriptor_set_); };
                operator const core::api::descriptor_set_t&() const { return (*descriptor_set_); };

            protected: 
                stu::descriptor_set descriptor_set_ = {};
                stu::descriptor_set_layout descriptor_set_layout_ = {};
                std::vector<uint8_t> heap_ = {};
                std::vector<vk::DescriptorUpdateTemplateEntry> entries_ = {};
                stu::device_t device_ = {};
            };
        };
    };
};
