#pragma once

#include "./core/unified/core.hpp"
#include "./factory/API/types.hpp"

namespace svt {
    namespace api {
        namespace factory {

            // Can Be Extended By VMA Allocators
            class buffer_t : public std::enable_shared_from_this<buffer_t> { public: 
                //std::vector<uint32_t> queueFamilyIndices = {};
                core::api::buffer_t buffer_ = API_NULL_HANDLE;


                operator uintptr_t&() { return (uintptr_t&)(this->buffer_); };
                operator const uintptr_t&() const { return (uintptr_t&)(this->buffer_); };

                buffer_t(const buffer_t& buffer) : buffer_(buffer_) {};
                buffer_t(const core::api::buffer_t& buffer_) : buffer_(buffer_) {};
                buffer_t& operator=(const buffer_t& buffer) { this->buffer_ = buffer_; return *this; };

                operator core::api::buffer_t&() { return buffer_; };
                operator const core::api::buffer_t&() const { return buffer_; };

                core::api::buffer_t* operator->() { return &(this->buffer_); };
                const core::api::buffer_t* operator->() const { return &(this->buffer_); };

                // export allocation ONCE as dedicated (should to be unbound from that object)
                virtual std::shared_ptr<allocation_t> export_allocation();

                // 
                virtual ~buffer_t(){};
                virtual uintptr_t get_allocation();
                virtual uintptr_t get_allocation_info();
                virtual void* get_mapped();
                virtual void* map();
                virtual void unmap(void* ptr = nullptr);
            };

        };
    };
};
