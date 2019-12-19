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
            class buffer_t : public svt::api::factory::buffer_t { public: 
                internal_allocation allocation_ = {};

                operator uintptr_t&() { return (uintptr_t&)(this->buffer_); };
                operator const uintptr_t&() const { return (uintptr_t&)(this->buffer_); };

                buffer_t(const buffer_t& buffer) : svt::api::factory::buffer_t(buffer) {};
                buffer_t(const core::api::buffer_t& buffer_) : svt::api::factory::buffer_t(buffer_) {};
                buffer_t& operator=(const buffer_t& buffer) { this->buffer_ = buffer_; return *this; };

                operator core::api::buffer_t&() { return buffer_; };
                operator const core::api::buffer_t&() const { return buffer_; };

                core::api::buffer_t* operator->() { return &(this->buffer_); };
                const core::api::buffer_t* operator->() const { return &(this->buffer_); };

                // export allocation ONCE as dedicated (should to be unbound from that object)
                virtual std::shared_ptr<svt::api::factory::allocation_t> export_allocation() override {
                    return std::dynamic_pointer_cast<svt::api::factory::allocation_t>(std::make_shared<allocation_t>(std::move(allocation_)));
                };

                // 
                virtual ~buffer_t() override {  };
                virtual uintptr_t get_allocation() override;
                virtual uintptr_t get_allocation_info() override;
                virtual void* get_mapped() override;
                virtual void* map() override;
                virtual void unmap(void* ptr = nullptr) override;
            };

        };
    };
};
