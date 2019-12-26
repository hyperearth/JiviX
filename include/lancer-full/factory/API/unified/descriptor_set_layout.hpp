#pragma once

#include "./core/unified/core.hpp"
#include "./factory/API/types.hpp"

namespace svt {
    namespace api {
        namespace factory {
            class descriptor_set_layout_t : public std::enable_shared_from_this<descriptor_set_layout_t> { public: 
                core::api::descriptor_set_layout_t layout_ = API_NULL_HANDLE;

                descriptor_set_layout_t() {};
                descriptor_set_layout_t(const descriptor_set_layout_t& descriptor_set_layout_t) : layout_(descriptor_set_layout_t) {};
                descriptor_set_layout_t(core::api::descriptor_set_layout_t layout_ = API_NULL_HANDLE) : layout_(layout_) {};
                descriptor_set_layout_t& operator=(const descriptor_set_layout_t& descriptor_set_layout) { this->layout_ = descriptor_set_layout; return *this; };
                descriptor_set_layout_t& operator=(const std::shared_ptr<descriptor_set_layout_t>& descriptor_set_layout) { this->layout_ = *descriptor_set_layout; return *this; };
                descriptor_set_layout_t& operator=(const core::api::descriptor_set_layout_t& layout_){ this->layout_ = layout_; return *this; };

                operator uintptr_t&() { return (uintptr_t&)(layout_); };
                operator const uintptr_t&() const { return (uintptr_t&)(layout_); };
                operator core::api::descriptor_set_layout_t&() { return layout_; };
                operator const core::api::descriptor_set_layout_t&() const { return layout_; };

                core::api::descriptor_set_layout_t* operator->() { return &(this->layout_); };
                const core::api::descriptor_set_layout_t* operator->() const { return &(this->layout_); };
                core::api::descriptor_set_layout_t& operator*() { return (this->layout_); };
                const core::api::descriptor_set_layout_t& operator*() const { return (this->layout_); };
            };
        };
    };
};
