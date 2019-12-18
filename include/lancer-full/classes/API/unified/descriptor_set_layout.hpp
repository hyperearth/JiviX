#pragma once

#include "./classes/API/types.hpp"
#include "./classes/API/unified/device.hpp"
#include "./factory/API/unified/descriptor_set_layout.hpp"
#include "./factory/API/unified/descriptor_set.hpp"

namespace svt {
    namespace api {
        namespace classes {

            // TODO: push bindings, full version
            // TODO: complete descriptor layout entries
            // TODO: add implementation into `.cpp`
            class descriptor_set_layout { public: 
                descriptor_set_layout(const descriptor_set_layout& descriptor_set_layout) : device_(descriptor_set_layout), descriptor_set_layout_(descriptor_set_layout) {};
                descriptor_set_layout(                            const stu::descriptor_set_layout& descriptor_set_layout_ = {}) : descriptor_set_layout_(descriptor_set_layout_) {};
                descriptor_set_layout(const stu::device& device_, const stu::descriptor_set_layout& descriptor_set_layout_ = {}) : descriptor_set_layout_(descriptor_set_layout_), device_(device_) {};

                // 
                descriptor_set_layout& operator=(const descriptor_set_layout &descriptor_set_layout) { 
                    this->descriptor_set_layout_ = descriptor_set_layout;
                    this->device_ = descriptor_set_layout;
                    return *this;
                };

                // 
                operator stu::descriptor_set_layout&() { return descriptor_set_layout_; };
                operator stu::device&() { return device_; };
                operator const stu::descriptor_set_layout&() const { return descriptor_set_layout_; };
                operator const stu::device&() const { return device_; };

                // 
                api::factory::descriptor_set_layout_t* operator->() { return &(*this->descriptor_set_layout_); };
                const api::factory::descriptor_set_layout_t* operator->() const { return &(*this->descriptor_set_layout_); };

                // 
                svt::core::handle_ref<descriptor_set_layout,core::api::result_t> create(const uint32_t& flags = 0u);

            protected: friend descriptor_set; friend descriptor_set_layout;
                stu::descriptor_set_layout descriptor_set_layout_ = {};
                stu::device device_ = {};
                std::vector<description_binding> bindings_ = {};
            };


        };
    };
};
