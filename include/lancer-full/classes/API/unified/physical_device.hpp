#pragma once

#include "./classes/API/types.hpp"
#include "./factory/API/unified/physical_device.hpp"

namespace svt {
    namespace api {
        namespace classes {
            class physical_device { public: 
                physical_device(const physical_device& physical_device) : physical_device_(physical_device) {};
                physical_device(const stu::physical_device_t& physical_device_ = {}) : physical_device_(physical_device_) {};

                // TODO: add assigment by core types and shared_ptr types
                physical_device& operator=(const physical_device &physical_device) { this->physical_device_ = physical_device; return *this; };

                // TODO: move into `.cpp` file
                operator stu::physical_device&() { return physical_device_; };
                operator stu::physical_device_t&() { return physical_device_; };
                operator const stu::physical_device&() const { return physical_device_; };
                operator const stu::physical_device_t&() const { return physical_device_; };

                api::factory::physical_device_t* operator->() { return &(*this->physical_device_); };
                const api::factory::physical_device_t* operator->() const { return &(*this->physical_device_); };
                api::factory::physical_device_t& operator*() { return (*this->physical_device_); };
                const api::factory::physical_device_t& operator*() const { return (*this->physical_device_); };

                // TODO: move into `.cpp`
                surface_capabilities get_surface_capabilities(const core::api::surface_t& surface) const {
                    return physical_device_->get_surface_capabilities(surface);
                };

                std::vector<present_mode> get_present_modes(const core::api::surface_t& surface) const {
                    return physical_device_->get_present_modes(surface);
                };

                format_properties get_format_properties(const format& format){
                    return physical_device_->get_format_properties(format);
                };

                surface_format get_surface_formats(const core::api::surface_t& surface) {
                    return physical_device_->get_surface_formats(surface);
                };

                // 
                operator uintptr_t&() { return physical_device_; };
                operator const uintptr_t&() const { return physical_device_; };
                
                operator core::api::physical_device_t&() { return physical_device_; };
                operator const core::api::physical_device_t&() const { return physical_device_; };

            protected: friend physical_device;
                stu::physical_device_t physical_device_ = {};
            };
        };
    };
};
