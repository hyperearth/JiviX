#pragma once

#include "./classes/API/types.hpp"
#include "./factory/API/unified/allocator.hpp"
#include "./factory/API/unified/buffer.hpp"
#include "./factory/API/unified/vector.hpp"
#include "./factory/API/unified/allocator.hpp"
#include "./classes/API/unified/allocator.hpp"

namespace svt {
    namespace api {
        namespace classes {
            class buffer {
                protected: 
                    stu::buffer buffer_t = {};
                    stu::device device_t = {};

                public: 
                    // structs by C++20

                    // TODO: extendent bits support
                    struct flags { uint32_t 
                        bit_sparse_binding : 1,
                        bit_residency_binding : 1,
                        bit_sparse_alised : 1,
                        bit_protected : 1,
                        bit_device_address_capture : 1;
                    };

                    struct usage { uint32_t 
                        bit_transfer_src : 1,
                        bit_transfer_dst : 1,
                        bit_uniform_texel_buffer : 1,
                        bit_storage_texel_buffer : 1,
                        bit_uniform_buffer : 1,
                        bit_storage_buffer : 1,
                        bit_index_buffer : 1,
                        bit_vertex_buffer : 1,
                        bit_indirect_buffer : 1,
                        bit_conditional_rendering : 1,
                        bit_ray_tracing : 1,
                        bit_transform_feedback_buffer : 1,
                        bit_transform_feedback_counter_buffer : 1,
                        bit_shader_device_address : 1;
                    };

                    // 
                    struct create_info {
                        union {
                            uint32_t flags32u = 0u;
                            flags flags;
                        };

                        size_t size = 4u;

                        union {
                            uint32_t usage32u = 0u;
                            usage usage;
                        };

                        sharing_mode sharing_mode = sharing_mode::t_exclusive;
                    };

                    // 
                    buffer(const stu::buffer& buffer_t = {}) : buffer_t(buffer_t) {};
                    buffer(const stu::device& device_t, const stu::buffer& buffer_t = {}) : buffer_t(buffer_t), device_t(device_t) {};
                    buffer(const buffer& buffer) : buffer_t(buffer), device_t(buffer) {};

                    // TODO: merge into `.cpp`
                    operator stu::buffer&() { return buffer_t; };
                    operator stu::device&() { return device_t; };
                    operator const stu::buffer&() const { return buffer_t; };
                    operator const stu::device&() const { return device_t; };

                    // 
                    stu::vector vector(uintptr_t offset = 0u, size_t size = 4u);
                    svt::core::handle_ref<buffer,core::api::result_t> create(const allocator& allocator = {}, const create_info& info = {});

                    // UN-safe (Debug) API, always should begin from `_`
                    svt::core::api::buffer_t _get_buffer_t();
                    
                    // assign mode 
                    // TODO: move into `.cpp` file
                    buffer& operator=(const buffer &buffer) { 
                        this->buffer_t = buffer;
                        this->device_t = buffer;
                        return *this;
                    };
                    
                    // TODO: move into `.cpp` file
                    api::factory::buffer_t* operator->() { return &(*this->buffer_t); };
                    const api::factory::buffer_t* operator->() const { return &(*this->buffer_t); };
            };
        };
    };
};
