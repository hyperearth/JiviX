#pragma once

#include "./classes/API/types.hpp"
#include "./dynamic/API/unified/buffer.hpp"

namespace svt {
    namespace api {

        class buffer {
            protected: 
                std::shared_ptr<data::dynamic::buffer_t> buffer_t = {};
                std::shared_ptr<data::device_t> device_t = {};

            public: 
                // structs by C++20

                // TODO: extendent bits support
                struct flags_t {
                    uint32_t 
                        bit_sparse_binding : 1,
                        bit_residency_binding : 1,
                        bit_sparse_alised : 1,
                        bit_protected : 1,
                        bit_device_address_capture : 1;
                };
                
                struct usage_t {
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
                enum class sharing_mode_t : uint32_t {
                    e_exclusive = 0u,
                    e_concurrent = 1u
                };

                // 
                struct create_info {
                    union {
                        uint32_t flags32u = 0u;
                        flags_t flags;
                    };
                    union {
                        uint32_t usage32u = 0u;
                        usage_t usage;
                    };
                    sharing_mode_t sharing_mode = sharing_mode_t::e_exclusive;
                    size_t size = 4u;
                };

                // 
                buffer(const buffer& buffer) : buffer_t(buffer.buffer_t), device_t(buffer.device_t) {};
                buffer(const std::shared_ptr<data::dynamic::buffer_t>& buffer_t = {}) : buffer_t(buffer_t) {};
                buffer(const std::shared_ptr<data::device_t>& device_t, const std::shared_ptr<data::dynamic::buffer_t>& buffer_t = {}) : buffer_t(buffer_t), device_t(device_t) {};

                // 
                operator std::shared_ptr<data::dynamic::buffer_t>&() { return buffer_t; };
                operator std::shared_ptr<data::device_t>&() { return device_t; };
                operator const std::shared_ptr<data::dynamic::buffer_t>&() const { return buffer_t; };
                operator const std::shared_ptr<data::device_t>&() const { return device_t; };

                // 
                svt::core::handle_ref<buffer,core::result_t> create(const create_info& info = {}, const allocator& allocator = {});

                // UN-safe (Debug) API, always should begin from `_`
                svt::api::core::buffer_t _get_buffer_t();

        };
    };

};
