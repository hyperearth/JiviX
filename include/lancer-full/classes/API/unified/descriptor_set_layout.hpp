#pragma once

#include "./classes/API/types.hpp"
#include "./factory/API/unified/descriptor_set_layout.hpp"
namespace svt {
    namespace api {
        namespace classes {

            // TODO: complete descriptor layout entries
            // TODO: add implementation into `.cpp`
            class descriptor_set_layout {
                protected: friend descriptor_set;
                    stu::descriptor_set_layout descriptor_set_layout_t = {};
                    stu::device device_t = {};
                    
                public: 
                    descriptor_set_layout& operator=(const descriptor_set_layout &descriptor_set_layout) { 
                        this->descriptor_set_layout_t = descriptor_set_layout;
                        this->device_t = descriptor_set_layout;
                        return *this;
                    };

                    // 
                    operator stu::descriptor_set_layout&() { return descriptor_set_layout_t; };
                    operator stu::device&() { return device_t; };
                    operator const stu::descriptor_set_layout&() const { return descriptor_set_layout_t; };
                    operator const stu::device&() const { return device_t; };

                    // 
                    api::factory::descriptor_set_layout_t* operator->() { return &(*this->descriptor_set_layout_t); };
                    const api::factory::descriptor_set_layout_t* operator->() const { return &(*this->descriptor_set_layout_t); };
            };
        };
    };
};
