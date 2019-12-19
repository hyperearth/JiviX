#pragma once

#include "./core/unified/core.hpp"
#include "./factory/API/types.hpp"
#include "./factory/VMA/types.hpp"
#include "./factory/API/unified/allocation.hpp"
#include "./factory/VMA/unified/enums.hpp"

namespace svt {
    namespace vma {
        namespace factory {

            class allocation_t : public svt::api::factory::allocation_t { public: 
                internal_allocation allocation_ = {};
                allocation_t(internal_allocation allocation_ = {}) : allocation_(std::move(allocation_)) {};
                virtual ~allocation_t() override { if (allocation_) vmaFreeMemory(allocation_->allocator, allocation_->allocation); allocation_ = {}; };
                virtual uintptr_t get_allocation() override { return uintptr_t(&allocation_->allocation_); };
                virtual uintptr_t get_allocation_info() override { return uintptr_t(&allocation_->allocation_info_); };
                virtual void* mapped() override { return allocation_->allocation_info_->pMappedData; };
                virtual void* map() override { void* map = nullptr; vmaMapMemory(allocation_->allocator, allocation_->allocation_, &map); return map; };
                virtual void unmap(void* ptr = nullptr) override { vmaUnmapMemory(allocation_->allocator, allocation_->allocation_); };
            };

        };
    };
};
