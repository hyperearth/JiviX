#pragma once
#include "./config.hpp"
#include "./driver.hpp"
#include "./thread.hpp"
namespace lancer {

    // TODO: Full Context Support
    class Context : public std::enable_shared_from_this<Context> { public: friend Mesh; friend Instance; friend Driver;
        Context(const std::shared_ptr<Driver>& driver){
            this->driver = driver;
            this->thread = std::make_shared<Thread>(this->driver);
        };

        // 
        std::shared_ptr<Context> setThread(const std::shared_ptr<Thread>& thread) {
            this->thread = thread;
            return shared_from_this();
        };

        // 
        vk::Rect2D& refScissor() { return scissor; };
        vk::Viewport& refViewport() { return viewport; };
        vk::RenderPass& refRenderPass() { return renderPass; };
        vk::Framebuffer& refFramebuffer() { return framebuffer; };

        // 
        const vk::Rect2D& refScissor() const { return scissor; };
        const vk::Viewport& refViewport() const { return viewport; };
        const vk::RenderPass& refRenderPass() const { return renderPass; };
        const vk::Framebuffer& refFramebuffer() const { return framebuffer; };

        // 
        std::shared_ptr<Context> createFramebuffers() {

            return shared_from_this();
        };

        // 
        std::shared_ptr<Context> createDescriptorSetLayouts() { // 
            for (uint32_t b=0u;b<8u;b++) { // For Ray Tracers
                meshDataDescriptorSetLayoutHelper.pushBinding(vkh::VkDescriptorSetLayoutBinding{ .binding = b, .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER     , .descriptorCount =   64u, .stageFlags = { .eCompute = 1, .eRaygen = 1, .eClosestHit = 1 } },vkh::VkDescriptorBindingFlagsEXT{.ePartiallyBound = 1});
            };

            //
            bindingsDescriptorSetLayoutHelper.pushBinding(vkh::VkDescriptorSetLayoutBinding{ .binding = 0u, .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER        , .descriptorCount =   1u, .stageFlags = { .eCompute = 1, .eRaygen = 1, .eClosestHit = 1 } },vkh::VkDescriptorBindingFlagsEXT{.ePartiallyBound = 1});
            bindingsDescriptorSetLayoutHelper.pushBinding(vkh::VkDescriptorSetLayoutBinding{ .binding = 1u, .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER        , .descriptorCount =   1u, .stageFlags = { .eCompute = 1, .eRaygen = 1, .eClosestHit = 1 } },vkh::VkDescriptorBindingFlagsEXT{.ePartiallyBound = 1});

            // 
            samplingDescriptorSetLayoutHelper.pushBinding(vkh::VkDescriptorSetLayoutBinding{ .binding = 0u, .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE         , .descriptorCount =   8u, .stageFlags = { .eCompute = 1, .eRaygen = 1, .eClosestHit = 1 } },vkh::VkDescriptorBindingFlagsEXT{.ePartiallyBound = 1});
            rayTraceDescriptorSetLayoutHelper.pushBinding(vkh::VkDescriptorSetLayoutBinding{ .binding = 0u, .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE         , .descriptorCount =   8u, .stageFlags = { .eCompute = 1, .eRaygen = 1, .eClosestHit = 1 } },vkh::VkDescriptorBindingFlagsEXT{.ePartiallyBound = 1});
            rayTraceDescriptorSetLayoutHelper.pushBinding(vkh::VkDescriptorSetLayoutBinding{ .binding = 1u, .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER        , .descriptorCount =   8u, .stageFlags = { .eCompute = 1, .eRaygen = 1, .eClosestHit = 1 } },vkh::VkDescriptorBindingFlagsEXT{.ePartiallyBound = 1});

            // 
            materialDescriptorSetLayoutHelper.pushBinding(vkh::VkDescriptorSetLayoutBinding{ .binding = 0u, .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, .descriptorCount = 128u, .stageFlags = { .eCompute = 1, .eRaygen = 1, .eClosestHit = 1 } },vkh::VkDescriptorBindingFlagsEXT{.ePartiallyBound = 1});
            materialDescriptorSetLayoutHelper.pushBinding(vkh::VkDescriptorSetLayoutBinding{ .binding = 1u, .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER        , .descriptorCount =   8u, .stageFlags = { .eCompute = 1, .eRaygen = 1, .eClosestHit = 1 } },vkh::VkDescriptorBindingFlagsEXT{.ePartiallyBound = 1});

            return shared_from_this();
        };
        
    protected: // 
        vk::Rect2D scissor = {};
        vk::Viewport viewport = {};
        vk::RenderPass renderPass = {};
        vk::Framebuffer framebuffer = {};

        // 
        std::array<vkt::ImageRegion,8u> frameImages = {};
        vkt::ImageRegion depthImage = {};

        // 
        vk::PipelineLayout unifiedPipelineLayout = {};
        vk::DescriptorSetLayout materialDescriptorSetLayout = {}; // Material Descriptions
        vk::DescriptorSetLayout rayTraceDescriptorSetLayout = {}; // Ray-Traced Data
        vk::DescriptorSetLayout meshDataDescriptorSetLayout = {}; // Packed Mesh Data (8-bindings)
        vk::DescriptorSetLayout samplingDescriptorSetLayout = {}; // Framebuffers and Samples (Diffuse, Path-Tracing and Reprojection)
        vk::DescriptorSetLayout bindingsDescriptorSetLayout = {}; // Bindings, Attributes Descriptions

        // 
        vkh::VsDescriptorSetLayoutCreateInfoHelper materialDescriptorSetLayoutHelper = {};
        vkh::VsDescriptorSetLayoutCreateInfoHelper rayTraceDescriptorSetLayoutHelper = {};
        vkh::VsDescriptorSetLayoutCreateInfoHelper meshDataDescriptorSetLayoutHelper = {};
        vkh::VsDescriptorSetLayoutCreateInfoHelper samplingDescriptorSetLayoutHelper = {};
        vkh::VsDescriptorSetLayoutCreateInfoHelper bindingsDescriptorSetLayoutHelper = {};

        // 
        std::shared_ptr<Driver> driver = {};
        std::shared_ptr<Thread> thread = {};
    };

};
