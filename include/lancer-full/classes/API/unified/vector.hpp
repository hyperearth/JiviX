#pragma once

#include "./classes/API/types.hpp"
#include "./factory/API/unified/allocator.hpp"
#include "./factory/API/unified/buffer.hpp"
#include "./factory/API/unified/vector.hpp"
#include "./classes/API/unified/allocator.hpp"
#include "./classes/API/unified/buffer.hpp"
//#include "./classes/API/unified/buffer_view.hpp"
#include "./classes/API/unified/descriptor_set.hpp"


namespace svt {
    namespace api {
        namespace classes {
            
            template<class T = uint8_t>
            class vector { public: using T = T; using vector_t = vector<T>;
                template<class A = T>
                inline vector_t(const vector<A>& vector) : vector_(vector), buffer_(vector), device_(vector) {};
                inline vector_t(                              const stu::buffer& buffer_ = {}, const stu::vector& vector_ = {}) : vector_(vector_), buffer_(buffer_) {};
                inline vector_t(const stu::device_t& device_, const stu::buffer& buffer_ = {}, const stu::vector& vector_ = {}) : vector_(vector_), buffer_(buffer_), device_(device_) {};

                // 
                inline operator const stu::device_t&() const { return device_; };
                inline operator const stu::vector&() const { return vector_; };
                inline operator const stu::device&() const { return device_; };
                inline operator const stu::buffer&() const { return buffer_; };
                inline operator stu::device_t&() { return device_; };
                inline operator stu::vector&() { return vector_; };
                inline operator stu::device&() { return device_; };
                inline operator stu::buffer&() { return buffer_; };

                // template type caster
                template<class A = T> inline operator vector<A>& () { return std::dynamic_cast<vector<A>&>(*this); };
                template<class A = T> inline operator const vector<A>& () const { return std::dynamic_cast<const vector<A>&>(*this); };

                // TODO: move into `.inl` file
                svt::core::handle_ref<vector<T>,core::api::result_t> create(uintptr_t offset = 0u, uintptr_t size = 1u) {
                    vector_ = std::make_shared<api::factory::vector_t>(*buffer_),
                    vector_->offset() = offset,
                    vector_->stride() = sizeof(T),
                    vector_->size() = size;
                    return {*this,core::api::result_t(0u)};
                };

                // use dynamic polymorphism
                inline T* map() const { return (T*)(this->buffer_->map()); };
                inline T* data() const { return (T*)(this->buffer_->mapped()); };
                inline const size_t& size() const { return this->vector_->size(); };
                inline const size_t& stride() const { return this->vector_->stride(); };
                inline const uintptr_t& offset() const { return this->vector_->offset(); };
                inline const uintptr_t& handle() const { return this->vector_->handle(); };

                // TODO: move into `.inl` file
                template<class A = T>
                inline vector_t& operator=(const vector<A>& vector) { 
                    this->vector_ = vector, 
                    this->buffer_ = vector, 
                    this->device_ = vector; 
                    return *this; 
                };

                // TODO: move into `.inl` file
                inline std::pair<vector<T>&, description_handle&> write_into_description(description_handle& handle, const uint32_t& idx = 0u){
                    handle.offset<core::api::buffer_region_t>(idx) = (*vector_); return {*this, handle};
                };

                // TODO: move into `.inl` file
                inline api::factory::vector_t* operator->() { return &(*this->vector_); };
                inline const api::factory::vector_t* operator->() const { return &(*this->vector_); };
                inline api::factory::vector_t& operator*() { return (*this->vector_); };
                inline const api::factory::vector_t& operator*() const { return (*this->vector_); };

            // TODO: low-level casting operator
            protected: friend vector_t; 
                stu::vector vector_ = {};
                stu::buffer buffer_ = {};
                stu::device_t device_ = {};
            };
        };
    };
};
