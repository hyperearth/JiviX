#pragma once

#include "./core/unified/core.hpp"
#include "./factory/API/types.hpp"
#include "./factory/VMA/types.hpp"
#include "./factory/API/unified/buffer.hpp"
#include "./factory/VMA/unified/enums.hpp"

namespace svt {
    namespace vma {
        namespace factory {

            // Can Be Extended By VMA Allocators
            class buffer_t : public api::factory::buffer_t { public: 
                internal_allocation allocation_ = {};

                operator uintptr_t&() { return (uintptr_t&)(this->buffer_); };
                operator const uintptr_t&() const { return (uintptr_t&)(this->buffer_); };

                buffer_t(const buffer_t& buffer) : api::factory::buffer_t(buffer) {};
                buffer_t(const core::api::buffer_t& buffer_) : api::factory::buffer_t(buffer_) {};
                buffer_t& operator=(const buffer_t& buffer) { this->buffer_ = buffer_; return *this; };

                operator core::api::buffer_t&() { return buffer_; };
                operator const core::api::buffer_t&() const { return buffer_; };

                core::api::buffer_t* operator->() { return &(this->buffer_); };
                const core::api::buffer_t* operator->() const { return &(this->buffer_); };

                // TODO: import/claim allocation
                // export allocation ONCE as dedicated (should to be unbound from that object)
                virtual std::shared_ptr<api::factory::allocation_t> export_allocation() override {
                    return std::dynamic_pointer_cast<api::factory::allocation_t>(std::make_shared<allocation_t>(std::move(allocation_)));
                };

                // 
                virtual ~buffer_t() override { if (allocation_) vmaDestroyBuffer(allocation_->allocator, buffer_, allocation_->allocation_); allocation_ = {}; };
                virtual uintptr_t get_allocation() override { return uintptr_t(&allocation_->allocation_); };
                virtual uintptr_t get_allocation_info() override { return uintptr_t(&allocation_->allocation_info_); };
                virtual void* mapped() override { return allocation_->allocation_info_->pMappedData; };
                virtual void* map() override { void* map = nullptr; vmaMapMemory(allocation_->allocator, allocation_->allocation_, &map); return map; };
                virtual void unmap(void* ptr = nullptr) override { vmaUnmapMemory(allocation_->allocator, allocation_->allocation_); };
            };

        };
    };
};
