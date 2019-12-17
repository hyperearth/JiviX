#pragma once

#include "./core/unified/core.hpp"
#include "./factory/API/types.hpp"

namespace svt {
    namespace api {
        namespace factory {
            class descriptor_set_layout_t : public std::enable_shared_from_this<descriptor_set_layout_t> { public: 
                core::api::descriptor_set_layout_t layout = {};

                descriptor_set_layout_t(const descriptor_set_layout_t& descriptor_set_layout_t) : layout(descriptor_set_layout_t) {};
                descriptor_set_layout_t& operator=(const descriptor_set_layout_t& descriptor_set_layout_t) { layout = descriptor_set_layout_t; return *this; };

                operator core::api::descriptor_set_layout_t&() { return layout; };
                operator const core::api::descriptor_set_layout_t&() const { return layout; };

                core::api::descriptor_set_layout_t* operator->() { return &(this->layout); };
                const core::api::descriptor_set_layout_t* operator->() const { return &(this->layout); };
                
            };
        };
    };
};
