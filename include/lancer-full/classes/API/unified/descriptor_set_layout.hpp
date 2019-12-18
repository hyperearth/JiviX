#pragma once

#include "./classes/API/types.hpp"
#include "./classes/API/unified/descriptor_set.hpp"
#include "./factory/API/unified/descriptor_set_layout.hpp"

namespace svt {
    namespace api {
        namespace classes {

            // TODO: complete descriptor layout entries
            // TODO: add implementation into `.cpp`
            class descriptor_set_layout {
                
                public: 
                    struct bindings {
                        uint32_t binding = 0u;

                        // TODO: resolve header ordering conflicts
                        descriptor_set::type type = descriptor_set::type::t_sampler;

                        // 
                        uint32_t count = 1u;

                        // TODO: shader stage flags type
                        uint32_t shader_stages = 1u;

                        // TODO: immutable samplers
                        //api::factory::sampler_t sampler;
                        uintptr_t samplers = 0u;
                        
                        // TODO: flags EXT type support
                        uint32_t flags_ext = 0u;
                    };

                    descriptor_set_layout& operator=(const descriptor_set_layout &descriptor_set_layout) { 
                        this->descriptor_set_layout_t = descriptor_set_layout;
                        this->device_t = descriptor_set_layout;
                        return *this;
                    };

                    // TODO: push bindings, full version
                    

                    // 
                    operator stu::descriptor_set_layout&() { return descriptor_set_layout_t; };
                    operator stu::device&() { return device_t; };
                    operator const stu::descriptor_set_layout&() const { return descriptor_set_layout_t; };
                    operator const stu::device&() const { return device_t; };

                    // 
                    api::factory::descriptor_set_layout_t* operator->() { return &(*this->descriptor_set_layout_t); };
                    const api::factory::descriptor_set_layout_t* operator->() const { return &(*this->descriptor_set_layout_t); };

                    // 
                    svt::core::handle_ref<descriptor_set_layout,core::api::result_t> create(const uint32_t& flags = 0u);

                protected: friend descriptor_set;
                    stu::descriptor_set_layout descriptor_set_layout_t = {};
                    stu::device device_t = {};
                    std::vector<bindings> bindings_t = {};
            };


        };
    };
};
