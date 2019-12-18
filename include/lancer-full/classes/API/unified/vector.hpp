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
                inline vector_t(                            const stu::buffer& buffer_ = {}, const stu::vector& vector_ = {}) : vector_(vector_), buffer_(buffer_) {};
                inline vector_t(const stu::device& device_, const stu::buffer& buffer_ = {}, const stu::vector& vector_ = {}) : vector_(vector_), buffer_(buffer_), device_(device_) {};

                // 
                inline operator const stu::vector&() const { return vector_; };
                inline operator const stu::device&() const { return device_; };
                inline operator const stu::buffer&() const { return buffer_; };
                inline operator stu::vector&() { return vector_; };
                inline operator stu::device&() { return device_; };
                inline operator stu::buffer&() { return buffer_; };

                // template type caster
                template<class A = T> inline operator vector<A>& () { return std::dynamic_cast<vector<A>&>(*this); };
                template<class A = T> inline operator const vector<A>& () const { return std::dynamic_cast<const vector<A>&>(*this); };

                // use dynamic polymorphism
                inline T* map() const { return (T*)(this->vector_->map()); };
                inline T* data() const { return (T*)(this->vector_->mapped()); };
                inline const size_t size() const { return this->vector_->range() / sizeof(T); };
                inline const uintptr_t& offset() const { return this->vector_->offset(); };
                inline const uintptr_t& handle() const { return this->vector_->handle(); };

                // TODO: move into `.inl` file
                template<class A = T>
                inline vector_t& operator=(const vector<A>& vector) {
                    this->vector_ = vector;
                    this->buffer_ = vector;
                    this->device_ = vector;
                    return *this;
                };

                // TODO: move into `.inl` file
                inline std::pair<vector_t&, description_handle&>& write_into_description(description_handle& handle, const uint32_t& idx = 0u){
                    handle.offset<core::api::buffer_region_t>(idx) = core::api::buffer_region_t{ *vector_, vector_->offset, vector_->range };
                    return {*this, handle};
                };

                // TODO: move into `.inl` file
                inline api::factory::vector_t* operator->() { return &(*this->vector_); };
                inline const api::factory::vector_t* operator->() const { return &(*this->vector_); };

            protected: friend vector_t; 
                stu::vector vector_ = {};
                stu::buffer buffer_ = {};
                stu::device device_ = {};
            };
        };
    };
};
