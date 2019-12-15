#pragma once

#include "./classes/API/types.hpp"
#include "./factory/API/unified/allocator.hpp"
#include "./factory/API/unified/buffer.hpp"
#include "./factory/API/unified/vector.hpp"
#include "./classes/API/unified/allocator.hpp"
#include "./classes/API/unified/buffer.hpp"


namespace svt {
    namespace api {
        namespace classes {

            //template<class T = uint8_t>
            class vector {
                protected: using T = uint8_t;
                    // 
                    stu::vector vector_t = {};
                    stu::buffer buffer_t = {};
                    stu::device device_t = {};


                public: 
                    vector(const vector& vector_t) : vector_t(vector_t), buffer_t(vector_t), device_t(vector_t) {};
                    vector(                             const stu::buffer& buffer_t = {}, const stu::vector& vector_t = {}) : vector_t(vector_t), buffer_t(buffer_t) {};
                    vector(const stu::device& device_t, const stu::buffer& buffer_t = {}, const stu::vector& vector_t = {}) : vector_t(vector_t), buffer_t(buffer_t), device_t(device_t) {};

                    // 
                    operator const stu::vector&() const { return vector_t; };
                    operator const stu::device&() const { return device_t; };
                    operator const stu::buffer&() const { return buffer_t; };
                    operator stu::vector&() { return vector_t; };
                    operator stu::device&() { return device_t; };
                    operator stu::buffer&() { return buffer_t; };

                    // 
                    const size_t& size() const { return this->vector_t->range; };
                    const uintptr_t& offset() const { return this->vector_t->offset; };
                    const uintptr_t& handle() const { return this->vector_t->handle; };

                    // assign mode 
                    // TODO: move into `.cpp` file
                    vector& operator=(const vector &vector) { 
                        this->vector_t = vector;
                        this->buffer_t = vector;
                        this->device_t = vector;
                        return *this;
                    };

                    // TODO: move into `.cpp` file
                    api::factory::vector_t* operator->() { return &(*this->vector_t); };
                    const api::factory::vector_t* operator->() const { return &(*this->vector_t); };
            };

        };
    };
};
