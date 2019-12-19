#pragma once

#include "./core/unified/core.hpp"
#include "./factory/API/types.hpp"

namespace svt {
    namespace api {
        namespace factory {

            // TODO: VMA version of allocator
            class allocator_t : public std::enable_shared_from_this<allocator_t> { public: 
                virtual ~allocator_t(){};
                virtual uintptr_t get_allocator() const { return 0u; };
                virtual uintptr_t get_allocator_info() const { return 0u; };

                // TODO: reorder arguments
                virtual std::shared_ptr<buffer_t> create_buffer(const std::shared_ptr<device_t>& device, const std::vector<uint32_t>& queue_family_indices = {}, const buffer_create_info& create_info = {}, const uintptr_t& info_ptr = 0u);
                virtual std::shared_ptr<image_t>  create_image (const std::shared_ptr<device_t>& device, const std::vector<uint32_t>& queue_family_indices = {}, const  image_create_info& create_info = {}, const uintptr_t& info_ptr = 0u, const image_layout& initial_layout = {});
                //virtual std::shared_ptr<acceleration_structure_t> create_acceleration_structure(const std::shared_ptr<device_t>& device, const uintptr_t& info_ptr = 0u);
            };
        };
    };
};
