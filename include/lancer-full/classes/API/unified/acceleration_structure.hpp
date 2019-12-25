#pragma once

#include "./classes/API/types.hpp"
#include "./factory/API/unified/acceleration_structure.hpp"

namespace svt {
    namespace api {
        namespace classes {
            class acceleration_structure { public: 
                acceleration_structure(                              const stu::acceleration_structure& structure_ = {}) : structure_(structure_) {};
                acceleration_structure(const stu::device_t& device_, const stu::acceleration_structure& structure_ = {}) : structure_(structure_), device_(device_) {};
                acceleration_structure(const acceleration_structure& acceleration_structure) : allocator_(acceleration_structure.allocator_), scratch_(acceleration_structure.scratch_), structure_(structure_), device_(acceleration_structure) {};

                // TODO: move into `.cpp` file
                operator stu::acceleration_structure&() { return structure_; };
                operator stu::device&() { return device_; };
                operator stu::device_t&() { return device_; };
                operator stu::allocator&() { return allocator_; };
                operator const stu::acceleration_structure&() const { return structure_; };
                operator const stu::device&() const { return device_; };
                operator const stu::device_t&() const { return device_; };
                operator const stu::allocator&() const { return allocator_; };

                // Currently Aggregator
                svt::core::handle_ref<acceleration_structure,core::api::result_t> create(const stu::allocator& allocator_ = {}, const acceleration_structure_create_info& info = {}, const uintptr_t& info_ptr = 0u);

                // UN-safe (Debug) API, always should begin from `_`
                svt::core::api::acceleration_structure_t _get_acceleration_structure_t();

                // TODO: move into `.cpp` file
                acceleration_structure& operator=(const acceleration_structure &acceleration_structure) { 
                    this->structure_ = acceleration_structure;
                    this->scratch_ = acceleration_structure.scratch_;
                    this->device_ = acceleration_structure;
                    this->allocator_ = acceleration_structure.allocator_;
                    return *this;
                };

                // TODO: move into `.cpp` file
                inline std::pair<acceleration_structure&, description_handle&> write_into_description(description_handle& handle, const uint32_t& idx = 0u){
                    handle.offset<core::api::acceleration_structure_t>(idx) = (*structure_); return {*this, handle};
                };

                // TODO: move into `.cpp` file
                api::factory::acceleration_structure_t* operator->() { return &(*this->structure_); };
                const api::factory::acceleration_structure_t* operator->() const { return &(*this->structure_); };
                api::factory::acceleration_structure_t& operator*() { return (*this->structure_); };
                const api::factory::acceleration_structure_t& operator*() const { return (*this->structure_); };

                // 
                operator uintptr_t&() { return (uintptr_t&)(structure_->structure_); };
                operator const uintptr_t&() const { return (uintptr_t&)(structure_->structure_); };

                operator core::api::acceleration_structure_t&() { return (*structure_); };
                operator const core::api::acceleration_structure_t&() const { return (*structure_); };

                operator core::api::device_t&() { return device_; };
                operator const core::api::device_t&() const { return device_; };

                operator core::api::physical_device_t&() { return device_; };
                operator const core::api::physical_device_t&() const { return device_; };

                stu::buffer& get_scratch_buffer() { return scratch_; };
                //stu::buffer& get_instances_buffer() { return instances_; };

            // 
            protected: friend acceleration_structure; friend allocator;
                //stu::buffer instances_ = {};
                //stu::vector scratch_ = {};
                //stu::vector instances_ = {};
                stu::acceleration_structure structure_ = {};
                stu::buffer scratch_ = {};
                //stu::allocation allocation_ = {};
                stu::allocator allocator_ = {};
                stu::device_t device_ = {};
            };
        };
    };
};
