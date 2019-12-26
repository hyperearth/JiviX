#pragma once

#include "./core/unified/core.hpp"
#include "./factory/API/types.hpp"
#include "./factory/API/unified/buffer.hpp"

namespace svt {
    namespace api {
        namespace factory {
            class vector_t : public std::enable_shared_from_this<vector_t> { protected: friend vector_t; 
                vector_structure vector_ = {};
                public: vector_t(const std::shared_ptr<buffer_t>& buffer = {}, const uintptr_t& offset = 0u, const size_t size = 1u, const size_t& stride = 4u) : vector_(vector_structure{*buffer, offset, size, stride}) {};
                public: vector_t(const vector_t& vector = {}) : vector_(vector) {};

                // 
                vector_t& operator=(const std::shared_ptr<buffer_t>& buffer_) { this->handle = (*buffer_); return *this; };
                vector_t& operator=(const std::shared_ptr<vector_t>& vector_) { this->vector_ = (*vector_); return *this; };
                vector_t& operator=(const buffer_t& buffer_) { this->handle = buffer_; return *this; };
                vector_t& operator=(const vector_t& vector_) { this->vector_ = vector_; return *this; };

                // 
                virtual operator vector_structure&() { return vector_; };
                virtual operator const vector_structure&() const { return vector_; };
                virtual operator core::api::buffer_t&() { return vector_.data; };
                virtual operator const core::api::buffer_t&() const { return vector_.data; };

                // 
                virtual uintptr_t& offset() { return vector_.offset; };
                virtual uintptr_t& handle() { return (uintptr_t&)(vector_.data); };
                virtual size_t& size() { return vector_.size; };
                virtual size_t& stride() { return vector_.stride; };
                virtual core::api::buffer_t& buffer() { return vector_.data; };

                // 
                virtual const size_t& size() const { return vector_.size; };
                virtual const size_t& stride() const { return vector_.stride; };
                virtual const uintptr_t& offset() const { return vector_.offset; };
                virtual const uintptr_t& handle() const { return (uintptr_t&)(vector_.data); };
                virtual const core::api::buffer_t& buffer() const { return vector_.data; };
                
                // 
                virtual size_t range() const { return vector_.range(); };
                //virtual void* map() { return nullptr; };
                //virtual void* mapped() { return nullptr; };

                // Un-Safe API Related
                operator const svt::core::api::buffer_region_t() const { return svt::core::api::buffer_region_t(this->buffer(),this->offset(),this->range()); };

                //operator uintptr_t&() { return (uintptr_t&)(sampler_); };
                //operator const uintptr_t&() const { return (uintptr_t&)(sampler_); };
            };

        };
    };
};
