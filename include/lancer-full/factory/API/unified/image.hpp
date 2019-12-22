#pragma once

#include "./core/unified/core.hpp"
#include "./factory/API/types.hpp"

namespace svt {
    namespace api {
        namespace factory {
            // Can Be Extended By VMA Allocators
            class image_t : public std::enable_shared_from_this<image_t> {
            public:
                //std::vector<uint32_t> queueFamilyIndices = {};
                core::api::image_t image_ = API_NULL_HANDLE;
                image_layout layout_ = image_layout::t_undefined;

                image_t() {};
                image_t(const image_t& image) : image_(image) {};
                image_t(const core::api::image_t& image_) : image_(image_) {};
                image_t& operator=(const image_t& image) { this->image_ = image; return *this; };

                operator uintptr_t&() { return (uintptr_t&)(image_); };
                operator const uintptr_t&() const { return (uintptr_t&)(image_); };
                operator core::api::image_t& () { return image_; };
                operator const core::api::image_t& () const { return image_; };

                core::api::image_t* operator->() { return &(this->image_); };
                const core::api::image_t* operator->() const { return &(this->image_); };
                core::api::image_t& operator*() { return (this->image_); };
                const core::api::image_t& operator*() const { return (this->image_); };

                // TODO: import/claim allocation
                // export allocation ONCE as dedicated (should to be unbound from that object)
                virtual std::shared_ptr<allocation_t> export_allocation();

                // 
                virtual ~image_t() {};
                virtual uintptr_t get_allocation();
                virtual uintptr_t get_allocation_info();
                virtual void* mapped();
                virtual void* map();
                virtual void unmap(void* ptr = nullptr);
            };
        };
    };
};
