// 
#include "./core/unified/core.hpp"
#include "./classes/API/unified/device.hpp"
#include "./factory/API/unified/allocator.hpp"
#include "./classes/API/unified/allocator.hpp"

// 
#include "./factory/API/unified/acceleration_structure.hpp"
#include "./classes/API/unified/acceleration_structure.hpp"

namespace svt {
    namespace api {
        namespace classes {
#ifdef USE_VULKAN
            // TODO: reorder arguments
            svt::core::handle_ref<acceleration_structure,core::api::result_t> acceleration_structure::create(const stu::allocator& allocator_, const acceleration_structure_create_info& info, const uintptr_t& info_ptr) {
                this->structure_ = (stu::acceleration_structure&)(allocator_->create_acceleration_structure(device_, device_, info, info_ptr));
                this->allocator_ = allocator_;

                // Allocate Memory 
                vk::AccelerationStructureMemoryRequirementsInfoNV mem_info = {};
                mem_info.accelerationStructure = *structure_;
                mem_info.type = vk::AccelerationStructureMemoryRequirementsTypeNV::eBuildScratch;
                vk::MemoryRequirements2 requirements = (*device_)->getAccelerationStructureMemoryRequirementsNV(mem_info,*device_);

                // Create scratch buffer (i.e. Part II)
                if (requirements.memoryRequirements.size > 0u) {
                    scratch_ = allocator_->create_buffer(device_,*device_,buffer_create_info{ .size = requirements.memoryRequirements.size, .usage = { .b_ray_tracing = 1 } }, info_ptr);
                };
                //if (info.info.instances.size > 0u) {
                //    instances_ = allocator_->create_buffer(device_,*device_,buffer_create_info{.size = sizeof(api::instance_data) * info.info.instances.size },info_ptr);
                //};

                // 
                return { *this, core::api::result_t(0u) };
            };
#endif
        };
    };
};
