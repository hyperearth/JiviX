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
            class vector { public: using vector_t = vector<t>;
                template<class A = T>
                inline vector_t(const vector<A>& vector_t) : vector_t(vector_t), buffer_t(vector_t), device_t(vector_t) {};
                inline vector_t(                             const stu::buffer& buffer_t = {}, const stu::vector& vector_t = {}) : vector_t(vector_t), buffer_t(buffer_t) {};
                inline vector_t(const stu::device& device_t, const stu::buffer& buffer_t = {}, const stu::vector& vector_t = {}) : vector_t(vector_t), buffer_t(buffer_t), device_t(device_t) {};

                // 
                inline operator const stu::vector&() const { return vector_t; };
                inline operator const stu::device&() const { return device_t; };
                inline operator const stu::buffer&() const { return buffer_t; };
                inline operator stu::vector&() { return vector_t; };
                inline operator stu::device&() { return device_t; };
                inline operator stu::buffer&() { return buffer_t; };

                // template type caster
                template<class A = T> inline operator vector<A>& () { return std::dynamic_cast<vector<A>&>(*this); };
                template<class A = T> inline operator const vector<A>& () const { return std::dynamic_cast<const vector<A>&>(*this); };

                // use dynamic polymorphism
                inline T* map() const { return (T*)(this->vector_t->map()); };
                inline T* data() const { return (T*)(this->vector_t->mapped()); };
                inline const size_t size() const { return this->vector_t->range() / sizeof(T); };
                inline const uintptr_t& offset() const { return this->vector_t->offset(); };
                inline const uintptr_t& handle() const { return this->vector_t->handle(); };

                // TODO: move into `.inl` file
                template<class A = T>
                inline vector_t& operator=(const vector<A>& vector_t) {
                    this->vector_t = vector_t;
                    this->buffer_t = vector_t;
                    this->device_t = vector_t;
                    return *this;
                };

                // TODO: move into `.inl` file
                inline std::pair<vector_t&, description_handle&>& write_into_description(description_handle& handle, const uint32_t& idx = 0u){
                    handle.offset<core::api::buffer_region_t>(idx) = core::api::buffer_region_t{ *vector_t, vector_t->offset, vector_t->range };
                    return {*this, handle};
                };

                // TODO: move into `.inl` file
                inline api::factory::vector_t* operator->() { return &(*this->vector_t); };
                inline const api::factory::vector_t* operator->() const { return &(*this->vector_t); };

            protected: friend vector_t; 
                stu::vector vector_t = {};
                stu::buffer buffer_t = {};
                stu::device device_t = {};
            };
        };
    };
};
