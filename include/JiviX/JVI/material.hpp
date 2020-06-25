#pragma once // #

#include "./config.hpp"
#include "./driver.hpp"
#include "./thread.hpp"
#include "./context.hpp"

namespace jvi {

    // 
#pragma pack(push, 1)
    struct MaterialUnit {
        glm::vec4 diffuse = { 1.f,1.f,1.f,1.f };
        glm::vec4 specular = { 0.f,0.f,0.f,1.f };
        glm::vec4 normals = { 0.5f,0.5f,1.f,1.f };
        glm::vec4 emission = { 0.0f,0.0f,0.f,1.f };

        int diffuseTexture = -1, specularTexture = -1, normalsTexture = -1,  emissionTexture = -1;

        glm::uvec4 udata = glm::uvec4(0u);
    };
#pragma pack(pop)

    // WIP Materials
    class Material : public std::enable_shared_from_this<Material> {
    public: friend Renderer;// 
        Material() {};
        Material(const vkt::uni_ptr<Context>& context, const uint32_t& MaxMaterialCount = 64u) : context(context), MaxMaterialCount(MaxMaterialCount){ this->construct(); };
        Material(const std::shared_ptr<Context>& context, const uint32_t& MaxMaterialCount = 64u) : context(context), MaxMaterialCount(MaxMaterialCount) { this->construct(); };
        ~Material() {};

        // 
        public: virtual vkt::uni_ptr<Material> sharedPtr() { return shared_from_this(); };
        //virtual vkt::uni_ptr<Material> sharedPtr() const { return std::shared_ptr<Material>(shared_from_this()); };

        // 
        protected: virtual uPTR(Material) construct() {
            this->driver = context->getDriver();
            this->thread = std::make_shared<Thread>(this->driver);

            //
            auto hostUsage = vkh::VkBufferUsageFlags{.eTransferSrc = 1, .eUniformBuffer = 1, .eStorageBuffer = 1, .eRayTracing = 1 };
            auto gpuUsage  = vkh::VkBufferUsageFlags{.eTransferDst = 1, .eUniformBuffer = 1, .eStorageBuffer = 1, .eRayTracing = 1 };

            // 
            this->rawMaterials = vkt::Vector<MaterialUnit>(std::make_shared<vkt::VmaBufferAllocation>(this->driver->getAllocator(), vkh::VkBufferCreateInfo{ .size = sizeof(MaterialUnit) * MaxMaterialCount, .usage = hostUsage }, vkt::VmaMemoryInfo{ .memUsage = VMA_MEMORY_USAGE_CPU_TO_GPU }));
            this->gpuMaterials = vkt::Vector<MaterialUnit>(std::make_shared<vkt::VmaBufferAllocation>(this->driver->getAllocator(), vkh::VkBufferCreateInfo{ .size = sizeof(MaterialUnit) * MaxMaterialCount, .usage = gpuUsage }, vkt::VmaMemoryInfo{ .memUsage = VMA_MEMORY_USAGE_GPU_ONLY }));
            return uTHIS;
        };

        // 
        public: virtual uPTR(Material) setContext(std::shared_ptr<Context> context) {
            this->context = context;
            this->descriptorSetInfo = vkh::VsDescriptorSetCreateInfoHelper(this->context->materialDescriptorSetLayout,this->thread->getDescriptorPool());
            return uTHIS;
        };

        //
        public: virtual uPTR(Material) setRawMaterials(const vkt::uni_arg<vkt::Vector<MaterialUnit>>& rawMaterials = {}, const VkDeviceSize& materialCounter = 0u) {
            this->rawMaterials = *rawMaterials; this->materialCounter = materialCounter;
            return uTHIS;
        };

        // 
        public: virtual uPTR(Material) setGpuMaterials(const vkt::uni_arg<vkt::Vector<MaterialUnit>>& gpuMaterials = {}) {
            this->gpuMaterials = *gpuMaterials;
            return uTHIS;
        };

        // 
        public: virtual VkDeviceSize pushMaterial(const vkt::uni_arg<MaterialUnit>& material = {}) {
            const auto materialID = materialCounter++;
            this->rawMaterials[materialID] = material;
            return materialID;
        };

        // 
        public: virtual uPTR(Material) resetMaterials() {
            materialCounter = 0u;
            return uTHIS;
        };

        // 
        public: virtual uPTR(Material) resetSampledImages() {
            this->sampledImages.resize(0u);
            return uTHIS;
        };

        // 
        public: virtual uint32_t pushSampledImage(const vkt::uni_arg<vkh::VkDescriptorImageInfo>& info = {}) {
            this->sampledImages.push_back(info);
            return this->sampledImages.size()-1;
        };

        // 
        public: virtual uPTR(Material) copyBuffers(const VkCommandBuffer& buildCommand = {}) {
            this->driver->getDeviceDispatch()->CmdCopyBuffer(buildCommand, this->rawMaterials, this->gpuMaterials, 1u, vkh::VkBufferCopy{ this->rawMaterials.offset(), this->gpuMaterials.offset(), this->gpuMaterials.range() });
            return uTHIS;
        };

        // 
        protected: virtual uPTR(Material) createDescriptorSet() {
            this->descriptorSetInfo = vkh::VsDescriptorSetCreateInfoHelper(context->materialDescriptorSetLayout, driver->descriptorPool);

            //
            this->descriptorSetInfo.pushDescription(vkh::VkDescriptorUpdateTemplateEntry{
                    .dstBinding = 20u,
                    .descriptorCount = 1u,
                    .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER
            }).offset<vkh::VkDescriptorBufferInfo>() = (vkh::VkDescriptorBufferInfo&)this->gpuMaterials;

            // 
            if (!this->backgroundImage) { // Make Background Image NOT Needed!
                int width = 2u, height = 2u;
                float* rgba = nullptr;
                const char* err = nullptr;

                // 
                std::vector<glm::vec4> gSkyColor = {
                    glm::vec4(0.9f,0.98,0.999f, 1.f),
                    glm::vec4(0.9f,0.98,0.999f, 1.f),
                    glm::vec4(0.9f,0.98,0.999f, 1.f),
                    glm::vec4(0.9f,0.98,0.999f, 1.f)
                };

                const auto aspectMask = vkh::VkImageAspectFlags{.eColor = 1};
                { //
                    auto bgUsage = vkh::VkImageUsageFlags{.eTransferDst = 1, .eSampled = 1, .eStorage = 1, .eColorAttachment = 1 };
                    this->backgroundImageClass = vkt::ImageRegion(std::make_shared<vkt::VmaImageAllocation>(this->driver->getAllocator(), vkh::VkImageCreateInfo{  // experimental: callify
                        .format = VK_FORMAT_R32G32B32A32_SFLOAT,
                        .extent = {uint32_t(width),uint32_t(height),1u},
                        .usage = bgUsage,
                    }, vkt::VmaMemoryInfo{ .memUsage = VMA_MEMORY_USAGE_GPU_ONLY }), vkh::VkImageViewCreateInfo{
                        .format = VK_FORMAT_R32G32B32A32_SFLOAT,
                        .subresourceRange = {.aspectMask = aspectMask},
                    });

                    // Create Sampler By Reference
                    vkh::handleVk(this->driver->getDeviceDispatch()->CreateSampler(vkh::VkSamplerCreateInfo{
                        .magFilter = VK_FILTER_LINEAR,
                        .minFilter = VK_FILTER_LINEAR,
                        .addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT,
                        .addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
                    }, nullptr, &this->backgroundImageClass.refSampler()));

                    //
                    auto usage = vkh::VkBufferUsageFlags{.eTransferSrc = 1, .eStorageTexelBuffer = 1, .eStorageBuffer = 1, .eIndexBuffer = 1, .eVertexBuffer = 1 };
                    vkt::Vector<> imageBuf = vkt::Vector<>(std::make_shared<vkt::VmaBufferAllocation>(this->driver->getAllocator(), vkh::VkBufferCreateInfo{ // experimental: callify
                        .size = size_t(width) * size_t(height) * sizeof(glm::vec4), .usage = usage,
                    }, vkt::VmaMemoryInfo{ .memUsage = VMA_MEMORY_USAGE_CPU_TO_GPU }));
                    memcpy(imageBuf.data(), rgba = (float*)gSkyColor.data(), size_t(width) * size_t(height) * sizeof(glm::vec4));

                    // 
                    context->getThread()->submitOnce([=](VkCommandBuffer& cmd) {
                        this->backgroundImageClass.transfer(cmd);
                        this->driver->getDeviceDispatch()->CmdCopyBufferToImage(cmd, imageBuf.buffer(), this->backgroundImageClass.getImage(), this->backgroundImageClass.getImageLayout(), 1u, vkh::VkBufferImageCopy{
                            .bufferOffset = imageBuf.offset(),
                            .bufferRowLength = uint32_t(width),
                            .bufferImageHeight = uint32_t(height),
                            .imageSubresource = this->backgroundImageClass.subresourceLayers(),
                            .imageOffset = {0u,0u,0u},
                            .imageExtent = {uint32_t(width),uint32_t(height),1u},
                        });
                    });

                    // 
                    this->backgroundImage = this->backgroundImageClass.getDescriptor();
                };
            };

            // 
            if (this->backgroundImage) { // 
                vkh::VsDescriptorHandle<vkh::VkDescriptorImageInfo> imagesHandle = this->descriptorSetInfo.pushDescription(vkh::VkDescriptorUpdateTemplateEntry{
                    .dstBinding = 21u,
                    .descriptorCount = uint32_t(1u),
                    .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER
                });
                imagesHandle.offset<vkh::VkDescriptorImageInfo>(0) = *backgroundImage;
            };

            //
            if (sampledImages.size() > 0u) { // Setup Textures
                vkh::VsDescriptorHandle<vkh::VkDescriptorImageInfo> imagesHandle = this->descriptorSetInfo.pushDescription(vkh::VkDescriptorUpdateTemplateEntry{
                        .dstBinding = 22u,
                        .descriptorCount = uint32_t(sampledImages.size()),
                        .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER
                });
                memcpy(&imagesHandle.offset<vkh::VkDescriptorImageInfo>(), sampledImages.data(), sampledImages.size() * sizeof(vkh::VkDescriptorImageInfo));
                //imagesHandle.offset<vkh::VkDescriptorImageInfo>(uint32_t(sampledImages.size())) = sampledImages[uint32_t(sampledImages.size())-1u];
                //for (uint32_t i = 0u; i < sampledImages.size(); i++) { imagesHandle.offset<vkh::VkDescriptorImageInfo>(i) = sampledImages[i]; };
            };

            // Reprojection WILL NOT write own depth... 
            vkh::handleVk(vkt::AllocateDescriptorSetWithUpdate(this->driver->getDeviceDispatch(), this->descriptorSetInfo, this->descriptorSet, this->descriptorUpdated));
            this->context->descriptorSets[4] = this->descriptorSet;

            // 
            return uTHIS;
        };

        // 
        public: virtual uPTR(Material) setBackgroundImage(const vkh::VkDescriptorImageInfo& backgroundImage = {}) {
            this->backgroundImage = backgroundImage; return uTHIS;
        };

    protected: // 
        std::vector<vkh::VkDescriptorImageInfo> sampledImages = {};
        std::optional<vkh::VkDescriptorImageInfo> backgroundImage = {};
        vkt::ImageRegion backgroundImageClass = {};
        bool descriptorUpdated = false;

        // 
        vkt::Vector<MaterialUnit> rawMaterials = {}; // Ray-Tracing instances Will re-located into meshes by Index, and will no depending by mesh list...
        vkt::Vector<MaterialUnit> gpuMaterials = {};

        // 
        vkh::VsDescriptorSetCreateInfoHelper descriptorSetInfo = {};
        VkDescriptorSet descriptorSet = VK_NULL_HANDLE;

        // 
        uintptr_t MaxMaterialCount = 64u;
        VkDeviceSize materialCounter = 0u;
        vkt::uni_ptr<Driver> driver = {};
        vkt::uni_ptr<Thread> thread = {};
        vkt::uni_ptr<Context> context = {};
    };
};
