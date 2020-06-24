#pragma once // #

#include <array>
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
        glm::mat3x4 modelviewPrev = glm::mat3x4(1.f);
        glm::mat3x4 modelviewPrevInv = glm::mat3x4(1.f);
        glm::uvec4 mdata = glm::uvec4(0u);                         // mesh mutation or modification data
        //glm::uvec2 tdata = glm::uvec2(0u), rdata = glm::uvec2(0u); // first for time, second for randoms
        glm::uvec2 tdata = glm::uvec2(0u);
        glm::uvec2 rdata = glm::uvec2(0u);
    };
#pragma pack(pop)

    // 
    class Context : public std::enable_shared_from_this<Context> { public: friend MeshInput; friend MeshBinding; friend Node; friend Driver; friend Material; friend Renderer;
        Context() {};
        Context(const vkt::uni_ptr<Driver>& driver) : driver(driver) { this->construct(); };
        Context(const std::shared_ptr<Driver>& driver) : driver(driver) { this->construct(); };
        ~Context() {};

        // 
        public: virtual vkt::uni_ptr<Context> sharedPtr() { return shared_from_this(); };
        //virtual vkt::uni_ptr<Context> sharedPtr() const { return shared_from_this(); };

        // 
        protected: virtual uPTR(Context) construct() {//
            vkh::VkBufferCreateFlags bflgs = {};
            vkt::unlock32(bflgs) = 0u;

            auto hostUsage = vkh::VkBufferUsageFlags{.eTransferSrc = 1, .eTransferDst = 1, .eUniformBuffer = 1, .eStorageBuffer = 1, .eRayTracing = 1 };
            auto gpuUsage  = vkh::VkBufferUsageFlags{.eTransferSrc = 1,                    .eUniformBuffer = 1, .eStorageBuffer = 1, .eRayTracing = 1 };

            this->thread = std::make_shared<Thread>(this->driver);
            this->uniformGPUData = vkt::Vector<jvi::Matrices>(std::make_shared<vkt::VmaBufferAllocation>(this->driver->getAllocator(), vkh::VkBufferCreateInfo{ .flags = bflgs, .size = VkDeviceSize(sizeof(Matrices) * 2u), .usage = hostUsage }, vkt::VmaMemoryInfo{ .memUsage = VMA_MEMORY_USAGE_GPU_ONLY }));
            this->uniformRawData = vkt::Vector<jvi::Matrices>(std::make_shared<vkt::VmaBufferAllocation>(this->driver->getAllocator(), vkh::VkBufferCreateInfo{ .flags = bflgs, .size = VkDeviceSize(sizeof(Matrices) * 2u), .usage = gpuUsage }, vkt::VmaMemoryInfo{ .memUsage = VMA_MEMORY_USAGE_CPU_TO_GPU }));
            this->beginTime = std::chrono::high_resolution_clock::now();
            this->leastTime = std::chrono::high_resolution_clock::now();
            this->previTime = std::chrono::high_resolution_clock::now();

            // 
            glm::mat4x4 projected = glm::perspective(80.f / 180.f * glm::pi<float>(), float(1600) / float(1200), 0.001f, 10000.f);
            glm::mat4x4 modelview = glm::lookAt(glm::vec3(0.f, 0.f, 0.f), glm::vec3(0.f, 0.f, 1.f), glm::vec3(0.f, 1.f, 0.f)); // Defaultly - Center Of World into Z coordinate

            //
            uniformRawData[0].modelview = glm::transpose(modelview);
            uniformRawData[0].modelviewInv = glm::transpose(glm::inverse(modelview));
            uniformRawData[0].projection = glm::transpose(projected);
            uniformRawData[0].projectionInv = glm::transpose(glm::inverse(projected));
            return uTHIS;
        };

        // 
        public: virtual uPTR(Context) setThread(const vkt::uni_ptr<Thread>& thread) {
            this->thread = thread;
            return uTHIS;
        };

        public: virtual vkt::uni_ptr<Thread>& getThread() {
            return this->thread;
        };

        public: virtual const vkt::uni_ptr<Thread>& getThread() const {
            return this->thread;
        };

        // 
        public: virtual VkRect2D& refScissor() { return scissor; };
        public: virtual VkViewport& refViewport() { return viewport; };
        public: virtual VkRenderPass& refRenderPass() { return renderPass; };
        //VkFramebuffer& refFramebuffer() { return framebuffer; };

        // 
        public: virtual const VkRect2D& refScissor() const { return scissor; };
        public: virtual const VkViewport& refViewport() const { return viewport; };
        public: virtual const VkRenderPass& refRenderPass() const { return renderPass; };
        //const VkFramebuffer& refFramebuffer() const { return framebuffer; };

        // 
        public: virtual vkt::uni_ptr<Driver>& getDriver() { return driver; };
        public: virtual const vkt::uni_ptr<Driver>& getDriver() const { return driver; };

        //
        protected: virtual uPTR(Context) createRenderPass() { // 
            std::cout << "Create Render Pass" << std::endl; // DEBUG!!

            //
            auto colAttachment = vkh::VkAttachmentDescription{
                .format = VK_FORMAT_R32G32B32A32_SFLOAT,
                .loadOp = VK_ATTACHMENT_LOAD_OP_LOAD,
                .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
                .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
                .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
                .initialLayout = VK_IMAGE_LAYOUT_GENERAL,
                .finalLayout = VK_IMAGE_LAYOUT_GENERAL,
            };

            //
            auto depAttachment = vkh::VkAttachmentDescription{
                .format = VK_FORMAT_D32_SFLOAT_S8_UINT,
                .loadOp = VK_ATTACHMENT_LOAD_OP_LOAD,
                .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
                .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
                .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
                .initialLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
                .finalLayout = VK_IMAGE_LAYOUT_GENERAL
            };

            //
            auto dep0 = vkh::VkSubpassDependency{ .srcSubpass = VK_SUBPASS_EXTERNAL, .dstSubpass = 0u };
            auto dep1 = vkh::VkSubpassDependency{ .srcSubpass = 0u, .dstSubpass = VK_SUBPASS_EXTERNAL };

            { // Fixing Linux Issue?
                auto srcStageMask = vkh::VkPipelineStageFlags{.eColorAttachmentOutput = 1, .eTransfer = 1, .eBottomOfPipe = 1}; ASSIGN(dep0, srcStageMask);
                auto dstStageMask = vkh::VkPipelineStageFlags{.eColorAttachmentOutput = 1};                                     ASSIGN(dep0, dstStageMask);
                auto srcAccessMask = vkh::VkAccessFlags{.eColorAttachmentWrite = 1};                                            ASSIGN(dep0, srcAccessMask);
                auto dstAccessMask = vkh::VkAccessFlags{.eColorAttachmentRead = 1, .eColorAttachmentWrite = 1};                 ASSIGN(dep0, dstAccessMask);
                auto dependencyFlags = vkh::VkDependencyFlags{.eByRegion = 1};                                                  ASSIGN(dep0, dependencyFlags);
            }

            { // Fixing Linux Issue?
                auto srcStageMask = vkh::VkPipelineStageFlags{.eColorAttachmentOutput = 1};                                     ASSIGN(dep1, srcStageMask);
                auto dstStageMask = vkh::VkPipelineStageFlags{.eTopOfPipe = 1, .eColorAttachmentOutput = 1, .eTransfer = 1};    ASSIGN(dep1, dstStageMask);
                auto srcAccessMask = vkh::VkAccessFlags{.eColorAttachmentRead = 1, .eColorAttachmentWrite = 1};                 ASSIGN(dep1, srcAccessMask);
                auto dstAccessMask = vkh::VkAccessFlags{.eColorAttachmentRead = 1, .eColorAttachmentWrite = 1};                 ASSIGN(dep1, dstAccessMask);
                auto dependencyFlags = vkh::VkDependencyFlags{.eByRegion = 1};                                                  ASSIGN(dep1, dependencyFlags);
            }

            {
                vkh::VsRenderPassCreateInfoHelper rpsInfo = {};
                for (uint32_t b = 0u; b < 8u; b++) { rpsInfo.addColorAttachment(colAttachment); };
                rpsInfo.setDepthStencilAttachment(depAttachment);
                rpsInfo.addSubpassDependency(dep0);
                rpsInfo.addSubpassDependency(dep1);
                vkh::handleVk(this->driver->getDeviceDispatch()->CreateRenderPass(rpsInfo, nullptr, &this->renderPass));
            }

            {
                vkh::VsRenderPassCreateInfoHelper rpsInfo = {};
                for (uint32_t b = 0u; b < 1u; b++) { rpsInfo.addColorAttachment(colAttachment); };
                rpsInfo.setDepthStencilAttachment(depAttachment);
                rpsInfo.addSubpassDependency(dep0);
                rpsInfo.addSubpassDependency(dep1);
                vkh::handleVk(this->driver->getDeviceDispatch()->CreateRenderPass(rpsInfo, nullptr, &this->mapRenderPass));
            }

            // 
            return uTHIS;
        };

        // 
        public: virtual std::vector<VkDescriptorSet>& getDescriptorSets() {
            return descriptorSets;
        };

        // 
        public: virtual VkPipelineLayout getPipelineLayout() {
            return unifiedPipelineLayout;
        };

        // 
        public: virtual uPTR(Context) registerTime() {
            this->previTime = this->leastTime;
            uniformRawData[0].tdata[0] = std::chrono::duration_cast<std::chrono::milliseconds>((this->leastTime = std::chrono::high_resolution_clock::now()) - this->beginTime).count(); // time from beginning
            uniformRawData[0].tdata[1] = std::chrono::duration_cast<std::chrono::milliseconds>(this->leastTime - this->previTime).count(); // difference 
            return uTHIS;
        };

        // 
        public: virtual uint32_t& drawTime() { return uniformRawData[0].tdata[0u]; };
        public: virtual uint32_t& timeDiff() { return uniformRawData[0].tdata[1u]; };
        public: virtual const uint32_t& drawTime() const { return uniformRawData[0].tdata[0u]; };
        public: virtual const uint32_t& timeDiff() const { return uniformRawData[0].tdata[1u]; };

        // 
        public: virtual uPTR(Context) setDrawCount(const uint32_t& count = 0u) {
            uniformRawData[0].rdata[0] = count;
            return uTHIS;
        };

        // Updated! 
        public: virtual uPTR(Context) setPerspective(vkt::uni_arg<glm::mat4x4> persp = glm::mat4x4(1.f)) {
            uniformRawData[0].projection = glm::transpose(*persp);
            uniformRawData[0].projectionInv = glm::transpose(glm::inverse(*persp));
            changedPerspective = true;
            return uTHIS;
        };

        // Updated! 
        public: virtual uPTR(Context) setModelView(vkt::uni_arg<glm::mat4x4> mv = glm::mat4x4(1.f)) {
            uniformRawData[0].modelview = glm::transpose(*mv);
            uniformRawData[0].modelviewInv = glm::transpose(glm::inverse(*mv));
            return uTHIS;
        };


        // 
        public: vkt::ImageRegion& getFrameBuffer(const uint32_t& I = 0u) { return this->frameBfImages[I]; };
        public: const vkt::ImageRegion& getFrameBuffer(const uint32_t& I = 0u) const { return this->frameBfImages[I]; };

        // 
        public: vkt::ImageRegion& getFlip0Buffer(const uint32_t& I = 0u) { return this->smFlip0Images[I]; };
        public: const vkt::ImageRegion& getFlip0Buffer(const uint32_t& I = 0u) const { return this->smFlip0Images[I]; };

        // 
        public: vkt::ImageRegion& getFlip1Buffer(const uint32_t& I = 0u) { return this->smFlip1Images[I]; };
        public: const vkt::ImageRegion& getFlip1Buffer(const uint32_t& I = 0u) const { return this->smFlip1Images[I]; };


        // 
        public: virtual std::array<vkt::ImageRegion, 12u>& getFlip0Buffers() {
            return this->smFlip0Images;
        }

        // 
        public: virtual const std::array<vkt::ImageRegion, 12u>& getFlip0Buffers() const {
            return this->smFlip0Images;
        }

        // 
        public: virtual std::array<vkt::ImageRegion, 12u>& getFlip1Buffers() {
            return this->smFlip1Images;
        }

        // 
        public: virtual const std::array<vkt::ImageRegion, 12u>& getFlip1Buffers() const {
            return this->smFlip1Images;
        }

        // 
        public: virtual std::array<vkt::ImageRegion, 12u>& getFrameBuffers() {
            return this->frameBfImages;
        }

        // 
        public: virtual const std::array<vkt::ImageRegion, 12u>& getFrameBuffers() const {
            return this->frameBfImages;
        }

        // 
        protected: virtual uPTR(Context) createFramebuffers(const uint32_t& width = 1600u, const uint32_t& height = 1200u) { // 
            std::cout << "Create Frame Buffer" << std::endl; // DEBUG!!

            //
            vkh::VkBufferCreateFlags bflg = {};
            vkt::unlock32(bflg) = 0u;

            // Used Native Perspective
            if (!changedPerspective) {
                glm::mat4x4 projected = glm::perspective(80.f / 180.f * glm::pi<float>(), float(width) / float(height), 0.001f, 10000.f);
                uniformRawData[0].projection = glm::transpose(projected);
                uniformRawData[0].projectionInv = glm::transpose(glm::inverse(projected));
            };

            // 
            std::array<VkImageView, 9u> deferredAttachments = {};
            std::array<VkImageView, 9u> smpFlip0Attachments = {};
            std::array<VkImageView, 9u> smpFlip1Attachments = {};
            std::array<VkImageView, 9u> rasteredAttachments = {};

            //
            auto fbusage = vkh::VkImageUsageFlags{.eTransferDst = 1, .eSampled = 1, .eStorage = 1, .eColorAttachment = 1 };
            auto aspect = vkh::VkImageAspectFlags{.eColor = 1};
            auto apres = vkh::VkImageSubresourceRange{.aspectMask = aspect};
            auto flagsU32 = 0u;
            auto& flags = reinterpret_cast<vkh::VkImageCreateFlags&>(flagsU32);
            auto& allocInfo = driver->memoryAllocationInfo();

            // 
            for (uint32_t b = 0u; b < 12u; b++) { // 
                this->frameBfImages[b] = vkt::ImageRegion(std::make_shared<vkt::ImageAllocation>(vkh::VkImageCreateInfo{
                    .flags = flags,
                    .format = VK_FORMAT_R32G32B32A32_SFLOAT,
                    .extent = {width,height,1u},
                    .usage = fbusage,
                }, allocInfo), vkh::VkImageViewCreateInfo{
                    .format = VK_FORMAT_R32G32B32A32_SFLOAT,
                    .subresourceRange = apres
                });

                // Create Sampler By Reference
                vkh::handleVk(this->driver->getDeviceDispatch()->CreateSampler(vkh::VkSamplerCreateInfo{
                    .magFilter = VK_FILTER_LINEAR,
                    .minFilter = VK_FILTER_LINEAR,
                    .addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
                    .addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
                    .unnormalizedCoordinates = true,
                }, nullptr, &this->frameBfImages[b].refSampler()));

                if (b < 8u) { deferredAttachments[b] = frameBfImages[b]; };
            };

            // 
            for (uint32_t b=0u;b<12u;b++) { { // 
                this->smFlip0Images[b] = vkt::ImageRegion(std::make_shared<vkt::ImageAllocation>(vkh::VkImageCreateInfo{
                    .flags = flags,
                    .format = VK_FORMAT_R32G32B32A32_SFLOAT,
                    .extent = {width,height,1u},
                    .usage = fbusage,
                }, allocInfo), vkh::VkImageViewCreateInfo{
                    .format = VK_FORMAT_R32G32B32A32_SFLOAT,
                    .subresourceRange = apres
                });

                // Create Sampler By Reference
                vkh::handleVk(this->driver->getDeviceDispatch()->CreateSampler(vkh::VkSamplerCreateInfo{
                    .magFilter = VK_FILTER_LINEAR,
                    .minFilter = VK_FILTER_LINEAR,
                    .addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
                    .addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
                    .unnormalizedCoordinates = true,
                }, nullptr, &this->smFlip0Images[b].refSampler()));

                if (b < 8u) { smpFlip0Attachments[b] = smFlip0Images[b]; };
            };

            // 
            {
                this->smFlip1Images[b] = vkt::ImageRegion(std::make_shared<vkt::ImageAllocation>(vkh::VkImageCreateInfo{
                    .flags = flags,
                    .format = VK_FORMAT_R32G32B32A32_SFLOAT,
                    .extent = {width,height,1u},
                    .usage = fbusage,
                }, allocInfo), vkh::VkImageViewCreateInfo{
                    .format = VK_FORMAT_R32G32B32A32_SFLOAT,
                    .subresourceRange = apres
                });

                // Create Sampler By Reference
                vkh::handleVk(this->driver->getDeviceDispatch()->CreateSampler(vkh::VkSamplerCreateInfo{
                    .magFilter = VK_FILTER_LINEAR,
                    .minFilter = VK_FILTER_LINEAR,
                    .addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
                    .addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
                    .unnormalizedCoordinates = true,
                }, nullptr, &this->smFlip1Images[b].refSampler()));

                if (b < 8u) { smpFlip1Attachments[b] = smFlip1Images[b]; };
            }; };

            // 
            for (uint32_t b = 0u; b < 8u; b++) { // 
                this->rastersImages[b] = vkt::ImageRegion(std::make_shared<vkt::ImageAllocation>(vkh::VkImageCreateInfo{
                    .flags = flags,
                    .format = VK_FORMAT_R32G32B32A32_SFLOAT,
                    .extent = {width,height,1u},
                    .usage = fbusage,
                }, allocInfo), vkh::VkImageViewCreateInfo{
                    .format = VK_FORMAT_R32G32B32A32_SFLOAT,
                    .subresourceRange = apres
                });

                // Create Sampler By Reference
                vkh::handleVk(this->driver->getDeviceDispatch()->CreateSampler(vkh::VkSamplerCreateInfo{
                    .magFilter = VK_FILTER_LINEAR,
                    .minFilter = VK_FILTER_LINEAR,
                    .addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
                    .addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
                    .unnormalizedCoordinates = true,
                }, nullptr, &this->rastersImages[b].refSampler()));

                if (b < 8u) { rasteredAttachments[b] = rastersImages[b]; };
            };

            // 
            {
                //
                auto aspect = vkh::VkImageAspectFlags{.eDepth = 1, .eStencil = 1};
                auto dpuse = vkh::VkImageUsageFlags{.eTransferDst = 1, .eSampled = 1, .eDepthStencilAttachment = 1 };
                auto dpres = vkh::VkImageSubresourceRange{.aspectMask = aspect};
                dpuse = dpuse | VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;

                this->depthImage = vkt::ImageRegion(std::make_shared<vkt::ImageAllocation>(vkh::VkImageCreateInfo{
                    .flags = flags,
                    .format = VK_FORMAT_D32_SFLOAT_S8_UINT,
                    .extent = {width,height,1u},
                    .usage = dpuse,
                }, allocInfo), vkh::VkImageViewCreateInfo{
                    .format = VK_FORMAT_D32_SFLOAT_S8_UINT,
                    .subresourceRange = dpres,
                }, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);

                vkh::handleVk(this->driver->getDeviceDispatch()->CreateSampler(vkh::VkSamplerCreateInfo{
                    .magFilter = VK_FILTER_LINEAR,
                    .minFilter = VK_FILTER_LINEAR,
                    .addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
                    .addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
                    .unnormalizedCoordinates = true,
                }, nullptr, &this->depthImage.refSampler()));

                // 5th attachment
                deferredAttachments[8u] = depthImage;
                smpFlip0Attachments[8u] = depthImage;
                smpFlip1Attachments[8u] = depthImage;
                rasteredAttachments[8u] = depthImage;
            };

            // 
            vkh::handleVk(this->driver->getDeviceDispatch()->CreateFramebuffer(vkh::VkFramebufferCreateInfo{
                .renderPass = renderPass,
                .attachmentCount = static_cast<uint32_t>(deferredAttachments.size()),
                .pAttachments = deferredAttachments.data(),
                .width = width,
                .height = height
            }, nullptr, &this->deferredFramebuffer));

            // Reprojection WILL NOT write own depth... 
            vkh::handleVk(this->driver->getDeviceDispatch()->CreateFramebuffer(vkh::VkFramebufferCreateInfo{
                .renderPass = renderPass,
                .attachmentCount = static_cast<uint32_t>(smpFlip0Attachments.size()),
                .pAttachments = smpFlip0Attachments.data(),
                .width = width,
                .height = height
            }, nullptr, &this->smpFlip0Framebuffer));

            // Reprojection WILL NOT write own depth... 
            vkh::handleVk(this->driver->getDeviceDispatch()->CreateFramebuffer(vkh::VkFramebufferCreateInfo{
                .renderPass = renderPass,
                .attachmentCount = static_cast<uint32_t>(smpFlip1Attachments.size()),
                .pAttachments = smpFlip1Attachments.data(),
                .width = width,
                .height = height
            }, nullptr, &this->smpFlip1Framebuffer));

            // 
            vkh::handleVk(this->driver->getDeviceDispatch()->CreateFramebuffer(vkh::VkFramebufferCreateInfo{
                .renderPass = renderPass,
                .attachmentCount = static_cast<uint32_t>(rasteredAttachments.size()),
                .pAttachments = rasteredAttachments.data(),
                .width = width,
                .height = height
            }, nullptr, &this->rasteredFramebuffer));

            // 
            scissor = vkh::VkRect2D{ vkh::VkOffset2D{0, 0}, vkh::VkExtent2D{width, height} };
            viewport = vkh::VkViewport{ 0.0f, 0.0f, static_cast<float>(scissor.extent.width), -static_cast<float>(scissor.extent.height), 0.f, 1.f };

            // 
            thread->submitOnce([&,this](VkCommandBuffer& cmd) {
                this->driver->getDeviceDispatch()->CmdClearDepthStencilImage(cmd, this->depthImage.transfer(cmd), this->depthImage.getImageLayout(), vkh::VkClearDepthStencilValue{ .depth = 1.0f, .stencil = 0 }, 1u, depthImage.getImageSubresourceRange());
                for (uint32_t i = 0u; i < 12u; i++) { // Definitely Not an Hotel
                    this->driver->getDeviceDispatch()->CmdClearColorImage(cmd, this->smFlip1Images[i].transfer(cmd), this->smFlip1Images[i].getImageLayout(), vkh::VkClearColorValue{ .float32 = { 0.f,0.f,0.f,0.f } }, 1u, this->smFlip1Images[i].getImageSubresourceRange());
                    this->driver->getDeviceDispatch()->CmdClearColorImage(cmd, this->smFlip0Images[i].transfer(cmd), this->smFlip0Images[i].getImageLayout(), vkh::VkClearColorValue{ .float32 = { 0.f,0.f,0.f,0.f } }, 1u, this->smFlip0Images[i].getImageSubresourceRange());
                    this->driver->getDeviceDispatch()->CmdClearColorImage(cmd, this->frameBfImages[i].transfer(cmd), this->frameBfImages[i].getImageLayout(), vkh::VkClearColorValue{ .float32 = { 0.f,0.f,0.f,0.f } }, 1u, this->frameBfImages[i].getImageSubresourceRange());

                    if (i < 8u) {
                        this->driver->getDeviceDispatch()->CmdClearColorImage(cmd, this->rastersImages[i].transfer(cmd), this->rastersImages[i].getImageLayout(), vkh::VkClearColorValue{ .float32 = { 0.f,0.f,0.f,0.f } }, 1u, this->rastersImages[i].getImageSubresourceRange());
                    };
                };
            });

            // 
            return uTHIS;
        };

        // 
        protected: virtual uPTR(Context) createDescriptorSetLayouts() { // reset layout descriptions
            if (!this->unifiedPipelineLayout) {
                std::cout << "Create Descriptor Set Layouts" << std::endl; // DEBUG!!

                auto templ = vkh::VkDescriptorSetLayoutCreateInfo{};
                vkt::unlock32(templ.flags) = 0u;

                this->meshDataDescriptorSetLayoutHelper = templ;
                this->bindingsDescriptorSetLayoutHelper = templ;
                this->samplingDescriptorSetLayoutHelper = templ;
                this->deferredDescriptorSetLayoutHelper = templ;
                this->materialDescriptorSetLayoutHelper = templ;

                auto pipusage = vkh::VkShaderStageFlags{.eVertex = 1, .eGeometry = 1, .eFragment = 1, .eCompute = 1, .eRaygen = 1, .eClosestHit = 1, .eMiss = 1};
                auto indexedf = vkh::VkDescriptorBindingFlags{ .ePartiallyBound = 1, .eVariableDescriptorCount = 1 };

                // Raw Data
                for (uint32_t b = 0u; b < 2u; b++) { // For Ray Tracers
                    this->meshDataDescriptorSetLayoutHelper.pushBinding(vkh::VkDescriptorSetLayoutBinding{ .binding = b, .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, .descriptorCount = 64u, .stageFlags = pipusage }, indexedf);
                };

                // Data Buffer
                this->meshDataDescriptorSetLayoutHelper.pushBinding(vkh::VkDescriptorSetLayoutBinding{ .binding = 2u, .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, .descriptorCount = 1u, .stageFlags = pipusage }, indexedf);

                // R32UI Map Buffer
                this->meshDataDescriptorSetLayoutHelper.pushBinding(vkh::VkDescriptorSetLayoutBinding{ .binding = 3u, .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, .descriptorCount = 1u, .stageFlags = pipusage }, indexedf);

                // RGBA32F and depth buffer
                this->meshDataDescriptorSetLayoutHelper.pushBinding(vkh::VkDescriptorSetLayoutBinding{ .binding = 4u, .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, .descriptorCount = 2u, .stageFlags = pipusage }, indexedf);

                // BETA: Ray Query Requirements
                this->bindingsDescriptorSetLayoutHelper.pushBinding(vkh::VkDescriptorSetLayoutBinding{ .binding = 5u, .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER           , .descriptorCount = 64u, .stageFlags = pipusage }, indexedf);
                this->bindingsDescriptorSetLayoutHelper.pushBinding(vkh::VkDescriptorSetLayoutBinding{ .binding = 6u, .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER           , .descriptorCount = 64u, .stageFlags = pipusage }, indexedf);
                this->bindingsDescriptorSetLayoutHelper.pushBinding(vkh::VkDescriptorSetLayoutBinding{ .binding = 7u, .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER           , .descriptorCount = 64u, .stageFlags = pipusage }, indexedf);
                this->bindingsDescriptorSetLayoutHelper.pushBinding(vkh::VkDescriptorSetLayoutBinding{ .binding = 8u, .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER           , .descriptorCount = 64u, .stageFlags = pipusage }, indexedf);

                //
                this->bindingsDescriptorSetLayoutHelper.pushBinding(vkh::VkDescriptorSetLayoutBinding{ .binding = 9u, .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER             , .descriptorCount = 1u, .stageFlags = pipusage }, indexedf);
                this->bindingsDescriptorSetLayoutHelper.pushBinding(vkh::VkDescriptorSetLayoutBinding{ .binding = 10u, .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER            , .descriptorCount = 1u, .stageFlags = pipusage }, indexedf);
                this->bindingsDescriptorSetLayoutHelper.pushBinding(vkh::VkDescriptorSetLayoutBinding{ .binding = 11u, .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER            , .descriptorCount = 1u, .stageFlags = pipusage }, indexedf);
                this->bindingsDescriptorSetLayoutHelper.pushBinding(vkh::VkDescriptorSetLayoutBinding{ .binding = 12u, .descriptorType = VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR, .descriptorCount = 1u, .stageFlags = pipusage }, indexedf);


                // 
                this->deferredDescriptorSetLayoutHelper.pushBinding(vkh::VkDescriptorSetLayoutBinding{ .binding = 13u, .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, .descriptorCount = 12u, .stageFlags = pipusage }, indexedf);
                this->deferredDescriptorSetLayoutHelper.pushBinding(vkh::VkDescriptorSetLayoutBinding{ .binding = 14u, .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, .descriptorCount = 12u, .stageFlags = pipusage }, indexedf);
                this->deferredDescriptorSetLayoutHelper.pushBinding(vkh::VkDescriptorSetLayoutBinding{ .binding = 15u, .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, .descriptorCount = 8u , .stageFlags = pipusage }, indexedf);

                //
                this->samplingDescriptorSetLayoutHelper.pushBinding(vkh::VkDescriptorSetLayoutBinding{ .binding = 16u, .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE        , .descriptorCount = 12u, .stageFlags = pipusage }, indexedf);
                this->samplingDescriptorSetLayoutHelper.pushBinding(vkh::VkDescriptorSetLayoutBinding{ .binding = 17u, .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE        , .descriptorCount = 12u, .stageFlags = pipusage }, indexedf);
                this->samplingDescriptorSetLayoutHelper.pushBinding(vkh::VkDescriptorSetLayoutBinding{ .binding = 18u, .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE        , .descriptorCount = 12u, .stageFlags = pipusage }, indexedf);

                //
                this->materialDescriptorSetLayoutHelper.pushBinding(vkh::VkDescriptorSetLayoutBinding{ .binding = 20u, .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER        , .descriptorCount = 8u, .stageFlags = pipusage }, indexedf);
                this->materialDescriptorSetLayoutHelper.pushBinding(vkh::VkDescriptorSetLayoutBinding{ .binding = 21u, .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, .descriptorCount = 1u, .stageFlags = pipusage }, indexedf);
                this->materialDescriptorSetLayoutHelper.pushBinding(vkh::VkDescriptorSetLayoutBinding{ .binding = 22u, .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, .descriptorCount = 128u, .stageFlags = pipusage }, indexedf);

                // 
                vkh::handleVk(this->driver->getDeviceDispatch()->CreateDescriptorSetLayout(this->meshDataDescriptorSetLayoutHelper, nullptr, &this->meshDataDescriptorSetLayout));
                vkh::handleVk(this->driver->getDeviceDispatch()->CreateDescriptorSetLayout(this->bindingsDescriptorSetLayoutHelper, nullptr, &this->bindingsDescriptorSetLayout));
                vkh::handleVk(this->driver->getDeviceDispatch()->CreateDescriptorSetLayout(this->deferredDescriptorSetLayoutHelper, nullptr, &this->deferredDescriptorSetLayout));
                vkh::handleVk(this->driver->getDeviceDispatch()->CreateDescriptorSetLayout(this->samplingDescriptorSetLayoutHelper, nullptr, &this->samplingDescriptorSetLayout));
                vkh::handleVk(this->driver->getDeviceDispatch()->CreateDescriptorSetLayout(this->materialDescriptorSetLayoutHelper, nullptr, &this->materialDescriptorSetLayout));

                //
                const auto pstage = vkh::VkShaderStageFlags{.eVertex = 1, .eGeometry = 1, .eFragment = 1, .eCompute = 1, .eRaygen = 1, .eClosestHit = 1, .eMiss = 1 };
                std::vector<vkh::VkPushConstantRange> ranges = { vkh::VkPushConstantRange{.stageFlags = pstage, .offset = 0u, .size = 16u } };
                std::vector<VkDescriptorSetLayout> layouts = { meshDataDescriptorSetLayout, bindingsDescriptorSetLayout, deferredDescriptorSetLayout, samplingDescriptorSetLayout, materialDescriptorSetLayout };

                // 
                for (auto& layoutExt: this->extDescriptorSetLayout) {
                    if (layoutExt) { layouts.push_back(layoutExt); };
                }

                // 
                vkh::handleVk(this->driver->getDeviceDispatch()->CreatePipelineLayout(vkh::VkPipelineLayoutCreateInfo{}.setSetLayouts(layouts).setPushConstantRanges(ranges), nullptr, &this->unifiedPipelineLayout));
            };

            return uTHIS;
        };

        // 
        protected: virtual uPTR(Context) createDescriptorSets() {
            std::cout << "Create Descriptor Sets" << std::endl; // DEBUG!!

            if (!this->unifiedPipelineLayout) { this->createDescriptorSetLayouts(); };
            this->descriptorSets.resize(5u);

            {
                vkh::VsDescriptorSetCreateInfoHelper descInfo(deferredDescriptorSetLayout, thread->getDescriptorPool());

                { // 
                    vkh::VsDescriptorHandle<vkh::VkDescriptorImageInfo> handle = descInfo.pushDescription(vkh::VkDescriptorUpdateTemplateEntry{
                        .dstBinding = 13u,
                        .descriptorCount = 12u,
                        .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER
                    });

                    // 
                    //memcpy(&handle, &frameBfImages[0u].getDescriptor(), sizeof(VkDescriptorImageInfo) * 8u);
                    for (uint32_t i = 0; i < 12u; i++) {
                        handle.offset<vkh::VkDescriptorImageInfo>(i) = frameBfImages[i].getDescriptor();
                    };
                }

                { //
                    vkh::VsDescriptorHandle<vkh::VkDescriptorImageInfo> handle = descInfo.pushDescription(vkh::VkDescriptorUpdateTemplateEntry{
                        .dstBinding = 14u,
                        .descriptorCount = 12u,
                        .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER
                    });

                    // 
                    for (uint32_t i = 0; i < 12u; i++) {
                        this->driver->getDeviceDispatch()->CreateSampler(vkh::VkSamplerCreateInfo{
                           .magFilter = VK_FILTER_LINEAR,
                           .minFilter = VK_FILTER_LINEAR,
                           .addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
                           .addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE
                        }, nullptr, &(handle.offset<vkh::VkDescriptorImageInfo>(i) = vkt::ImageRegion(frameBfImages[i]).getDescriptor())->sampler);
                    }
                }

                { //
                    vkh::VsDescriptorHandle<vkh::VkDescriptorImageInfo> handle = descInfo.pushDescription(vkh::VkDescriptorUpdateTemplateEntry{
                            .dstBinding = 15u,
                            .descriptorCount = 8u,
                            .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER
                    });

                    //
                    for (uint32_t i = 0; i < 8u; i++) {
                        this->driver->getDeviceDispatch()->CreateSampler(vkh::VkSamplerCreateInfo{
                                .magFilter = VK_FILTER_LINEAR,
                                .minFilter = VK_FILTER_LINEAR,
                                .addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
                                .addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE
                        }, nullptr, &(handle.offset<vkh::VkDescriptorImageInfo>(i) = vkt::ImageRegion(rastersImages[i]).getDescriptor())->sampler);
                    }
                }

                // 
                vkh::handleVk(vkt::AllocateDescriptorSetWithUpdate(this->driver->getDeviceDispatch(), descInfo, this->deferredDescriptorSet, this->descriptorUpdatedF));
            };

            { // For Reprojection Pipeline
                vkh::VsDescriptorSetCreateInfoHelper descInfo(samplingDescriptorSetLayout, thread->getDescriptorPool());

                { //
                    vkh::VsDescriptorHandle<vkh::VkDescriptorImageInfo> handle = descInfo.pushDescription(vkh::VkDescriptorUpdateTemplateEntry{
                            .dstBinding = 16u,
                            .descriptorCount = 12u,
                            .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE
                    });

                    //
                    for (uint32_t i = 0; i < 12u; i++) {
                        handle.offset<vkh::VkDescriptorImageInfo>(i) = vkt::ImageRegion(frameBfImages[i]).getDescriptor();
                    };
                }

                {
                    vkh::VsDescriptorHandle<vkh::VkDescriptorImageInfo> handle = descInfo.pushDescription(vkh::VkDescriptorUpdateTemplateEntry{
                        .dstBinding = 17u,
                        .descriptorCount = 12u,
                        .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE
                    });

                    // 
                    for (uint32_t i = 0; i < 12u; i++) {
                        handle.offset<vkh::VkDescriptorImageInfo>(i) = smFlip0Images[i].getDescriptor();
                    };
                };

                {
                    vkh::VsDescriptorHandle<vkh::VkDescriptorImageInfo> handle = descInfo.pushDescription(vkh::VkDescriptorUpdateTemplateEntry{
                        .dstBinding = 18u,
                        .descriptorCount = 12u,
                        .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE
                    });

                    // 
                    for (uint32_t i = 0; i < 12u; i++) {
                        handle.offset<vkh::VkDescriptorImageInfo>(i) = smFlip1Images[i].getDescriptor();
                    };
                };

                // Reprojection WILL NOT write own depth... 
                vkh::handleVk(vkt::AllocateDescriptorSetWithUpdate(this->driver->getDeviceDispatch(), descInfo, this->smpFlip0DescriptorSet, this->descriptorUpdated0));
            };

            { // For Reprojection Pipeline
                vkh::VsDescriptorSetCreateInfoHelper descInfo(samplingDescriptorSetLayout, thread->getDescriptorPool());

                { //
                    vkh::VsDescriptorHandle<vkh::VkDescriptorImageInfo> handle = descInfo.pushDescription(vkh::VkDescriptorUpdateTemplateEntry{
                        .dstBinding = 16u,
                        .descriptorCount = 12u,
                        .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE
                    });

                    //
                    for (uint32_t i = 0; i < 12u; i++) {
                        handle.offset<vkh::VkDescriptorImageInfo>(i) = vkt::ImageRegion(frameBfImages[i]).getDescriptor();
                    };
                }

                {
                    vkh::VsDescriptorHandle<vkh::VkDescriptorImageInfo> handle = descInfo.pushDescription(vkh::VkDescriptorUpdateTemplateEntry{
                        .dstBinding = 17u,
                        .descriptorCount = 12u,
                        .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE
                    });

                    // 
                    for (uint32_t i = 0; i < 12u; i++) {
                        handle.offset<vkh::VkDescriptorImageInfo>(i) = smFlip1Images[i].getDescriptor();
                    };
                };

                {
                    vkh::VsDescriptorHandle<vkh::VkDescriptorImageInfo> handle = descInfo.pushDescription(vkh::VkDescriptorUpdateTemplateEntry{
                        .dstBinding = 18u,
                        .descriptorCount = 12u,
                        .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE
                    });

                    // 
                    for (uint32_t i = 0; i < 12u; i++) {
                        handle.offset<vkh::VkDescriptorImageInfo>(i) = smFlip0Images[i].getDescriptor();
                    };
                };

                // Reprojection WILL NOT write own depth... 
                vkh::handleVk(vkt::AllocateDescriptorSetWithUpdate(this->driver->getDeviceDispatch(), descInfo, this->smpFlip1DescriptorSet, this->descriptorUpdated1));
            };

            // 
            this->descriptorSets[2] = this->deferredDescriptorSet;
            this->descriptorSets[3] = this->smpFlip0DescriptorSet;

            // 
            return uTHIS;
        };

        // 
        public: virtual uPTR(Context) setDescriptorSet(uintptr_t I, vkt::uni_arg<VkDescriptorSet> set) {
            if (set.has() && *set) {
                this->descriptorSets.resize(5u + I + 1u);
                this->descriptorSets[I] = set;
            };
            return uTHIS;
        };

        // 
        public: virtual uPTR(Context) setDescriptorSetLayout(uintptr_t I, vkt::uni_arg<VkDescriptorSetLayout> layout) {
            if (layout.has() && *layout) {
                this->extDescriptorSetLayout.resize(I + 1u);
                this->extDescriptorSetLayout[I] = layout;
            };
            return uTHIS;
        };

        // 
        public: virtual uPTR(Context) initialize(const uint32_t& width = 1600u, const uint32_t& height = 1200u) {
            this->createRenderPass();
            this->createFramebuffers(width,height);
            this->createDescriptorSetLayouts();
            this->createDescriptorSets();
            return uTHIS;
        };

    protected: // 
        std::chrono::time_point<std::chrono::steady_clock> beginTime = std::chrono::high_resolution_clock::now();
        std::chrono::time_point<std::chrono::steady_clock> leastTime = std::chrono::high_resolution_clock::now();
        std::chrono::time_point<std::chrono::steady_clock> previTime = std::chrono::high_resolution_clock::now();
        bool descriptorUpdated0 = false;
        bool descriptorUpdated1 = false;
        bool descriptorUpdatedF = false;
        bool changedPerspective = false;

        // 
        vkh::VkRect2D scissor = {};
        vkh::VkViewport viewport = {};
        VkRenderPass renderPass = VK_NULL_HANDLE;
        VkRenderPass mapRenderPass = VK_NULL_HANDLE;
        VkFramebuffer smpFlip0Framebuffer = VK_NULL_HANDLE;
        VkFramebuffer smpFlip1Framebuffer = VK_NULL_HANDLE;
        VkFramebuffer deferredFramebuffer = VK_NULL_HANDLE;
        VkFramebuffer rasteredFramebuffer = VK_NULL_HANDLE;

        // 
        vkt::Vector<Matrices> uniformGPUData = {};
        vkt::Vector<Matrices> uniformRawData = {};
        //Matrices uniformData = {};

        // Image Buffers
        std::array<vkt::ImageRegion, 12u> smFlip0Images = {};
        std::array<vkt::ImageRegion, 12u> smFlip1Images = {}; // Path Tracing
        std::array<vkt::ImageRegion, 12u> frameBfImages = {}; // Rasterization
        std::array<vkt::ImageRegion, 8u>  rastersImages = {}; // Rasterization
        std::vector<VkDescriptorSet> descriptorSets = {};
        //std::vector<VkDescriptorSet> extDescriptorSets = {};
        vkt::ImageRegion depthImage = {};

        // 
        VkDescriptorSet deferredDescriptorSet = VK_NULL_HANDLE;
        VkDescriptorSet smpFlip0DescriptorSet = VK_NULL_HANDLE;
        VkDescriptorSet smpFlip1DescriptorSet = VK_NULL_HANDLE;
        VkPipelineLayout unifiedPipelineLayout = VK_NULL_HANDLE;

        // 
        VkDescriptorSetLayout materialDescriptorSetLayout = VK_NULL_HANDLE; // Material Descriptions
        VkDescriptorSetLayout deferredDescriptorSetLayout = VK_NULL_HANDLE; // Deferred Shading Descriptions (Diffuse Texturing)
        VkDescriptorSetLayout meshDataDescriptorSetLayout = VK_NULL_HANDLE; // Packed Mesh Data (8-bindings)
        VkDescriptorSetLayout samplingDescriptorSetLayout = VK_NULL_HANDLE; // Framebuffers and Samples (Diffuse, Path-Tracing and ReProjection)
        VkDescriptorSetLayout bindingsDescriptorSetLayout = VK_NULL_HANDLE; // Bindings, Attributes Descriptions
        std::vector<VkDescriptorSetLayout> extDescriptorSetLayout = {};

        // 
        vkh::VsDescriptorSetLayoutCreateInfoHelper materialDescriptorSetLayoutHelper = {};
        vkh::VsDescriptorSetLayoutCreateInfoHelper deferredDescriptorSetLayoutHelper = {};
        vkh::VsDescriptorSetLayoutCreateInfoHelper meshDataDescriptorSetLayoutHelper = {};
        vkh::VsDescriptorSetLayoutCreateInfoHelper samplingDescriptorSetLayoutHelper = {};
        vkh::VsDescriptorSetLayoutCreateInfoHelper bindingsDescriptorSetLayoutHelper = {};
        
        // 
        vkt::uni_ptr<Driver> driver = {};
        vkt::uni_ptr<Thread> thread = {};

        // 
        //glm::mat4 projected = glm::mat4(1.f);
        //glm::mat4 modelview = glm::mat4(1.f);
    };

};
