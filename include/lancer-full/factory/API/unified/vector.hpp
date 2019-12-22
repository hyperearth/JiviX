#pragma once

#include "./core/unified/core.hpp"
#include "./factory/API/types.hpp"
#include "./factory/API/unified/buffer.hpp"

namespace svt {
    namespace api {
        namespace factory {
            class vector_t : public std::enable_shared_from_this<vector_t> { protected: friend vector_t; uintptr_t handle = 0u; uintptr_t offset = 0u; size_t range = 4u;
                public: vector_t(const std::shared_ptr<buffer_t>& buffer = {}, const uintptr_t& offset = 0u, const size_t& range = 4u) : offset(offset), range(range), handle(*buffer) {};
                public: vector_t(const vector_t& vector = {}) : offset(vector.offset), range(vector.range), handle(vector.handle) {};

                vector_t& operator=(const std::shared_ptr<buffer_t>& buffer){
                    this->handle = *buffer;
                    return *this;
                };

                vector_t& operator=(const std::shared_ptr<vector_t>& vector){
                    this->handle = vector->handle;
                    this->offset = vector->offset;
                    this->range = vector->range;
                    return *this;
                };

                virtual size_t& range() { return this->range; };
                virtual uintptr_t& offset() { return this->offset; };
                virtual uintptr_t& handle() { return this->handle; };
                virtual const size_t& range() const { return this->range; };
                virtual const uintptr_t& offset() const { return this->offset; };
                virtual const uintptr_t& handle() const { return this->handle; };
                virtual void* map() { return nullptr; };
                virtual void* mapped() { return nullptr; };

                // Un-Safe API Related
                operator const svt::core::api::buffer_region_t& () const;
                operator svt::core::api::buffer_region_t& ();

                //operator uintptr_t&() { return (uintptr_t&)(sampler_); };
                //operator const uintptr_t&() const { return (uintptr_t&)(sampler_); };
            };

        };
    };
};
