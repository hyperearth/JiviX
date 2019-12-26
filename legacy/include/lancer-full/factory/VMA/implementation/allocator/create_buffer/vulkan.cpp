#include "./factory/API/unified/device.hpp"
#include "./factory/VMA/unified/buffer.hpp"
#include "./factory/VMA/unified/allocator.hpp"


namespace svt {
    namespace vma {
        namespace factory {
#ifdef USE_VULKAN
            svt::core::handle_ref<std::shared_ptr<api::factory::buffer_t>,core::api::result_t> vma::factory::allocator_t::create_buffer(
                const std::shared_ptr<api::factory::device_t>& device, 
                const std::vector<uint32_t>& queue_family_indices, 
                const api::buffer_create_info& create_info, 
                const uintptr_t& info_ptr,
                const buffer_modifier& modifier
            ) { // 
                vk::BufferCreateInfo vk_info{};
                vk_info.size = create_info.size;
                vk_info.usage = vk::BufferUsageFlagBits(create_info.usage_32u);
                vk_info.flags = vk::BufferCreateFlagBits(create_info.flags_32u);
                vk_info.sharingMode = vk::SharingMode(create_info.sharing_mode);
                vk_info.queueFamilyIndexCount = queue_family_indices.size();
                vk_info.pQueueFamilyIndices = queue_family_indices.data();

                // From-Host-related
                auto* vma_info = (VmaAllocationCreateInfo*)info_ptr;
                if (modifier == buffer_modifier::t_cpu_to_gpu) {
                    vma_info->usage = VMA_MEMORY_USAGE_CPU_TO_GPU;
                    vma_info->flags |= VMA_ALLOCATION_CREATE_MAPPED_BIT;
                    vk_info.usage.b_transfer_src = true;
                };

                // To-Host-related
                if (modifier == buffer_modifier::t_gpu_to_cpu) {
                    vma_info->usage = VMA_MEMORY_USAGE_GPU_TO_CPU;
                    vma_info->flags |= VMA_ALLOCATION_CREATE_MAPPED_BIT;
                    vk_info.usage.b_transfer_dst = true;
                };

                // GPU-related
                if (modifier == buffer_modifier::t_gpu_only) {
                    vma_info->usage = VMA_MEMORY_USAGE_GPU_ONLY;
                    vk_info.usage.b_transfer_src = true;
                    vk_info.usage.b_transfer_dst = true;
                };

                // 
                auto buffer = std::make_shared<buffer_t>(); buffer->allocation_ = std::make_unique<internal_allocation_t>(); // Create Image With Internal Allocation
                vmaCreateBuffer(allocator_,(VkBufferCreateInfo*)(&vk_info),vma_info,(VkBuffer*)(&buffer->buffer_),&buffer->allocation_->allocation_,&buffer->allocation_->allocation_info_);
                return { std::dynamic_pointer_cast<api::factory::buffer_t>(buffer), core::api::result_t(0u) };
            };
#endif
        };
    };
};
