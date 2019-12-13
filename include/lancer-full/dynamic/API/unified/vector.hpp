#pragma once

#include "./core/unified/core.hpp"
#include "./dynamic/API/types.hpp"
#include "./dynamic/API/unified/buffer.hpp"

namespace svt {
    namespace api {
        namespace dynamic {
            class vector_t : public std::enable_shared_from_this<vector_t> { public: 
                vector_t(const std::shared_ptr<buffer_t>& buffer = {}, const uintptr_t& offset = 0u, const size_t& range = 4u) : offset(offset), range(range), handle(*buffer) {};
                //core::buffer_t buffer;
                uintptr_t handle = 0u; uintptr_t offset = 0u; size_t range = 4u;

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

                // Un-Safe API Related
                operator const svt::core::api::buffer_region_t&() const;
                operator svt::core::api::buffer_region_t&();
            };

        };
    };
};
