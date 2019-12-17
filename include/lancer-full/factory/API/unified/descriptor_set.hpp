#pragma once

#include "./core/unified/core.hpp"
#include "./factory/API/types.hpp"

namespace svt {
    namespace api {
        namespace factory {
            class descriptor_set_t : public std::enable_shared_from_this<descriptor_set_t> { public: 
                core::api::descriptor_set_t set = {};
                core::api::descriptor_set_template_t temp = {};

                descriptor_set_t(const descriptor_set_t& descriptor_set_t) : set(descriptor_set_t), temp(descriptor_set_t) {};
                descriptor_set_t& operator=(const descriptor_set_t& descriptor_set_t) { set = descriptor_set_t, temp = descriptor_set_t; return *this; };

                operator core::api::descriptor_set_t&() {return set; };
                operator core::api::descriptor_set_template_t&() {return temp; };
                operator const core::api::descriptor_set_t&() const {return set; };
                operator const core::api::descriptor_set_template_t&() const {return temp; };

                core::api::descriptor_set_t* operator->() { return &(this->set); };
                const core::api::descriptor_set_t* operator->() const { return &(this->set); };
            };
        };
    };
};
