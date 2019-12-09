#pragma once
#include "../types.hpp"

namespace svt {
    namespace api {

        class buffer {
            protected: 
                std::shared_ptr<data::buffer_t> buffer_t = {};
                std::shared_ptr<data::device_t> device_t = {};

            public: 
                // structs by C++20

                // TODO: extendent bits support
                struct flags_t {
                    uint32_t sparse_binding_bit : 1,
                            residency_binding_bit : 1,
                            sparse_alised_bit : 1,
                            protected_bit : 1,
                            device_address_capture_bit : 1;
                };

                struct usage_t {
                    uint32_t transfer_src_bit : 1,
                            transfer_dst_bit : 1,
                            uniform_texel_buffer_bit : 1,
                            storage_texel_buffer_bit : 1,
                            uniform_buffer_bit : 1,
                            storage_buffer_bit : 1,
                            index_buffer_bit : 1,
                            vertex_buffer_bit : 1,
                            indirect_buffer_bit : 1,
                            conditional_rendering_bit : 1,
                            ray_tracing_bit : 1,
                            transform_feedback_buffer_bit : 1,
                            transform_feedback_counter_buffer_bit : 1,
                            shader_device_address : 1;
                };

                // 
                enum class sharing_mode_t : uint32_t {
                    exclusive = 0u,
                    concurrent = 1u
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
                    sharing_mode_t sharing_mode = sharing_mode_t::exclusive;
                    size_t size = 4u;
                };

                // 
                buffer(const buffer& buffer_t) : buffer_t(buffer_t), device_t(buffer_t) {};
                buffer(const std::shared_ptr<data::buffer_t>& buffer_t = {}) : buffer_t(buffer_t) {};
                buffer(const std::shared_ptr<data::device_t>& device_t = {}, const std::shared_ptr<data::buffer_t>& buffer_t = {}) : buffer_t(buffer_t), device_t(device_t) {};

                // 
                operator std::shared_ptr<data::buffer_t>&() { return buffer_t; };
                operator std::shared_ptr<data::device_t>&() { return device_t; };
                operator const std::shared_ptr<data::buffer_t>&() const { return buffer_t; };
                operator const std::shared_ptr<data::device_t>&() const { return device_t; };

                // 
                svt::core::handle_ref<buffer,core::result_t> create(const create_info& info = {});

        };
    };

};
