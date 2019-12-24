#include "./factory/API/unified/device.hpp"
#include "./factory/VMA/unified/buffer.hpp"
#include "./factory/VMA/unified/allocator.hpp"


namespace svt {
    namespace vma {
        namespace factory {
#ifdef USE_VULKAN
            svt::core::handle_ref<std::shared_ptr<api::factory::acceleration_structure_t>,core::api::result_t> vma::factory::allocator_t::create_acceleration_structure(
                const std::shared_ptr<api::factory::device_t>& device, 
                const std::vector<uint32_t>& queue_family_indices, 
                const api::acceleration_structure_create_info& create_info, 
                const uintptr_t& info_ptr
            ) {
                
            };
#endif
        };
    };
};
