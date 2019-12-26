#pragma once

#include "./core/unified/core.hpp"
#include "./factory/API/types.hpp"

namespace svt {
    namespace api {
        namespace factory {
            // Can Be Extended By VMA Allocators
            class acceleration_structure_t : public std::enable_shared_from_this<acceleration_structure_t> { public: 
                //std::vector<uint32_t> queueFamilyIndices = {};
                core::api::acceleration_structure_t structure_ = API_NULL_HANDLE;

                operator uintptr_t&() { return (uintptr_t&)(this->structure_); };
                operator const uintptr_t&() const { return (uintptr_t&)(this->structure_); };

                // 
                acceleration_structure_t() {};
                acceleration_structure_t(const acceleration_structure_t& acceleration_structure) : structure_(acceleration_structure) {};
                acceleration_structure_t(const core::api::acceleration_structure_t& structure_) : structure_(structure_) {};
                acceleration_structure_t& operator=(const acceleration_structure_t& acceleration_structure) { this->structure_ = acceleration_structure; return *this; };
                acceleration_structure_t& operator=(const std::shared_ptr<acceleration_structure_t>& acceleration_structure) { this->structure_ = *acceleration_structure; return *this; };
                acceleration_structure_t& operator=(const core::api::acceleration_structure_t& structure_){ this_structure_ = structure_; return *this; };

                // 
                operator uintptr_t&() { return (uintptr_t&)(structure_); };
                operator const uintptr_t&() const { return (uintptr_t&)(structure_); };
                operator core::api::acceleration_structure_t&() { return structure_; };
                operator const core::api::acceleration_structure_t&() const { return structure_; };

                // 
                core::api::acceleration_structure_t* operator->() { return &(this->structure_); };
                const core::api::acceleration_structure_t* operator->() const { return &(this->structure_); };
                core::api::acceleration_structure_t& operator*() { return (this->structure_); };
                const core::api::acceleration_structure_t& operator*() const { return (this->structure_); };

                // TODO: import/claim allocation
                // export allocation ONCE as dedicated (should to be unbound from that object)
                virtual std::shared_ptr<allocation_t> export_allocation();

                // 
                virtual ~acceleration_structure_t(){};
                virtual uintptr_t get_allocation();
                virtual uintptr_t get_allocation_info();
                virtual void* mapped();
                virtual void* map();
                virtual void unmap(void* ptr = nullptr);
            };

        };
    };
};
