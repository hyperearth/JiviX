#pragma once

#include "./core/unified/core.hpp"
#include "./dynamic/API/types.hpp"

namespace svt {
    namespace api {
        namespace dynamic {

            // Can Be Extended By VMA Allocators
            class buffer_t : public std::enable_shared_from_this<buffer_t> { public: 
                //std::vector<uint32_t> queueFamilyIndices = {};
                core::api::buffer_t buffer;

                // custom destructor for inheritance
                virtual ~buffer_t(){};

                // due std::shared_ptr<data::dynamic::buffer_t> is dynamic, should contain some general methods
                virtual uintptr_t get_allocation();
                virtual uintptr_t get_allocation_info();
                virtual void* get_mapped();
                virtual void* map();
                virtual void unmap(void* ptr = nullptr);

                operator uintptr_t&() { return (uintptr_t&)(this->buffer); };
                operator const uintptr_t&() const { return (uintptr_t&)(this->buffer); };
            };

        };
    };
};
