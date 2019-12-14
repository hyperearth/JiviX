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
                    using vector_st = std::shared_ptr<api::factory::vector_t>;
                    using buffer_st = std::shared_ptr<api::factory::buffer_t>;
                    using device_st = std::shared_ptr<api::factory::device_t>;

                    // 
                    vector_st vector_t = {};
                    buffer_st buffer_t = {};
                    device_st device_t = {};


                public: 
                    vector(const vector& vector_t) : vector_t(vector_t), buffer_t(vector_t), device_t(vector_t) {};
                    vector(                           const buffer_st& buffer_t = {}, const vector_st& vector_t = {}) : vector_t(vector_t), buffer_t(buffer_t) {};
                    vector(const device_st& device_t, const buffer_st& buffer_t = {}, const vector_st& vector_t = {}) : vector_t(vector_t), buffer_t(buffer_t), device_t(device_t) {};

                    // 
                    operator const vector_st&() const { return vector_t; };
                    operator const device_st&() const { return device_t; };
                    operator const buffer_st&() const { return buffer_t; };
                    operator vector_st&() { return vector_t; };
                    operator device_st&() { return device_t; };
                    operator buffer_st&() { return buffer_t; };

                    // 
                    const size_t& size() const { return this->vector_t->range; };
                    const uintptr_t& offset() const { return this->vector_t->offset; };
                    const uintptr_t& handle() const { return this->vector_t->handle; };
            };

        };
    };
};
