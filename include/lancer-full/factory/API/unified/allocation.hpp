#pragma once

#include "./core/unified/core.hpp"
#include "./factory/API/types.hpp"

namespace svt {
    namespace api {
        namespace factory {
            // original class (also, memory can be extracted from buffer or image)
            class allocation_t : public std::enable_shared_from_this<allocation_t> { public: 
                //allocation_t() {};

                virtual ~allocation_t(){};
                virtual uintptr_t get_allocation();
                virtual uintptr_t get_allocation_info();
                virtual void* mapped();
                virtual void* map();
                virtual void unmap(void* ptr = nullptr);

                
            };
        };
    };
};
