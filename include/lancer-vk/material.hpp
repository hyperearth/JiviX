#pragma once
#include "./config.hpp"
#include "./driver.hpp"

namespace lancer {

    // 
    struct MaterialUnit {

    };

    // WIP Materials
    class Material : public std::enable_shared_from_this<Material> { public: 
        Material() {};

        // 
        std::shared_ptr<Material> setRawMaterials(const vkt::Vector<MaterialUnit>& rawMaterials = {}, const uint32_t& materialCounter = 0u) {
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

    protected: // 
        std::vector<vkh::VkDescriptorImageInfo> sampledImages = {};

        // 
        vkt::Vector<MaterialUnit> rawMaterials = {}; // Ray-Tracing instances Will re-located into meshes by Index, and will no depending by mesh list...
        vkt::Vector<MaterialUnit> gpuMaterials = {};
        uint32_t materialCounter = 0u;

        // 
        vkh::VsDescriptorSetCreateInfoHelper descriptorSetInfo = {};
        vkh::VkAccelerationStructureInfoNV accelerationStructureInfo = {};

        // 
        vk::DescriptorSet descriptorSet = {};

        // 
        std::shared_ptr<Driver> driver = {};
        std::shared_ptr<Thread> thread = {};
    };

};
