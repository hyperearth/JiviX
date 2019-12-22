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
            class image_t : public api::factory::image_t { public:
                internal_allocation allocation_ = {};

                image_t(const image_t& image) : api::factory::image_t(image) {};
                image_t(const core::api::image_t& image_) : api::factory::image_t(image_) {};
                image_t& operator=(const image_t& image) { this->image_ = image; return *this; };

                operator core::api::image_t& () { return image_; };
                operator const core::api::image_t& () const { return image_; };
                operator uintptr_t&() { return (uintptr_t&)(image_); };
                operator const uintptr_t&() const { return (uintptr_t&)(image_); };

                core::api::image_t* operator->() { return &(this->image_); };
                const core::api::image_t* operator->() const { return &(this->image_); };

                // TODO: import/claim allocation
                // export allocation ONCE as dedicated (should to be unbound from that object)
                virtual std::shared_ptr<api::factory::allocation_t> export_allocation() override {
                    return std::dynamic_pointer_cast<api::factory::allocation_t>(std::make_shared<allocation_t>(std::move(allocation_)));
                };

                // 
                virtual ~image_t() override { if (allocation_) vmaDestroyImage(allocation_->allocator, image_, allocation_->allocation_); allocation_ = {}; };
                virtual uintptr_t get_allocation() override { return uintptr_t(&allocation_->allocation_); };
                virtual uintptr_t get_allocation_info() override { return uintptr_t(&allocation_->allocation_info_); };
                virtual void* mapped() override { return allocation_->allocation_info_->pMappedData; };
                virtual void* map() override { void* map = nullptr; vmaMapMemory(allocation_->allocator, allocation_->allocation_, &map); return map; };
                virtual void unmap(void* ptr = nullptr) override { vmaUnmapMemory(allocation_->allocator, allocation_->allocation_); };
            };
        };
    };
};
