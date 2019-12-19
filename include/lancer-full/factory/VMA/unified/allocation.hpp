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
                allocation_t(internal_allocation&& allocation_ = {}) : allocation_(allocation_) {};
                virtual ~allocation_t() override {};
                virtual uintptr_t get_allocation() override;
                virtual uintptr_t get_allocation_info() override;
                virtual void* get_mapped() override;
                virtual void* map() override;
                virtual void unmap(void* ptr = nullptr) override;
            };

        };
    };
};
