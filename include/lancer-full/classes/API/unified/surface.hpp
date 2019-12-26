#pragma once

#include "./classes/API/types.hpp"
#include "./factory/API/unified/device.hpp"
#include "./factory/API/unified/surface.hpp"

namespace svt {
    namespace api {
        namespace classes {
            class surface { public: 
                surface(const surface& surface) : surface_(surface) {};
                surface(const stu::surface& surface_ = {}) : surface_(surface_) {};

                // TODO: add assigment by core types and shared_ptr types
                surface& operator=(const surface &surface) { this->surface_ = surface; return *this; };

                // TODO: move into `.cpp` file
                operator stu::surface&() { return surface_; };
                operator stu::device&() { return device_; };
                operator stu::device_t&() { return device_; };
                operator const stu::surface&() const { return surface_; };
                operator const stu::device&() const { return device_; };
                operator const stu::device_t&() const { return device_; };

                api::factory::surface_t* operator->() { return &(*this->surface_); };
                const api::factory::surface_t* operator->() const { return &(*this->surface_); };
                api::factory::surface_t& operator*() { return (*this->surface_); };
                const api::factory::surface_t& operator*() const { return (*this->surface_); };

                // 
                operator uintptr_t&() { return (*surface_); };
                operator const uintptr_t&() const { return (*surface_); };
                
                operator core::api::device_t&() { return device_; };
                operator const core::api::device_t&() const { return device_; };

                operator core::api::physical_device_t&() { return device_; };
                operator const core::api::physical_device_t&() const { return device_; };

                operator core::api::surface_t&() { return *surface_; };
                operator const core::api::surface_t&() const { return *surface_; };
                
            //
            protected: friend surface;
                stu::surface surface_ = {};
                stu::device_t device_ = {};
            };
        };
    };
};
