#pragma once

#include "./classes/API/types.hpp"
#include "./factory/API/unified/allocator.hpp"
#include "./factory/API/unified/buffer.hpp"
#include "./factory/API/unified/vector.hpp"
#include "./factory/API/classes/allocator.hpp"

namespace svt {
    namespace api {
        namespace classes {
            class buffer {
                protected: 
                    std::shared_ptr<api::factory::buffer_t> buffer_t = {};
                    std::shared_ptr<api::factory::device_t> device_t = {};
                    
                public: 
                    // structs by C++20

                    // TODO: extendent bits support
                    struct flags {
                        uint32_t 
                            bit_sparse_binding : 1,
                            bit_residency_binding : 1,
                            bit_sparse_alised : 1,
                            bit_protected : 1,
                            bit_device_address_capture : 1;
                    };
                    
                    struct usage {
                        uint32_t 
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
                    buffer(const buffer& buffer) : buffer_t(buffer.buffer_t), device_t(buffer.device_t) {};
                    buffer(const std::shared_ptr<api::factory::buffer_t>& buffer_t = {}) : buffer_t(buffer_t) {};
                    buffer(const std::shared_ptr<api::factory::device_t>& device_t, const std::shared_ptr<api::factory::buffer_t>& buffer_t = {}) : buffer_t(buffer_t), device_t(device_t) {};

                    // 
                    operator std::shared_ptr<api::factory::buffer_t>&() { return buffer_t; };
                    operator std::shared_ptr<api::factory::device_t>&() { return device_t; };
                    operator const std::shared_ptr<api::factory::buffer_t>&() const { return buffer_t; };
                    operator const std::shared_ptr<api::factory::device_t>&() const { return device_t; };

                    // 
                    std::shared_ptr<svt::api::factory::vector_t> vector(uintptr_t offset = 0u, size_t size = 4u);
                    svt::core::handle_ref<buffer,core::api::result_t> create(const create_info& info = {}, const svt::api::allocator& allocator = {});

                    // UN-safe (Debug) API, always should begin from `_`
                    svt::core::api::buffer_t _get_buffer_t();
                    
            };
        };
    };
};
