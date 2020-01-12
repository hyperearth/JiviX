#pragma once
#include "./config.hpp"
#include "./driver.hpp"
#include "./thread.hpp"
namespace lancer {

    struct Matrices {
        glm::mat4 projection;
        glm::mat4 projectionInv;
        glm::mat3x4 modelview;
        glm::mat3x4 modelviewInv;
    };

    // TODO: Full Context Support
    class Context : public std::enable_shared_from_this<Context> { public: friend Mesh; friend Node; friend Driver; friend Material; friend Renderer;
        Context() {};
        Context(const std::shared_ptr<Driver>& driver) {
            this->driver = driver;
            this->thread = std::make_shared<Thread>(this->driver);
            this->uniformGPUData = vkt::Vector<Matrices>(std::make_shared<vkt::VmaBufferAllocation>(this->driver->getAllocator(), vkh::VkBufferCreateInfo{ .size = sizeof(Matrices) * 2u, .usage = { .eTransferSrc = 1, .eTransferDst = 1, .eUniformBuffer = 1, .eStorageBuffer = 1, .eRayTracing = 1 } }, VMA_MEMORY_USAGE_GPU_ONLY));
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
        //vk::Framebuffer& refFramebuffer() { return framebuffer; };

        // 
        const vk::Rect2D& refScissor() const { return scissor; };
        const vk::Viewport& refViewport() const { return viewport; };
        const vk::RenderPass& refRenderPass() const { return renderPass; };
        //const vk::Framebuffer& refFramebuffer() const { return framebuffer; };

        // 
        std::shared_ptr<Driver>& getDriver() { return driver; };
        const std::shared_ptr<Driver>& getDriver() const { return driver; };

        // 
        std::shared_ptr<Context> createRenderPass() { // 
            vkh::VsRenderPassCreateInfoHelper rpsInfo = {};

            for (uint32_t b=0u;b<4u;b++) {
                rpsInfo.addColorAttachment(vkh::VkAttachmentDescription{
                    .format = VK_FORMAT_R32G32B32A32_SFLOAT,
                    .loadOp = VK_ATTACHMENT_LOAD_OP_LOAD,
                    .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
                    .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
                    .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
                    .initialLayout = VK_IMAGE_LAYOUT_GENERAL,
                    .finalLayout = VK_IMAGE_LAYOUT_GENERAL,
                });
            };

            // 
            rpsInfo.setDepthStencilAttachment(vkh::VkAttachmentDescription{
                .format = VK_FORMAT_D32_SFLOAT_S8_UINT,
                .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
                .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
                .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
                .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
                .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
                .finalLayout = VK_IMAGE_LAYOUT_GENERAL//VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
            });

            // 
            rpsInfo.addSubpassDependency(vkh::VkSubpassDependency{
                .srcSubpass = VK_SUBPASS_EXTERNAL,
                .dstSubpass = 0u,
                .srcStageMask = {.eColorAttachmentOutput = 1, .eTransfer = 1, .eBottomOfPipe = 1},
                .dstStageMask = {.eColorAttachmentOutput = 1},
                .srcAccessMask = {.eColorAttachmentWrite = 1},
                .dstAccessMask = {.eColorAttachmentRead = 1, .eColorAttachmentWrite = 1},
                .dependencyFlags = {.eByRegion = 1}
            });

            // 
            rpsInfo.addSubpassDependency(vkh::VkSubpassDependency{
                .srcSubpass = 0u,
                .dstSubpass = VK_SUBPASS_EXTERNAL,
                .srcStageMask = {.eColorAttachmentOutput = 1},
                .dstStageMask = {.eTopOfPipe = 1, .eColorAttachmentOutput = 1, .eTransfer = 1},
                .srcAccessMask = {.eColorAttachmentRead = 1, .eColorAttachmentWrite = 1},
                .dstAccessMask = {.eColorAttachmentRead = 1, .eColorAttachmentWrite = 1},
                .dependencyFlags = {.eByRegion = 1}
            });

            // 
            this->renderPass = driver->getDevice().createRenderPass(rpsInfo);

            // 
            return shared_from_this();
        };

        // 
        std::array<vk::DescriptorSet,5u>& getDescriptorSets(){
            return descriptorSets;
        };

        // 
        vk::PipelineLayout getPipelineLayout(){
            return unifiedPipelineLayout;
        };

        // 
        std::shared_ptr<Context> createFramebuffers(const uint32_t& width = 800u, const uint32_t& height = 600u) { // 
            std::array<VkImageView, 5u> deferredAttachments = {};
            std::array<VkImageView, 5u> samplingAttachments = {};

            // 
            for (uint32_t b=0u;b<4u;b++) { // 
                frameBfImages[b] = vkt::ImageRegion(std::make_shared<vkt::VmaImageAllocation>(driver->getAllocator(), vkh::VkImageCreateInfo{ 
                    .format = VK_FORMAT_R32G32B32A32_SFLOAT, 
                    .extent = {width,height,1u}, 
                    .usage = { .eTransferDst = 1, .eSampled = 1, .eStorage = 1, .eColorAttachment = 1 }, 
                }), vkh::VkImageViewCreateInfo{
                    .format = VK_FORMAT_R32G32B32A32_SFLOAT,
                });
                deferredAttachments[b] = frameBfImages[b];
            };

            // 
            for (uint32_t b=0u;b<4u;b++) { // 
                samplesImages[b] = vkt::ImageRegion(std::make_shared<vkt::VmaImageAllocation>(driver->getAllocator(), vkh::VkImageCreateInfo{ 
                    .format = VK_FORMAT_R32G32B32A32_SFLOAT, 
                    .extent = {width,height,1u}, 
                    .usage = { .eTransferDst = 1, .eSampled = 1, .eStorage = 1, .eColorAttachment = 1 }, 
                }), vkh::VkImageViewCreateInfo{
                    .format = VK_FORMAT_R32G32B32A32_SFLOAT,
                });
                samplingAttachments[b] = samplesImages[b];
            };

            // 
            depthImage = vkt::ImageRegion(std::make_shared<vkt::VmaImageAllocation>(driver->getAllocator(), vkh::VkImageCreateInfo{
                .format = VK_FORMAT_D32_SFLOAT_S8_UINT,
                .extent = {width,height,1u},
                .usage = { .eTransferDst = 1, .eDepthStencilAttachment = 1 },
            }), vkh::VkImageViewCreateInfo{
                .format = VK_FORMAT_D32_SFLOAT_S8_UINT,
                .subresourceRange = { .aspectMask = { .eDepth = 1 } },

            });
            
            // 5th attachment
            deferredAttachments[4u] = depthImage;
            samplingAttachments[4u] = depthImage;

            // 
            deferredFramebuffer = driver->getDevice().createFramebuffer(vkh::VkFramebufferCreateInfo{
                .renderPass = renderPass,
                .attachmentCount = deferredAttachments.size(),
                .pAttachments = deferredAttachments.data(),
                .width = width,
                .height = height
            });
            
            // Reprojection WILL NOT write own depth... 
            samplingFramebuffer = driver->getDevice().createFramebuffer(vkh::VkFramebufferCreateInfo{
                .renderPass = renderPass,
                .attachmentCount = samplingAttachments.size(),
                .pAttachments = samplingAttachments.data(),
                .width = width,
                .height = height
            });

            // TODO: controllable 
            glm::mat4 projected = glm::perspective(110.f*glm::pi<float>()/180.f, float(width) / float(height), 0.0001f, 10000.f);
            glm::mat4 modelview = glm::lookAt(glm::vec3(0.f,1.f,1.f),glm::vec3(0.f,0.f,0.f),glm::vec3(0.f,1.f,0.f));

            // 
            uniformData.modelview = glm::transpose(modelview);
            uniformData.modelviewInv = glm::transpose(glm::inverse(modelview));
            uniformData.projection = glm::transpose(projected);
            uniformData.projectionInv = glm::transpose(glm::inverse(projected));

            // 
            scissor = vk::Rect2D{ vk::Offset2D(0, 0), vk::Extent2D(width, height) };
            viewport = vk::Viewport{ 0.0f, 0.0f, static_cast<float>(scissor.extent.width), static_cast<float>(scissor.extent.height), 0.f, 1.f };

            //  
            vkt::submitOnce(*thread, *thread, *thread, [&,this](vk::CommandBuffer& cmd) { for (uint32_t i = 0u; i < 4u; i++) { // Definitely Not an Hotel
                vkt::imageBarrier(cmd, vkt::ImageBarrierInfo{.image = this->frameBfImages[i], .targetLayout = vk::ImageLayout::eGeneral, .originLayout = vk::ImageLayout::eUndefined, .subresourceRange = this->frameBfImages[i] });
                vkt::imageBarrier(cmd, vkt::ImageBarrierInfo{.image = this->samplesImages[i], .targetLayout = vk::ImageLayout::eGeneral, .originLayout = vk::ImageLayout::eUndefined, .subresourceRange = this->samplesImages[i] });
            };});

            // 
            return shared_from_this();
        };

        // 
        std::shared_ptr<Context> createDescriptorSetLayouts() { // reset layout descriptions
            this->meshDataDescriptorSetLayoutHelper = {};
            this->bindingsDescriptorSetLayoutHelper = {};
            this->samplingDescriptorSetLayoutHelper = {};
            this->deferredDescriptorSetLayoutHelper = {};
            this->materialDescriptorSetLayoutHelper = {};

            for (uint32_t b=0u;b<8u;b++) { // For Ray Tracers
                this->meshDataDescriptorSetLayoutHelper.pushBinding(vkh::VkDescriptorSetLayoutBinding{ .binding = b, .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER     , .descriptorCount =   64u, .stageFlags = { .eCompute = 1, .eRaygen = 1, .eClosestHit = 1 } },vkh::VkDescriptorBindingFlagsEXT{ .ePartiallyBound = 1});
            };

            // TODO: Inline Ray Tracing in Compute Shaders (as in DirectX 12.1)
            // TODO: Open Issue for Vulkan API to implement inline Ray-Tracing in Compute Shaders
            // https://github.com/KhronosGroup/Vulkan-Docs/issues/1155
            this->bindingsDescriptorSetLayoutHelper.pushBinding(vkh::VkDescriptorSetLayoutBinding{ .binding = 0u, .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER           , .descriptorCount =   64u, .stageFlags = { .eVertex = 1, .eFragment = 1, .eCompute = 1, .eRaygen = 1, .eClosestHit = 1 } }, vkh::VkDescriptorBindingFlagsEXT{ .ePartiallyBound = 1 });
            this->bindingsDescriptorSetLayoutHelper.pushBinding(vkh::VkDescriptorSetLayoutBinding{ .binding = 1u, .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER           , .descriptorCount =   64u, .stageFlags = { .eVertex = 1, .eFragment = 1, .eCompute = 1, .eRaygen = 1, .eClosestHit = 1 } }, vkh::VkDescriptorBindingFlagsEXT{ .ePartiallyBound = 1 });
            this->bindingsDescriptorSetLayoutHelper.pushBinding(vkh::VkDescriptorSetLayoutBinding{ .binding = 2u, .descriptorType = VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_NV, .descriptorCount =    1u, .stageFlags = { .eVertex = 1, .eFragment = 1, .eCompute = 1, .eRaygen = 1, .eClosestHit = 1 } }, vkh::VkDescriptorBindingFlagsEXT{ .ePartiallyBound = 1 });
            this->bindingsDescriptorSetLayoutHelper.pushBinding(vkh::VkDescriptorSetLayoutBinding{ .binding = 3u, .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER           , .descriptorCount =    1u, .stageFlags = { .eVertex = 1, .eFragment = 1, .eCompute = 1, .eRaygen = 1, .eClosestHit = 1 } }, vkh::VkDescriptorBindingFlagsEXT{ .ePartiallyBound = 1 });

            // 
            this->samplingDescriptorSetLayoutHelper.pushBinding(vkh::VkDescriptorSetLayoutBinding{ .binding = 0u, .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE         , .descriptorCount =   4u, .stageFlags = { .eVertex = 1, .eFragment = 1, .eCompute = 1, .eRaygen = 1, .eClosestHit = 1 } },vkh::VkDescriptorBindingFlagsEXT{ .ePartiallyBound = 1});
            this->samplingDescriptorSetLayoutHelper.pushBinding(vkh::VkDescriptorSetLayoutBinding{ .binding = 1u, .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER        , .descriptorCount =   4u, .stageFlags = { .eVertex = 1, .eFragment = 1, .eCompute = 1, .eRaygen = 1, .eClosestHit = 1 } },vkh::VkDescriptorBindingFlagsEXT{ .ePartiallyBound = 1});

            // 
            this->deferredDescriptorSetLayoutHelper.pushBinding(vkh::VkDescriptorSetLayoutBinding{ .binding = 0u, .descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE         , .descriptorCount =   4u, .stageFlags = { .eVertex = 1, .eFragment = 1, .eCompute = 1, .eRaygen = 1, .eClosestHit = 1 } },vkh::VkDescriptorBindingFlagsEXT{ .ePartiallyBound = 1});

            // 
            this->materialDescriptorSetLayoutHelper.pushBinding(vkh::VkDescriptorSetLayoutBinding{ .binding = 0u, .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, .descriptorCount = 128u, .stageFlags = { .eVertex = 1, .eFragment = 1, .eCompute = 1, .eRaygen = 1, .eClosestHit = 1 } },vkh::VkDescriptorBindingFlagsEXT{ .ePartiallyBound = 1});
            this->materialDescriptorSetLayoutHelper.pushBinding(vkh::VkDescriptorSetLayoutBinding{ .binding = 1u, .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER        , .descriptorCount =   8u, .stageFlags = { .eVertex = 1, .eFragment = 1, .eCompute = 1, .eRaygen = 1, .eClosestHit = 1 } },vkh::VkDescriptorBindingFlagsEXT{ .ePartiallyBound = 1});

            // 
            this->materialDescriptorSetLayout = driver->getDevice().createDescriptorSetLayout(materialDescriptorSetLayoutHelper);
            this->deferredDescriptorSetLayout = driver->getDevice().createDescriptorSetLayout(deferredDescriptorSetLayoutHelper);
            this->samplingDescriptorSetLayout = driver->getDevice().createDescriptorSetLayout(samplingDescriptorSetLayoutHelper);
            this->bindingsDescriptorSetLayout = driver->getDevice().createDescriptorSetLayout(bindingsDescriptorSetLayoutHelper);
            this->meshDataDescriptorSetLayout = driver->getDevice().createDescriptorSetLayout(meshDataDescriptorSetLayoutHelper);

            // 
            std::vector<VkDescriptorSetLayout> layouts = { meshDataDescriptorSetLayout, bindingsDescriptorSetLayout, deferredDescriptorSetLayout, samplingDescriptorSetLayout, materialDescriptorSetLayout };
            this->unifiedPipelineLayout = driver->getDevice().createPipelineLayout(vkh::VkPipelineLayoutCreateInfo{}.setSetLayouts(layouts));
            return shared_from_this();
        };

        // 
        std::shared_ptr<Context> createDescriptorSets() {
            if (!this->unifiedPipelineLayout) { this->createDescriptorSetLayouts(); };

            std::array<VkDescriptorImageInfo, 4u> descriptions = {};
            { // For Deferred Rendering
                for (uint32_t b = 0u; b < 4u; b++) { descriptions[b] = frameBfImages[b]; };

                // 
                vkh::VsDescriptorSetCreateInfoHelper descInfo(deferredDescriptorSetLayout, thread->getDescriptorPool());
                vkh::VsDescriptorHandle<VkDescriptorImageInfo> handle = descInfo.pushDescription(vkh::VkDescriptorUpdateTemplateEntry{
                    .dstBinding = 0u,
                    .descriptorCount = 4u,
                    .descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE
                });
                //for (uint32_t i = 0u; i < descriptions.size(); i++) { handle.offset<VkDescriptorImageInfo>(i) = descriptions[i]; };
                memcpy(&handle.offset<VkDescriptorImageInfo>(), descriptions.data(), descriptions.size()*sizeof(VkDescriptorImageInfo));

                // 
                this->driver->getDevice().updateDescriptorSets(vkt::vector_cast<vk::WriteDescriptorSet,vkh::VkWriteDescriptorSet>(descInfo.setDescriptorSet(
                    this->deferredDescriptorSet = driver->getDevice().allocateDescriptorSets(descInfo)[0]
                )),{});
            };

            { // For Reprojection Pipeline
                for (uint32_t b = 0u; b < 4u; b++) { descriptions[b] = samplesImages[b]; };

                // 
                vkh::VsDescriptorSetCreateInfoHelper descInfo(samplingDescriptorSetLayout, thread->getDescriptorPool());
                vkh::VsDescriptorHandle<VkDescriptorImageInfo> handle = descInfo.pushDescription(vkh::VkDescriptorUpdateTemplateEntry{
                    .dstBinding = 0u,
                    .descriptorCount = 4u,
                    .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE
                });
                //for (uint32_t i = 0u; i < descriptions.size(); i++) { handle.offset<VkDescriptorImageInfo>(i) = descriptions[i]; };
                memcpy(&handle.offset<VkDescriptorImageInfo>(), descriptions.data(), descriptions.size()*sizeof(VkDescriptorImageInfo));

                // Reprojection WILL NOT write own depth... 
                this->driver->getDevice().updateDescriptorSets(vkt::vector_cast<vk::WriteDescriptorSet,vkh::VkWriteDescriptorSet>(descInfo.setDescriptorSet(
                    this->samplingDescriptorSet = driver->getDevice().allocateDescriptorSets(descInfo)[0]
                )),{});
            };

            // 
            this->descriptorSets[2] = this->deferredDescriptorSet;
            this->descriptorSets[3] = this->samplingDescriptorSet;
            
            // 
            return shared_from_this();
        };

        // 
        std::shared_ptr<Context> initialize(const uint32_t& width = 800u, const uint32_t& height = 600u) {
            this->createRenderPass();
            this->createFramebuffers(width,height);
            this->createDescriptorSetLayouts();
            this->createDescriptorSets();
            return shared_from_this();
        };

    protected: // 
        vk::Rect2D scissor = {};
        vk::Viewport viewport = {};
        vk::RenderPass renderPass = {};
        vk::Framebuffer samplingFramebuffer = {};
        vk::Framebuffer deferredFramebuffer = {};

        // 
        vkt::Vector<Matrices> uniformGPUData = {};
        Matrices uniformData = {};

        // Image Buffers
        std::array<vkt::ImageRegion,4u> samplesImages = {}; // Path Tracing
        std::array<vkt::ImageRegion,4u> frameBfImages = {}; // Rasterization
        std::array<vk::DescriptorSet,5u> descriptorSets = {};
        vkt::ImageRegion depthImage = {};

        // 
        vk::DescriptorSet deferredDescriptorSet = {};
        vk::DescriptorSet samplingDescriptorSet = {};
        vk::PipelineLayout unifiedPipelineLayout = {};

        // 
        vk::DescriptorSetLayout materialDescriptorSetLayout = {}; // Material Descriptions
        vk::DescriptorSetLayout deferredDescriptorSetLayout = {}; // Deferred Shading Descriptions (Diffuse Texturing)
        vk::DescriptorSetLayout meshDataDescriptorSetLayout = {}; // Packed Mesh Data (8-bindings)
        vk::DescriptorSetLayout samplingDescriptorSetLayout = {}; // Framebuffers and Samples (Diffuse, Path-Tracing and ReProjection)
        vk::DescriptorSetLayout bindingsDescriptorSetLayout = {}; // Bindings, Attributes Descriptions
        
        // 
        vkh::VsDescriptorSetLayoutCreateInfoHelper materialDescriptorSetLayoutHelper = {};
        vkh::VsDescriptorSetLayoutCreateInfoHelper deferredDescriptorSetLayoutHelper = {};
        vkh::VsDescriptorSetLayoutCreateInfoHelper meshDataDescriptorSetLayoutHelper = {};
        vkh::VsDescriptorSetLayoutCreateInfoHelper samplingDescriptorSetLayoutHelper = {};
        vkh::VsDescriptorSetLayoutCreateInfoHelper bindingsDescriptorSetLayoutHelper = {};
        
        // 
        std::shared_ptr<Driver> driver = {};
        std::shared_ptr<Thread> thread = {};
    };

};
