#include "./factory/API/unified/device.hpp"
#include "./factory/VMA/unified/buffer.hpp"
#include "./factory/VMA/unified/acceleration_structure.hpp"
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
                // 
                std::vector<vk::GeometryNV> geometries = {};
                for (auto& g : create_info.info.geometries) {
                    vk::GeometryNV geometry = {};
                    geometry.flags = vk::GeometryFlagsNV(g.flags_32u);
                    geometry.geometryType = vk::GeometryTypeNV(g.type);
                    geometry.geometry.triangles.indexCount = g.triangles.indices.size;
                    geometry.geometry.triangles.indexData = g.triangles.indices.data;
                    geometry.geometry.triangles.indexOffset = g.triangles.indices.offset;
                    geometry.geometry.triangles.indexType = vk::IndexType(g.triangles.index_type);
                    geometry.geometry.triangles.vertexCount = g.triangles.vertices.size;
                    geometry.geometry.triangles.vertexData = g.triangles.vertices.data;
                    geometry.geometry.triangles.vertexOffset = g.triangles.vertices.offset;
                    geometry.geometry.triangles.vertexFormat = vk::Format(g.triangles.vertex_format);
                    geometry.geometry.triangles.vertexStride = g.triangles.vertices.stride;
                    geometry.geometry.triangles.transformData = g.triangles.transform.data;
                    geometry.geometry.triangles.transformOffset = g.triangles.transform.offset;
                    geometry.geometry.aabbs.aabbData = g.aabbs.data;
                    geometry.geometry.aabbs.numAABBs = g.aabbs.size;
                    geometry.geometry.aabbs.offset = g.aabbs.offset;
                    geometry.geometry.aabbs.stride = g.aabbs.stride;
                    geometries.push_back(geometry);
                };

                // 
                vk::AccelerationStructureCreateInfoNV vk_info = {};
                vk::AccelerationStructureInfoNV& acc_info = vk_info.info;
                acc_info.type = vk::AccelerationStructureTypeNV(create_info.info.type);
                acc_info.flags = vk::BuildAccelerationStructureFlagsNV(create_info.info.flags_32u);
                acc_info.pGeometries = geometries.data();
                acc_info.geometryCount = geometries.size();
                acc_info.instanceCount = create_info.info.instances.size;

                // 
                vk_info.compactedSize = create_info.compacted_size;
                auto acceleration = std::make_shared<acceleration_structure_t>((*device)->createAccelerationStructureNV(vk_info,nullptr,*device));

                // Allocate Memory 
                vk::AccelerationStructureMemoryRequirementsInfoNV mem_info = {};
                mem_info.accelerationStructure = *acceleration;
                mem_info.type = vk::AccelerationStructureMemoryRequirementsTypeNV::eObject;
                vk::MemoryRequirements2 requirements = (*device)->getAccelerationStructureMemoryRequirementsNV(mem_info,*device);
                vmaAllocateMemory(allocator_, (VkMemoryRequirements*)&requirements.memoryRequirements, (VmaAllocationCreateInfo*)info_ptr, &acceleration->allocation_->allocation_, &acceleration->allocation_->allocation_info_);

                // Bind Memory 
                vk::BindAccelerationStructureMemoryInfoNV bind_info{};
                bind_info.accelerationStructure = *acceleration;
                bind_info.memory = acceleration->allocation_->allocation_info_.deviceMemory;
                bind_info.memoryOffset = acceleration->allocation_->allocation_info_.offset; // Sometimes can be required...
                (*device)->bindAccelerationStructureMemoryNV(bind_info);

                // 
                return { std::dynamic_pointer_cast<api::factory::acceleration_structure_t>(acceleration), core::api::result_t(0u) };
            };
#endif
        };
    };
};
