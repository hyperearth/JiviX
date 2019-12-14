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
                    using buffer_st = std::shared_ptr<api::factory::buffer_t>;
                    using device_st = std::shared_ptr<api::factory::device_t>;
                    using vector_st = std::shared_ptr<api::factory::vector_t>;

                    buffer_st buffer_t = {};
                    device_st device_t = {};

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
                        union {
                            uint32_t usage32u = 0u;
                            usage usage;
                        };
                        sharing_mode sharing_mode = sharing_mode::t_exclusive;
                        size_t size = 4u;
                    };

                    // 
                    buffer(const buffer_st& buffer_t = {}) : buffer_t(buffer_t) {};
                    buffer(const device_st& device_t, const buffer_st& buffer_t = {}) : buffer_t(buffer_t), device_t(device_t) {};
                    buffer(const buffer& buffer) : buffer_t(buffer), device_t(buffer) {};

                    // TODO: merge into `.cpp`
                    operator buffer_st&() { return buffer_t; };
                    operator device_st&() { return device_t; };
                    operator const buffer_st&() const { return buffer_t; };
                    operator const device_st&() const { return device_t; };

                    // 
                    vector_st vector(uintptr_t offset = 0u, size_t size = 4u);
                    svt::core::handle_ref<buffer,core::api::result_t> create(const create_info& info = {}, const allocator& allocator = {});

                    // UN-safe (Debug) API, always should begin from `_`
                    svt::core::api::buffer_t _get_buffer_t();
                    
            };
        };
    };
};
