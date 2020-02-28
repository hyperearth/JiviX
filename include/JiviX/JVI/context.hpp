#pragma once // #

#include <memory>
#include <chrono>
#include "./config.hpp"
#include "./driver.hpp"
#include "./thread.hpp"

namespace jvi {

#pragma pack(push, 1)
    struct Matrices {
        glm::mat4 projection = glm::mat4(1.f);
        glm::mat4 projectionInv = glm::mat4(1.f);
        glm::mat3x4 modelview = glm::mat3x4(1.f);
        glm::mat3x4 modelviewInv = glm::mat3x4(1.f);
        glm::uvec4 mdata = glm::uvec4(0u);                         // mesh mutation or modification data
        //glm::uvec2 tdata = glm::uvec2(0u), rdata = glm::uvec2(0u); // first for time, second for randoms
        glm::uvec2 tdata = glm::uvec2(0u);
        glm::uvec2 rdata = glm::uvec2(0u);
    };
#pragma pack(pop)

    // TODO: Full Context Support
    class Context : public std::enable_shared_from_this<Context> { public: friend Mesh; friend Node; friend Driver; friend Material; friend Renderer;
        Context() {};
        Context(const std::shared_ptr<Driver>& driver) {
            this->driver = driver;
            this->thread = std::make_shared<Thread>(this->driver);
            this->uniformGPUData = vkt::Vector<Matrices>(std::make_shared<vkt::VmaBufferAllocation>(this->driver->getAllocator(), vkh::VkBufferCreateInfo{ .size = sizeof(Matrices) * 2u, .usage = { .eTransferDst = 1, .eUniformBuffer = 1, .eStorageBuffer = 1, .eRayTracing = 1 } }, VMA_MEMORY_USAGE_GPU_ONLY));
            this->uniformRawData = vkt::Vector<Matrices>(std::make_shared<vkt::VmaBufferAllocation>(this->driver->getAllocator(), vkh::VkBufferCreateInfo{ .size = sizeof(Matrices) * 2u, .usage = { .eTransferSrc = 1, .eUniformBuffer = 1, .eStorageBuffer = 1, .eRayTracing = 1 } }, VMA_MEMORY_USAGE_CPU_TO_GPU));
            this->beginTime = std::chrono::high_resolution_clock::now();
            this->leastTime = std::chrono::high_resolution_clock::now();
            this->previTime = std::chrono::high_resolution_clock::now();

            // 
            glm::mat4x4 projected = glm::perspective(80.f / 180.f * glm::pi<float>(), float(1600) / float(1200), 0.0001f, 10000.f);
            glm::mat4x4 modelview = glm::lookAt(glm::vec3(0.f, 0.f, 1.f), glm::vec3(0.f, 0.f, 0.f), glm::vec3(0.f, 1.f, 0.f));

            // 
            uniformRawData[0].modelview = glm::transpose(modelview);
            uniformRawData[0].modelviewInv = glm::transpose(glm::inverse(modelview));
            uniformRawData[0].projection = glm::transpose(projected);
            uniformRawData[0].projectionInv = glm::transpose(glm::inverse(projected));
        };

        // 
        std::shared_ptr<Context> setThread(const std::shared_ptr<Thread>& thread) {
            this->thread = thread;
            return shared_from_this();
        };

        std::shared_ptr<Thread>& getThread() {
            return this->thread;
        };

        const std::shared_ptr<Thread>& getThread() const {
            return this->thread;
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

            for (uint32_t b=0u;b<8u;b++) {
                rpsInfo.addColorAttachment(vkh::VkAttachmentDescription{
                    .format = VK_FORMAT_R32G32B32A32_SFLOAT,
                    .loadOp = VK_ATTACHMENT_LOAD_OP_LOAD,
                    .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
                    .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
                    .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
                    .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
                    .finalLayout = VK_IMAGE_LAYOUT_GENERAL,
                });
            };

            // 
            rpsInfo.setDepthStencilAttachment(vkh::VkAttachmentDescription{
                .format = VK_FORMAT_D32_SFLOAT_S8_UINT,
                .loadOp = VK_ATTACHMENT_LOAD_OP_LOAD,
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
        std::array<vk::DescriptorSet,5u>& getDescriptorSets() {
            return descriptorSets;
        };

        // 
        vk::PipelineLayout getPipelineLayout() {
            return unifiedPipelineLayout;
        };

        // 
        std::shared_ptr<Context> registerTime() {
            this->previTime = this->leastTime;
            uniformRawData[0].tdata[0] = std::chrono::duration_cast<std::chrono::milliseconds>((this->leastTime = std::chrono::high_resolution_clock::now()) - this->beginTime).count(); // time from beginning
            uniformRawData[0].tdata[1] = std::chrono::duration_cast<std::chrono::milliseconds>(this->leastTime - this->previTime).count(); // difference 
            return shared_from_this();
        };

        // 
        uint32_t& drawTime() { return uniformRawData[0].tdata[0u]; };
        uint32_t& timeDiff() { return uniformRawData[0].tdata[1u]; };
        const uint32_t& drawTime() const { return uniformRawData[0].tdata[0u]; };
        const uint32_t& timeDiff() const { return uniformRawData[0].tdata[1u]; };

        // 
        std::shared_ptr<Context> setDrawCount(const uint32_t& count = 0u) {
            uniformRawData[0].rdata[0] = count;
            return shared_from_this();
        };

        // 
        std::shared_ptr<Context> setPerspective(const glm::mat4x4& persp = glm::mat4(1.f)) {
            uniformRawData[0].projection = glm::transpose(persp);
            uniformRawData[0].projectionInv = glm::transpose(glm::inverse(persp));
            return shared_from_this();
        };

        // 
        std::shared_ptr<Context> setModelView(const glm::mat4x4& mv = glm::mat3x4(1.f)) {
            uniformRawData[0].modelview = glm::transpose(mv);
            uniformRawData[0].modelviewInv = glm::transpose(glm::inverse(mv));
            return shared_from_this();
        };

        // 
        std::array<vkt::ImageRegion, 8u>& getFlip1Buffers() {
            return this->smFlip1Images;
        }

        // 
        const std::array<vkt::ImageRegion, 8u>& const getFlip1Buffers() const {
            return this->smFlip1Images;
        }

        // 
        std::array<vkt::ImageRegion, 8u>& getFrameBuffers() {
            return this->frameBfImages;
        }

        // 
        const std::array<vkt::ImageRegion, 8u>& const getFrameBuffers() const {
            return this->frameBfImages;
        }

        // 
        std::shared_ptr<Context> createFramebuffers(const uint32_t& width = 800u, const uint32_t& height = 600u) { // 
            std::array<VkImageView, 9u> deferredAttachments = {};
            std::array<VkImageView, 9u> smpFlip0Attachments = {};
            std::array<VkImageView, 9u> smpFlip1Attachments = {};

            // 
            auto allocInfo = vkt::MemoryAllocationInfo{};
            allocInfo.device = *driver;
            allocInfo.memoryProperties = driver->getMemoryProperties().memoryProperties;
            allocInfo.dispatch = driver->getDispatch();

            // 
            for (uint32_t b = 0u; b < 8u; b++) { // 
                deferredAttachments[b] = frameBfImages[b] = vkt::ImageRegion(std::make_shared<vkt::ImageAllocation>(allocInfo, vkh::VkImageCreateInfo{
                    .format = VK_FORMAT_R32G32B32A32_SFLOAT,
                    .extent = {width,height,1u},
                    .usage = {.eTransferDst = 1, .eSampled = 1, .eStorage = 1, .eColorAttachment = 1 },
                }), vkh::VkImageViewCreateInfo{
                    .format = VK_FORMAT_R32G32B32A32_SFLOAT,
                }).setSampler(driver->device.createSampler(vkh::VkSamplerCreateInfo{
                    .magFilter = VK_FILTER_LINEAR,
                    .minFilter = VK_FILTER_LINEAR,
                    .addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
                    .addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
                    .unnormalizedCoordinates = true,
                }));
            };

            // 
            for (uint32_t b=0u;b<8u;b++) { // 
                smpFlip0Attachments[b] = smFlip0Images[b] = vkt::ImageRegion(std::make_shared<vkt::ImageAllocation>(allocInfo, vkh::VkImageCreateInfo{
                    .format = VK_FORMAT_R32G32B32A32_SFLOAT,
                    .extent = {width,height,1u},
                    .usage = {.eTransferDst = 1, .eSampled = 1, .eStorage = 1, .eColorAttachment = 1 },
                }), vkh::VkImageViewCreateInfo{
                    .format = VK_FORMAT_R32G32B32A32_SFLOAT,
                }).setSampler(driver->device.createSampler(vkh::VkSamplerCreateInfo{
                    .magFilter = VK_FILTER_LINEAR,
                    .minFilter = VK_FILTER_LINEAR,
                    .addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
                    .addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
                    .unnormalizedCoordinates = true,
                }));

                smpFlip1Attachments[b] = smFlip1Images[b] = vkt::ImageRegion(std::make_shared<vkt::ImageAllocation>(allocInfo, vkh::VkImageCreateInfo{
                    .format = VK_FORMAT_R32G32B32A32_SFLOAT,
                    .extent = {width,height,1u},
                    .usage = {.eTransferDst = 1, .eSampled = 1, .eStorage = 1, .eColorAttachment = 1 },
                }), vkh::VkImageViewCreateInfo{
                    .format = VK_FORMAT_R32G32B32A32_SFLOAT,
                }).setSampler(driver->device.createSampler(vkh::VkSamplerCreateInfo{
                    .magFilter = VK_FILTER_LINEAR,
                    .minFilter = VK_FILTER_LINEAR,
                    .addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
                    .addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
                    .unnormalizedCoordinates = true,
                }));
            };

            // 
            depthImage = vkt::ImageRegion(std::make_shared<vkt::VmaImageAllocation>(driver->getAllocator(), vkh::VkImageCreateInfo{
                .format = VK_FORMAT_D32_SFLOAT_S8_UINT,
                .extent = {width,height,1u},
                .usage = { .eTransferDst = 1, .eDepthStencilAttachment = 1 },
            }), vkh::VkImageViewCreateInfo{
                .format = VK_FORMAT_D32_SFLOAT_S8_UINT,
                .subresourceRange = { .aspectMask = { .eDepth = 1, .eStencil = 1 } },
            });

            // 5th attachment
            deferredAttachments[8u] = depthImage;
            smpFlip0Attachments[8u] = depthImage;
            smpFlip1Attachments[8u] = depthImage;

            // 
            deferredFramebuffer = driver->getDevice().createFramebuffer(vkh::VkFramebufferCreateInfo{
                .renderPass = renderPass,
                .attachmentCount = deferredAttachments.size(),
                .pAttachments = deferredAttachments.data(),
                .width = width,
                .height = height
            });

            // Reprojection WILL NOT write own depth... 
            smpFlip0Framebuffer = driver->getDevice().createFramebuffer(vkh::VkFramebufferCreateInfo{
                .renderPass = renderPass,
                .attachmentCount = smpFlip0Attachments.size(),
                .pAttachments = smpFlip0Attachments.data(),
                .width = width,
                .height = height
            });

            // Reprojection WILL NOT write own depth... 
            smpFlip1Framebuffer = driver->getDevice().createFramebuffer(vkh::VkFramebufferCreateInfo{
                .renderPass = renderPass,
                .attachmentCount = smpFlip1Attachments.size(),
                .pAttachments = smpFlip1Attachments.data(),
                .width = width,
                .height = height
            });

            // 
            scissor = vk::Rect2D{ vk::Offset2D(0, 0), vk::Extent2D(width, height) };
            viewport = vk::Viewport{ 0.0f, 0.0f, static_cast<float>(scissor.extent.width), static_cast<float>(scissor.extent.height), 0.f, 1.f };

            // 
            thread->submitOnce([&,this](vk::CommandBuffer& cmd) {
                vkt::imageBarrier(cmd, vkt::ImageBarrierInfo{ .image = depthImage, .targetLayout = vk::ImageLayout::eGeneral, .originLayout = vk::ImageLayout::eUndefined, .subresourceRange = depthImage });
                for (uint32_t i = 0u; i < 8u; i++) { // Definitely Not an Hotel
                    vkt::imageBarrier(cmd, vkt::ImageBarrierInfo{ .image = this->frameBfImages[i], .targetLayout = vk::ImageLayout::eGeneral, .originLayout = vk::ImageLayout::eUndefined, .subresourceRange = this->frameBfImages[i] });
                    vkt::imageBarrier(cmd, vkt::ImageBarrierInfo{ .image = this->smFlip0Images[i], .targetLayout = vk::ImageLayout::eGeneral, .originLayout = vk::ImageLayout::eUndefined, .subresourceRange = this->smFlip0Images[i] });
                    vkt::imageBarrier(cmd, vkt::ImageBarrierInfo{ .image = this->smFlip1Images[i], .targetLayout = vk::ImageLayout::eGeneral, .originLayout = vk::ImageLayout::eUndefined, .subresourceRange = this->smFlip1Images[i] });
                    cmd.clearColorImage(this->smFlip1Images[i], vk::ImageLayout::eGeneral, vk::ClearColorValue().setFloat32({ 0.f,0.f,0.f,0.f }), { this->smFlip1Images[i] });
                    cmd.clearColorImage(this->smFlip0Images[i], vk::ImageLayout::eGeneral, vk::ClearColorValue().setFloat32({ 0.f,0.f,0.f,0.f }), { this->smFlip0Images[i] });
                    cmd.clearColorImage(this->frameBfImages[i], vk::ImageLayout::eGeneral, vk::ClearColorValue().setFloat32({ 0.f,0.f,0.f,0.f }), { this->frameBfImages[i] });
                };
            });

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

            // Raw Data
            for (uint32_t b=0u;b<8u;b++) { // For Ray Tracers
                this->meshDataDescriptorSetLayoutHelper.pushBinding(vkh::VkDescriptorSetLayoutBinding{ .binding = b, .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, .descriptorCount =   64u, .stageFlags = { .eVertex = 1, .eGeometry = 1, .eFragment = 1, .eCompute = 1, .eRaygen = 1, .eClosestHit = 1 } },vkh::VkDescriptorBindingFlags{ .ePartiallyBound = 1});
            };

            // IndeX Data
            this->meshDataDescriptorSetLayoutHelper.pushBinding(vkh::VkDescriptorSetLayoutBinding{ .binding = 8u, .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, .descriptorCount = 64u, .stageFlags = { .eVertex = 1, .eGeometry = 1, .eFragment = 1, .eCompute = 1, .eRaygen = 1, .eClosestHit = 1 } }, vkh::VkDescriptorBindingFlags{ .ePartiallyBound = 1 });

            // TODO: Inline Ray Tracing in Compute Shaders (as in DirectX 12.1)
            // TODO: Open Issue for Vulkan API to implement inline Ray-Tracing in Compute Shaders
            // https://github.com/KhronosGroup/Vulkan-Docs/issues/1155
            this->bindingsDescriptorSetLayoutHelper.pushBinding(vkh::VkDescriptorSetLayoutBinding{ .binding = 0u, .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER           , .descriptorCount =   64u, .stageFlags = { .eVertex = 1, .eGeometry = 1, .eFragment = 1, .eCompute = 1, .eRaygen = 1, .eClosestHit = 1, .eMiss = 1 } }, vkh::VkDescriptorBindingFlags{ .ePartiallyBound = 1, .eVariableDescriptorCount = 1 });
            this->bindingsDescriptorSetLayoutHelper.pushBinding(vkh::VkDescriptorSetLayoutBinding{ .binding = 1u, .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER           , .descriptorCount =   64u, .stageFlags = { .eVertex = 1, .eGeometry = 1, .eFragment = 1, .eCompute = 1, .eRaygen = 1, .eClosestHit = 1, .eMiss = 1 } }, vkh::VkDescriptorBindingFlags{ .ePartiallyBound = 1, .eVariableDescriptorCount = 1 });
            this->bindingsDescriptorSetLayoutHelper.pushBinding(vkh::VkDescriptorSetLayoutBinding{ .binding = 2u, .descriptorType = VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_NV, .descriptorCount =    1u, .stageFlags = { .eVertex = 1, .eGeometry = 1, .eFragment = 1, .eCompute = 1, .eRaygen = 1, .eClosestHit = 1, .eMiss = 1 } }, vkh::VkDescriptorBindingFlags{ .ePartiallyBound = 1, .eVariableDescriptorCount = 1 });
            this->bindingsDescriptorSetLayoutHelper.pushBinding(vkh::VkDescriptorSetLayoutBinding{ .binding = 3u, .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER           , .descriptorCount =    1u, .stageFlags = { .eVertex = 1, .eGeometry = 1, .eFragment = 1, .eCompute = 1, .eRaygen = 1, .eClosestHit = 1, .eMiss = 1 } }, vkh::VkDescriptorBindingFlags{ .ePartiallyBound = 1, .eVariableDescriptorCount = 1 });
            this->bindingsDescriptorSetLayoutHelper.pushBinding(vkh::VkDescriptorSetLayoutBinding{ .binding = 4u, .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER           , .descriptorCount =   64u, .stageFlags = { .eVertex = 1, .eGeometry = 1, .eFragment = 1, .eCompute = 1, .eRaygen = 1, .eClosestHit = 1, .eMiss = 1 } }, vkh::VkDescriptorBindingFlags{ .ePartiallyBound = 1, .eVariableDescriptorCount = 1 });
            this->bindingsDescriptorSetLayoutHelper.pushBinding(vkh::VkDescriptorSetLayoutBinding{ .binding = 5u, .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER           , .descriptorCount =    1u, .stageFlags = { .eVertex = 1, .eGeometry = 1, .eFragment = 1, .eCompute = 1, .eRaygen = 1, .eClosestHit = 1, .eMiss = 1 } }, vkh::VkDescriptorBindingFlags{ .ePartiallyBound = 1, .eVariableDescriptorCount = 1 });
            this->bindingsDescriptorSetLayoutHelper.pushBinding(vkh::VkDescriptorSetLayoutBinding{ .binding = 6u, .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER           , .descriptorCount =    1u, .stageFlags = { .eVertex = 1, .eGeometry = 1, .eFragment = 1, .eCompute = 1, .eRaygen = 1, .eClosestHit = 1, .eMiss = 1 } }, vkh::VkDescriptorBindingFlags{ .ePartiallyBound = 1, .eVariableDescriptorCount = 1 }); // Super-Instance
            this->bindingsDescriptorSetLayoutHelper.pushBinding(vkh::VkDescriptorSetLayoutBinding{ .binding = 7u, .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER           , .descriptorCount =   64u, .stageFlags = { .eVertex = 1, .eGeometry = 1, .eFragment = 1, .eCompute = 1, .eRaygen = 1, .eClosestHit = 1, .eMiss = 1 } }, vkh::VkDescriptorBindingFlags{ .ePartiallyBound = 1, .eVariableDescriptorCount = 1 });

            // 
            this->samplingDescriptorSetLayoutHelper.pushBinding(vkh::VkDescriptorSetLayoutBinding{ .binding = 0u, .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE         , .descriptorCount =   8u, .stageFlags = { .eVertex = 1, .eGeometry = 1, .eFragment = 1, .eCompute = 1, .eRaygen = 1, .eClosestHit = 1, .eMiss = 1 } },vkh::VkDescriptorBindingFlags{ .ePartiallyBound = 1, .eVariableDescriptorCount = 1 });
            this->samplingDescriptorSetLayoutHelper.pushBinding(vkh::VkDescriptorSetLayoutBinding{ .binding = 1u, .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER        , .descriptorCount =   8u, .stageFlags = { .eVertex = 1, .eGeometry = 1, .eFragment = 1, .eCompute = 1, .eRaygen = 1, .eClosestHit = 1, .eMiss = 1 } },vkh::VkDescriptorBindingFlags{ .ePartiallyBound = 1, .eVariableDescriptorCount = 1 });

            // 
            this->deferredDescriptorSetLayoutHelper.pushBinding(vkh::VkDescriptorSetLayoutBinding{ .binding = 0u, .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, .descriptorCount =   8u, .stageFlags = { .eVertex = 1, .eGeometry = 1, .eFragment = 1, .eCompute = 1, .eRaygen = 1, .eClosestHit = 1, .eMiss = 1 } },vkh::VkDescriptorBindingFlags{ .ePartiallyBound = 1, .eVariableDescriptorCount = 1 });

            // 
            this->materialDescriptorSetLayoutHelper.pushBinding(vkh::VkDescriptorSetLayoutBinding{ .binding = 0u, .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, .descriptorCount = 128u, .stageFlags = { .eVertex = 1, .eGeometry = 1, .eFragment = 1, .eCompute = 1, .eRaygen = 1, .eClosestHit = 1, .eMiss = 1 } },vkh::VkDescriptorBindingFlags{ .ePartiallyBound = 1, .eVariableDescriptorCount = 1 });
            this->materialDescriptorSetLayoutHelper.pushBinding(vkh::VkDescriptorSetLayoutBinding{ .binding = 1u, .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER        , .descriptorCount =   8u, .stageFlags = { .eVertex = 1, .eGeometry = 1, .eFragment = 1, .eCompute = 1, .eRaygen = 1, .eClosestHit = 1, .eMiss = 1 } },vkh::VkDescriptorBindingFlags{ .ePartiallyBound = 1, .eVariableDescriptorCount = 1 });

            // 
            this->materialDescriptorSetLayout = driver->getDevice().createDescriptorSetLayout(materialDescriptorSetLayoutHelper);
            this->deferredDescriptorSetLayout = driver->getDevice().createDescriptorSetLayout(deferredDescriptorSetLayoutHelper);
            this->samplingDescriptorSetLayout = driver->getDevice().createDescriptorSetLayout(samplingDescriptorSetLayoutHelper);
            this->bindingsDescriptorSetLayout = driver->getDevice().createDescriptorSetLayout(bindingsDescriptorSetLayoutHelper);
            this->meshDataDescriptorSetLayout = driver->getDevice().createDescriptorSetLayout(meshDataDescriptorSetLayoutHelper);

            // 
            std::vector<vkh::VkPushConstantRange> ranges = { {.stageFlags = { .eVertex = 1, .eGeometry = 1, .eFragment = 1, .eRaygen = 1, .eClosestHit = 1, .eMiss = 1 }, .offset = 0u, .size = 16u } };
            std::vector<VkDescriptorSetLayout> layouts = { meshDataDescriptorSetLayout, bindingsDescriptorSetLayout, deferredDescriptorSetLayout, samplingDescriptorSetLayout, materialDescriptorSetLayout };
            this->unifiedPipelineLayout = driver->getDevice().createPipelineLayout(vkh::VkPipelineLayoutCreateInfo{}.setSetLayouts(layouts).setPushConstantRanges(ranges));
            return shared_from_this();
        };

        // 
        std::shared_ptr<Context> createDescriptorSets() {
            if (!this->unifiedPipelineLayout) { this->createDescriptorSetLayouts(); };

            {
                // 
                vkh::VsDescriptorSetCreateInfoHelper descInfo(deferredDescriptorSetLayout, thread->getDescriptorPool());
                vkh::VsDescriptorHandle<VkDescriptorImageInfo> handle = descInfo.pushDescription(vkh::VkDescriptorUpdateTemplateEntry{
                    .dstBinding = 0u,
                    .descriptorCount = 8u,
                    .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER
                });

                // 
                for (uint32_t i = 0; i < 8u; i++) {
                    handle.offset<VkDescriptorImageInfo>(i) = frameBfImages[i];
                };

                // 
                this->driver->getDevice().updateDescriptorSets(vkt::vector_cast<vk::WriteDescriptorSet,vkh::VkWriteDescriptorSet>(descInfo.setDescriptorSet(
                    this->deferredDescriptorSet = driver->getDevice().allocateDescriptorSets(descInfo)[0]
                )),{});
            };

            { // For Reprojection Pipeline
                vkh::VsDescriptorSetCreateInfoHelper descInfo(samplingDescriptorSetLayout, thread->getDescriptorPool());
                vkh::VsDescriptorHandle<VkDescriptorImageInfo> handle = descInfo.pushDescription(vkh::VkDescriptorUpdateTemplateEntry{
                    .dstBinding = 0u,
                    .descriptorCount = 8u,
                    .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE
                });

                // 
                for (uint32_t i = 0; i < 8u; i++) {
                    handle.offset<VkDescriptorImageInfo>(i) = smFlip0Images[i];
                };

                // Reprojection WILL NOT write own depth... 
                this->driver->getDevice().updateDescriptorSets(vkt::vector_cast<vk::WriteDescriptorSet,vkh::VkWriteDescriptorSet>(descInfo.setDescriptorSet(
                    this->smpFlip0DescriptorSet = driver->getDevice().allocateDescriptorSets(descInfo)[0]
                )),{});
            };

            { // For Reprojection Pipeline
                vkh::VsDescriptorSetCreateInfoHelper descInfo(samplingDescriptorSetLayout, thread->getDescriptorPool());
                vkh::VsDescriptorHandle<VkDescriptorImageInfo> handle = descInfo.pushDescription(vkh::VkDescriptorUpdateTemplateEntry{
                    .dstBinding = 0u,
                    .descriptorCount = 8u,
                    .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE
                });

                // 
                for (uint32_t i = 0; i < 8u; i++) {
                    handle.offset<VkDescriptorImageInfo>(i) = smFlip1Images[i];
                };

                // Reprojection WILL NOT write own depth... 
                this->driver->getDevice().updateDescriptorSets(vkt::vector_cast<vk::WriteDescriptorSet, vkh::VkWriteDescriptorSet>(descInfo.setDescriptorSet(
                    this->smpFlip1DescriptorSet = driver->getDevice().allocateDescriptorSets(descInfo)[0]
                )), {});
            };

            // 
            this->descriptorSets[2] = this->deferredDescriptorSet;
            this->descriptorSets[3] = this->smpFlip0DescriptorSet;

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
        std::chrono::time_point<std::chrono::steady_clock> beginTime = std::chrono::high_resolution_clock::now();
        std::chrono::time_point<std::chrono::steady_clock> leastTime = std::chrono::high_resolution_clock::now();
        std::chrono::time_point<std::chrono::steady_clock> previTime = std::chrono::high_resolution_clock::now();

        // 
        vk::Rect2D scissor = {};
        vk::Viewport viewport = {};
        vk::RenderPass renderPass = {};
        vk::Framebuffer smpFlip0Framebuffer = {};
        vk::Framebuffer smpFlip1Framebuffer = {};
        vk::Framebuffer deferredFramebuffer = {};

        // 
        vkt::Vector<Matrices> uniformGPUData = {};
        vkt::Vector<Matrices> uniformRawData = {};
        //Matrices uniformData = {};

        // Image Buffers
        std::array<vkt::ImageRegion, 8u> smFlip0Images = {};
        std::array<vkt::ImageRegion, 8u> smFlip1Images = {}; // Path Tracing
        std::array<vkt::ImageRegion, 8u> frameBfImages = {}; // Rasterization
        std::array<vk::DescriptorSet, 5u> descriptorSets = {};
        vkt::ImageRegion depthImage = {};

        // 
        vk::DescriptorSet deferredDescriptorSet = {};
        vk::DescriptorSet smpFlip0DescriptorSet = {};
        vk::DescriptorSet smpFlip1DescriptorSet = {};
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

        // 
        //glm::mat4 projected = glm::mat4(1.f);
        //glm::mat4 modelview = glm::mat4(1.f);
    };

};
