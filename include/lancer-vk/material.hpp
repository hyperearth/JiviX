#pragma once
#include "./config.hpp"
#include "./driver.hpp"
#include "./thread.hpp"
#include "./context.hpp"

namespace lancer {

    // 
    struct MaterialUnit {

    };

    // WIP Materials
    class Material : public std::enable_shared_from_this<Material> { public: 
        Material(const std::shared_ptr<Context>& context, const std::shared_ptr<Driver>& driver) {
            this->driver = driver;
            this->thread = std::make_shared<Thread>(this->driver);
            this->context = context;
            
            // 
            this->rawMaterials = vkt::Vector<vkh::VsGeometryInstance>(std::make_shared<vkt::VmaBufferAllocation>(this->driver->getAllocator(), vkh::VkBufferCreateInfo{ .size = sizeof(MaterialUnit)*64u, .usage = { .eUniformBuffer = 1, .eRayTracing = 1 } }, VMA_MEMORY_USAGE_CPU_TO_GPU));
            this->gpuMaterials = vkt::Vector<vkh::VsGeometryInstance>(std::make_shared<vkt::VmaBufferAllocation>(this->driver->getAllocator(), vkh::VkBufferCreateInfo{ .size = sizeof(MaterialUnit)*64u, .usage = { .eUniformBuffer = 1, .eRayTracing = 1 } }, VMA_MEMORY_USAGE_GPU_ONLY));
        };

        // 
        std::shared_ptr<Material> setContext(const std::shared_ptr<Context>& context) {
            this->context = context;
            this->descriptorSetInfo = vkh::VsDescriptorSetCreateInfoHelper(this->context->materialDescriptorSetLayout,this->thread->getDescriptorPool());
            return shared_from_this();
        };

        // 
        std::shared_ptr<Material> setRawMaterials(const vkt::Vector<MaterialUnit>& rawMaterials = {}, const vk::DeviceSize& materialCounter = 0u) {
            this->rawMaterials = rawMaterials; this->materialCounter = materialCounter;
            return shared_from_this();
        };

        // 
        std::shared_ptr<Material> setGpuInstance(const vkt::Vector<MaterialUnit>& gpuMaterials = {}) {
            this->gpuMaterials = gpuMaterials;
            return shared_from_this();
        };

        // 
        std::shared_ptr<Material> pushSampledImage(const vkh::VkDescriptorImageInfo& info = {}) {
            this->sampledImages.push_back(info);
            return shared_from_this();
        };

        // 
        std::shared_ptr<Material> copyBuffers(const vk::CommandBuffer& buildCommand = {}) {
            buildCommand.copyBuffer(this->rawMaterials, this->gpuMaterials, { vk::BufferCopy{ this->rawMaterials.offset(), this->gpuMaterials.offset(), this->gpuMaterials.range() } });
            return shared_from_this();
        };

        // 
        std::shared_ptr<Material> createDescriptorSet() {
            {   // Setup Textures
                auto& handle = this->descriptorSetInfo.pushDescription(vkh::VkDescriptorUpdateTemplateEntry{
                    .dstBinding = 0u,
                    .descriptorCount = uint32_t(sampledImages.size()),
                    .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE
                });
                memcpy(&handle.offset<VkDescriptorImageInfo>(), sampledImages.data(), sampledImages.size()*sizeof(VkDescriptorImageInfo));

                // Reprojection WILL NOT write own depth... 
                this->descriptorSet = driver->getDevice().allocateDescriptorSets(this->descriptorSetInfo)[0];
                this->driver->getDevice().updateDescriptorSets(vkt::vector_cast<vk::WriteDescriptorSet,vkh::VkWriteDescriptorSet>(this->descriptorSetInfo.setDescriptorSet(this->descriptorSet)),{});
            };

            // 
            this->context->descriptorSets[4] = this->descriptorSet;
            return shared_from_this();
        };

    protected: // 
        std::vector<vkh::VkDescriptorImageInfo> sampledImages = {};

        // 
        vkt::Vector<MaterialUnit> rawMaterials = {}; // Ray-Tracing instances Will re-located into meshes by Index, and will no depending by mesh list...
        vkt::Vector<MaterialUnit> gpuMaterials = {};

        // 
        vkh::VsDescriptorSetCreateInfoHelper descriptorSetInfo = {};
        vk::DescriptorSet descriptorSet = {};

        // 
        vk::DeviceSize materialCounter = 0u;
        std::shared_ptr<Driver> driver = {};
        std::shared_ptr<Thread> thread = {};
        std::shared_ptr<Context> context = {};
    };
};
