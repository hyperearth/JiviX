#pragma once

#include "./core/unified/core.hpp"
#include "./factory/API/types.hpp"
#include "./factory/VMA/types.hpp"
#include "./factory/API/unified/enums.hpp"
#include <vma/vk_mem_alloc.h>


namespace svt {
    namespace vma {
        namespace factory {
            struct internal_allocation_t {
                VmaAllocator allocator_ = {};
                VmaAllocation allocation_ = {};
                VmaAllocationInfo allocation_info_ = {};
            };

            using internal_allocation = std::unique_ptr<internal_allocation_t>;
        };
    };
};
