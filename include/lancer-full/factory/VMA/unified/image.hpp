#pragma once

#include "./core/unified/core.hpp"
#include "./factory/API/types.hpp"
#include "./factory/VMA/types.hpp"
#include "./factory/API/unified/image.hpp"
#include "./factory/VMA/unified/enums.hpp"

namespace svt {
    namespace vma {
        namespace factory {
            // Can Be Extended By VMA Allocators
            class image_t : public svt::api::factory::image_t { public:
                internal_allocation allocation_ = {};

                image_t(const image_t& image) : svt::api::factory::image_t(image) {};
                image_t(const core::api::image_t& image_) : svt::api::factory::image_t(image_) {};
                image_t& operator=(const image_t& image) { this->image_ = image; return *this; };

                operator core::api::image_t& () { return image_; };
                operator const core::api::image_t& () const { return image_; };

                core::api::image_t* operator->() { return &(this->image_); };
                const core::api::image_t* operator->() const { return &(this->image_); };

                // export allocation ONCE as dedicated (should to be unbound from that object)
                virtual std::shared_ptr<svt::api::factory::allocation_t> export_allocation() override {
                    return std::dynamic_pointer_cast<svt::api::factory::allocation_t>(std::make_shared<allocation_t>(std::move(allocation_)));
                };

                // 
                virtual ~image_t() override {  };
                virtual uintptr_t get_allocation() override {  };
                virtual uintptr_t get_allocation_info() override {  };
                virtual void* get_mapped() override {  };
                virtual void* map() override {  };
                virtual void unmap(void* ptr = nullptr) override {  };
            };
        };
    };
};
