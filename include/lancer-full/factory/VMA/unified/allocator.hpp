#pragma once

#include "./core/unified/core.hpp"
#include "./factory/API/types.hpp"
#include "./factory/VMA/types.hpp"
#include "./factory/API/unified/allocator.hpp"
#include "./factory/VMA/unified/enums.hpp"

namespace svt {
    namespace vma {
        namespace factory {
            class allocator_t : public api::factory::allocator_t { public: 
                virtual ~allocator_t() override {};
                virtual uintptr_t get_allocator() const override { return 0u; };
                virtual uintptr_t get_allocator_info() const override { return 0u; };
                VmaAllocator allocator_ = {};

                virtual svt::core::handle_ref<std::shared_ptr<api::factory::buffer_t>,core::api::result_t> create_buffer(const std::shared_ptr<api::factory::device_t>& device = {}, const std::vector<uint32_t>& queue_family_indices = {}, const buffer_create_info& create_info = {}, const uintptr_t& info_ptr = 0u) override;
                virtual svt::core::handle_ref<std::shared_ptr<api::factory::image_t >,core::api::result_t> create_image (const std::shared_ptr<api::factory::device_t>& device = {}, const std::vector<uint32_t>& queue_family_indices = {}, const  image_create_info& create_info = {}, const uintptr_t& info_ptr = 0u, const image_layout& initial_layout = image_layout::t_undefined) override;
                virtual svt::core::handle_ref<std::shared_ptr<api::factory::acceleration_structure_t>,core::api::result_t> create_acceleration_structure(const std::shared_ptr<api::factory::device_t>& device = {}, const std::vector<uint32_t>& queue_family_indices = {}, const acceleration_structure_create_info& create_info = {}, const uintptr_t& info_ptr = 0u) override;
            };
        };
    };
};
