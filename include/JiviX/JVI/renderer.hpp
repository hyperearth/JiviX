#pragma once // #

#include "./config.hpp"
#include "./driver.hpp"
#include "./thread.hpp"
#include "./context.hpp"
#include "./mesh-input.hpp"
#include "./mesh-binding.hpp"
#include "./node.hpp"


namespace jvi {

    // TODO: Descriptor Sets
    class Renderer : public std::enable_shared_from_this<Renderer> { public: // 
        Renderer(){};
        Renderer(const vkt::uni_ptr<Context>& context) : context(context) { this->construct(); };
        Renderer(const std::shared_ptr<Context>& context) : context(context) { this->construct(); };
        ~Renderer() {};

        // 
        public: virtual vkt::uni_ptr<Renderer> sharedPtr() { return shared_from_this(); };
        //virtual vkt::uni_ptr<Renderer> sharedPtr() const { return std::shared_ptr<Renderer>(shared_from_this()); };

        // 
        protected: virtual uPTR(Renderer) construct() {
            this->driver = context->getDriver();
            this->thread = std::make_shared<Thread>(this->driver);

            // get ray-tracing properties
            this->properties.pNext = &this->rayTracingProperties;
            this->driver->getInstanceDispatch()->GetPhysicalDeviceProperties2(driver->getPhysicalDevice(), this->properties);

            // 
            this->raytraceStage = vkt::makePipelineStageInfo(this->driver->getDeviceDispatch(), vkt::readBinary(std::string("./shaders/rtrace/raytrace.comp.spv")), VK_SHADER_STAGE_COMPUTE_BIT);
            this->denoiseStage = vkt::makePipelineStageInfo(this->driver->getDeviceDispatch(), vkt::readBinary(std::string("./shaders/rtrace/denoise.comp.spv")), VK_SHADER_STAGE_COMPUTE_BIT);
            this->reflectStage = vkt::makePipelineStageInfo(this->driver->getDeviceDispatch(), vkt::readBinary(std::string("./shaders/rtrace/reflect.comp.spv")), VK_SHADER_STAGE_COMPUTE_BIT);

            //
            this->resampStages = {
                vkt::makePipelineStageInfo(this->driver->getDeviceDispatch(), vkt::readBinary(std::string("./shaders/rtrace/resample.vert.spv")), VK_SHADER_STAGE_VERTEX_BIT),
                vkt::makePipelineStageInfo(this->driver->getDeviceDispatch(), vkt::readBinary(std::string("./shaders/rtrace/resample.geom.spv")), VK_SHADER_STAGE_GEOMETRY_BIT),
                vkt::makePipelineStageInfo(this->driver->getDeviceDispatch(), vkt::readBinary(std::string("./shaders/rtrace/resample.frag.spv")), VK_SHADER_STAGE_FRAGMENT_BIT)
            };

            // 
            this->raytraceStages = {
                vkt::makePipelineStageInfo(this->driver->getDeviceDispatch(), vkt::readBinary(std::string("./shaders/rtrace/raytrace.rgen.spv")), VK_SHADER_STAGE_RAYGEN_BIT_KHR),
                vkt::makePipelineStageInfo(this->driver->getDeviceDispatch(), vkt::readBinary(std::string("./shaders/rtrace/raytrace.rchit.spv")), VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR),
                vkt::makePipelineStageInfo(this->driver->getDeviceDispatch(), vkt::readBinary(std::string("./shaders/rtrace/raytrace.rmiss.spv")), VK_SHADER_STAGE_MISS_BIT_KHR)
            };

            //
            auto bflgs = vkh::VkBufferCreateFlags{};
            vkt::unlock32(bflgs) = 0u;

            //
            auto sbtUsage = vkh::VkBufferUsageFlags{.eTransferSrc = 1, .eStorageBuffer = 1, .eRayTracing = 1 };
            this->sbtBuffer = vkt::Vector<glm::u64vec4>(std::make_shared<vkt::VmaBufferAllocation>(this->driver->getAllocator(), vkh::VkBufferCreateInfo{ .flags = bflgs, .size = sizeof(glm::u64vec4) * 4u, .usage = sbtUsage }, vkt::VmaMemoryInfo{ .memUsage = VMA_MEMORY_USAGE_CPU_TO_GPU }));

            // 
            return uTHIS;
        };

        // 
        public: virtual uPTR(Renderer) linkMaterial(const std::shared_ptr<Material>& materials) { return this->linkMaterial(vkt::uni_ptr<Material>(materials)); };
        public: virtual uPTR(Renderer) linkNode(const std::shared_ptr<Node>& node) { return this->linkNode(vkt::uni_ptr<Node>(node)); };

        // 
        public: virtual uPTR(Renderer) linkMaterial(const vkt::uni_ptr<Material>& materials) {
            this->materials = materials;
            if (this->materials->descriptorSet) {
                this->context->descriptorSets[4] = this->materials->descriptorSet;
            };
            return uTHIS;
        };

        // 
        public: virtual uPTR(Renderer) linkNode(const vkt::uni_ptr<Node>& node) {
            this->node = node;
            if (this->node->meshDataDescriptorSet) { this->context->descriptorSets[0] = this->node->meshDataDescriptorSet; };
            if (this->node->bindingsDescriptorSet) { this->context->descriptorSets[1] = this->node->bindingsDescriptorSet; };
            return uTHIS;
        };

        // 
        protected: virtual uPTR(Renderer) setupSkyboxedState(vkt::uni_arg<VkCommandBuffer> rasterCommand = {}, const glm::uvec4& meshData = glm::uvec4(0u)) { // 
            this->denoiseState = vkt::createCompute(this->driver->getDeviceDispatch(), vkt::FixConstruction(this->denoiseStage), VkPipelineLayout(this->context->unifiedPipelineLayout), this->driver->getPipelineCache());
            this->reflectState = vkt::createCompute(this->driver->getDeviceDispatch(), vkt::FixConstruction(this->reflectStage), VkPipelineLayout(this->context->unifiedPipelineLayout), this->driver->getPipelineCache());
            this->raytraceState = vkt::createCompute(this->driver->getDeviceDispatch(), vkt::FixConstruction(this->raytraceStage), VkPipelineLayout(this->context->unifiedPipelineLayout), this->driver->getPipelineCache());
            //this->denoiseState = vkt::handleHpp(vkt::createCompute(driver->getDevice(), vkt::FixConstruction(this->denoiseStage), VkPipelineLayout(this->context->unifiedPipelineLayout), driver->getPipelineCache()));
            //this->reflectState = vkt::handleHpp(vkt::createCompute(driver->getDevice(), vkt::FixConstruction(this->reflectStage), VkPipelineLayout(this->context->unifiedPipelineLayout), driver->getPipelineCache()));
            //this->raytraceState = vkt::handleHpp(vkt::createCompute(driver->getDevice(), vkt::FixConstruction(this->raytraceStage), VkPipelineLayout(this->context->unifiedPipelineLayout), driver->getPipelineCache()));
            return uTHIS;
        };

        // 
        protected: virtual uPTR(Renderer) saveDiffuseColor(vkt::uni_arg<VkCommandBuffer> cmdBuf = {}) {
            const vkh::VkViewport viewport = reinterpret_cast<vkh::VkViewport&>(this->context->refViewport());
            const vkh::VkRect2D renderArea = reinterpret_cast<vkh::VkRect2D&>(this->context->refScissor());

            // 
            auto srcImage = this->context->getFrameBuffer(0u);
            auto dstImage = this->context->getFlip1Buffer(4u);
            this->driver->getDeviceDispatch()->CmdCopyImage(*cmdBuf,
                srcImage.getImage(), srcImage.getImageLayout(),
                dstImage.getImage(), dstImage.getImageLayout(),
                1u, vkh::VkImageCopy{
                    .srcSubresource = srcImage.subresourceLayers(), .srcOffset = vkh::VkOffset3D{ 0u,0u,0u },
                    .dstSubresource = dstImage.subresourceLayers(), .dstOffset = vkh::VkOffset3D{ 0u,0u,0u },
                    .extent = vkh::VkExtent3D{ renderArea.extent.width, renderArea.extent.height, 1u }
                });

            // 
            return uTHIS;
        };

        // 
        protected: virtual uPTR(Renderer) setupResamplingPipeline() {
            const vkh::VkViewport viewport = reinterpret_cast<vkh::VkViewport&>(this->context->refViewport());
            const vkh::VkRect2D renderArea = reinterpret_cast<vkh::VkRect2D&>(this->context->refScissor());

            //
            this->pipelineInfo = vkh::VsGraphicsPipelineCreateInfoConstruction();
            vkt::unlock32(this->pipelineInfo.graphicsPipelineCreateInfo.flags) = 0u;

            //
            for (uint32_t i = 0u; i < 8u; i++) { // 
                this->pipelineInfo.colorBlendAttachmentStates.push_back(vkh::VkPipelineColorBlendAttachmentState{
                    .blendEnable = true,
                    .srcColorBlendFactor = VK_BLEND_FACTOR_ONE,
                    .dstColorBlendFactor = VK_BLEND_FACTOR_ONE,
                    .srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE,
                    .dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE,
                });
            };

            // 
            this->pipelineInfo.stages = this->resampStages;
            this->pipelineInfo.depthStencilState = vkh::VkPipelineDepthStencilStateCreateInfo{ .depthTestEnable = false, .depthWriteEnable = false };
            this->pipelineInfo.inputAssemblyState.topology = VK_PRIMITIVE_TOPOLOGY_POINT_LIST;
            this->pipelineInfo.graphicsPipelineCreateInfo.renderPass = this->context->renderPass;
            this->pipelineInfo.graphicsPipelineCreateInfo.layout = this->context->unifiedPipelineLayout;
            this->pipelineInfo.viewportState.pViewports = &(vkh::VkViewport&)viewport;
            this->pipelineInfo.viewportState.pScissors = &(vkh::VkRect2D&)renderArea;
            this->pipelineInfo.dynamicStates.push_back(VK_DYNAMIC_STATE_VIEWPORT);
            this->pipelineInfo.dynamicStates.push_back(VK_DYNAMIC_STATE_SCISSOR);
            //this->resamplingState = vkt::handleHpp(driver->getDevice().createGraphicsPipeline(driver->getPipelineCache(), this->pipelineInfo));
            vkh::handleVk(this->driver->getDeviceDispatch()->CreateGraphicsPipelines(driver->getPipelineCache(), 1u, this->pipelineInfo, nullptr, &this->resamplingState));

            // 
            this->reflectState  = vkt::createCompute(this->driver->getDeviceDispatch(), vkt::FixConstruction(this->reflectStage ), VkPipelineLayout(this->context->unifiedPipelineLayout), this->driver->getPipelineCache());
            this->denoiseState  = vkt::createCompute(this->driver->getDeviceDispatch(), vkt::FixConstruction(this->denoiseStage ), VkPipelineLayout(this->context->unifiedPipelineLayout), this->driver->getPipelineCache());
            this->raytraceState = vkt::createCompute(this->driver->getDeviceDispatch(), vkt::FixConstruction(this->raytraceStage), VkPipelineLayout(this->context->unifiedPipelineLayout), this->driver->getPipelineCache());

            // 
            //this->raytraceStages
            vkh::VkRayTracingPipelineCreateInfoKHR rInfo = {};
            vkt::unlock32(rInfo.flags) = 0u;
            rInfo.layout = this->context->unifiedPipelineLayout;
            rInfo.maxRecursionDepth = 4u;
            //rInfo.setStages(this->raytraceStages);

            // 
            this->raytraceInfo = vkh::VsRayTracingPipelineCreateInfoHelper(rInfo);
            this->raytraceInfo.addShaderStages(this->raytraceStages);

            // 
            const size_t uHandleSize = this->rayTracingProperties.shaderGroupHandleSize, rHandleSize = uHandleSize * 3u;
            vkh::handleVk(this->driver->getDeviceDispatch()->CreateRayTracingPipelinesKHR(driver->getPipelineCache(), 1u, this->raytraceInfo, nullptr, &this->raytraceTypeState));
            vkh::handleVk(this->driver->getDeviceDispatch()->GetRayTracingShaderGroupHandlesKHR(this->raytraceTypeState, 0u, 3u, rHandleSize, this->sbtBuffer.data()));
            
            // 
            this->rgenSbtBuffer = vkt::Vector<glm::u64vec4>(this->sbtBuffer.getAllocation(), this->rayTracingProperties.shaderGroupHandleSize * 0u, 1u * this->rayTracingProperties.shaderGroupHandleSize, this->rayTracingProperties.shaderGroupHandleSize);
            this->rchitSbtBuffer = vkt::Vector<glm::u64vec4>(this->sbtBuffer.getAllocation(), this->rayTracingProperties.shaderGroupHandleSize * this->raytraceInfo.hitOffsetIndex(), 1u * this->rayTracingProperties.shaderGroupHandleSize, this->rayTracingProperties.shaderGroupHandleSize);
            this->rmissSbtBuffer = vkt::Vector<glm::u64vec4>(this->sbtBuffer.getAllocation(), this->rayTracingProperties.shaderGroupHandleSize * this->raytraceInfo.missOffsetIndex(), 1u * this->rayTracingProperties.shaderGroupHandleSize, this->rayTracingProperties.shaderGroupHandleSize);

            // 
            return uTHIS;
        };

        // 
        protected: virtual uPTR(Renderer) setupResampleCommand(vkt::uni_arg<VkCommandBuffer> cmdBuf = {}, const glm::uvec4& meshData = glm::uvec4(0u)) {
            const vkh::VkViewport viewport = reinterpret_cast<vkh::VkViewport&>(this->context->refViewport());
            const vkh::VkRect2D renderArea = reinterpret_cast<vkh::VkRect2D&>(this->context->refScissor());

            // 
            const auto clearValues = std::vector<vkh::VkClearValue>{
                {.color = vkh::VkClearColorValue{.float32 = glm::vec4(0.f, 0.f, 0.f, 0.0f)} },
                {.color = vkh::VkClearColorValue{.float32 = glm::vec4(0.f, 0.f, 0.f, 0.0f)} },
                {.color = vkh::VkClearColorValue{.float32 = glm::vec4(0.f, 0.f, 0.f, 0.0f)} },
                {.color = vkh::VkClearColorValue{.float32 = glm::vec4(0.f, 0.f, 0.f, 0.0f)} },
                {.color = vkh::VkClearColorValue{.float32 = glm::vec4(0.f, 0.f, 0.f, 0.0f)} },
                {.color = vkh::VkClearColorValue{.float32 = glm::vec4(0.f, 0.f, 0.f, 0.0f)} },
                {.color = vkh::VkClearColorValue{.float32 = glm::vec4(0.f, 0.f, 0.f, 0.0f)} },
                {.color = vkh::VkClearColorValue{.float32 = glm::vec4(0.f, 0.f, 0.f, 0.0f)} },
                {.depthStencil = vkh::VkClearDepthStencilValue{.depth = 1.0f, .stencil = 0} }
            };

            // 
            this->context->descriptorSets[3] = this->context->smpFlip1DescriptorSet;

            //
            auto pstage = vkh::VkShaderStageFlags{.eVertex = 1, .eGeometry = 1, .eFragment = 1, .eCompute = 1, .eRaygen = 1, .eClosestHit = 1, .eMiss = 1};
            this->driver->getDeviceDispatch()->CmdBeginRenderPass(cmdBuf, vkh::VkRenderPassBeginInfo{ .renderPass = this->context->refRenderPass(), .framebuffer = this->context->smpFlip0Framebuffer, .renderArea = renderArea, .clearValueCount = static_cast<uint32_t>(clearValues.size()), .pClearValues = clearValues.data() }, VK_SUBPASS_CONTENTS_INLINE);
            this->driver->getDeviceDispatch()->CmdBindPipeline(cmdBuf, VK_PIPELINE_BIND_POINT_GRAPHICS, this->resamplingState);
            this->driver->getDeviceDispatch()->CmdBindDescriptorSets(cmdBuf, VK_PIPELINE_BIND_POINT_GRAPHICS, this->context->unifiedPipelineLayout, 0u, this->context->descriptorSets.size(), this->context->descriptorSets.data(), 0u, nullptr);
            this->driver->getDeviceDispatch()->CmdPushConstants(cmdBuf, this->context->unifiedPipelineLayout, pstage, 0u, sizeof(meshData), &meshData);
            this->driver->getDeviceDispatch()->CmdSetViewport(cmdBuf, 0u, 1u, viewport);
            this->driver->getDeviceDispatch()->CmdSetScissor(cmdBuf, 0u, 1u, renderArea);
            this->driver->getDeviceDispatch()->CmdDraw(cmdBuf, renderArea.extent.width, renderArea.extent.height, 0u, 0u);
            this->driver->getDeviceDispatch()->CmdEndRenderPass(cmdBuf);
            vkt::commandBarrier(this->driver->getDeviceDispatch(), cmdBuf);

            //
            this->context->descriptorSets[3] = this->context->smpFlip0DescriptorSet;

            //
            return uTHIS;
        };

        // 
        public: virtual uPTR(Renderer) setupRenderer() {
            const vkh::VkViewport viewport = reinterpret_cast<vkh::VkViewport&>(this->context->refViewport());
            const vkh::VkRect2D renderArea = reinterpret_cast<vkh::VkRect2D&>(this->context->refScissor());

            // 
            if (!this->context->refRenderPass()) {
                this->context->createRenderPass();
            };

            // TODO: Build in Dedicated Command
            auto I = 0u; for (auto& M : this->node->meshes) { M->createRasterizePipeline()->createAccelerationStructure(); };

            // first-step rendering
            for (auto& M : this->node->meshes) { M->mapCount = 0u; };
            for (uint32_t i = 0; i < this->node->instanceCounter; i++) {
                const auto I = this->node->rawInstances[i].instanceId;
                this->node->meshes[I]->linkWithInstance(i);
            };

            // create sampling points
            this->materials->createDescriptorSet();
            this->node->createAccelerationStructure()->createDescriptorSet();

            // 
            this->setupResamplingPipeline();

            // 
            this->initialized = true;
            return uTHIS;
        };

        // TODO: Fix Command Create For Every Frame
        public: virtual uPTR(Renderer) setupCommands(vkt::uni_arg<VkCommandBuffer> cmdBuf = {}, const bool& once = true, vkt::uni_arg<CommandOptions> parameters = CommandOptions{1u,1u,1u,1u,1u,1u,1u,1u}) { // setup Commands
            const vkh::VkViewport viewport = reinterpret_cast<vkh::VkViewport&>(this->context->refViewport());
            const vkh::VkRect2D renderArea = reinterpret_cast<vkh::VkRect2D&>(this->context->refScissor());

            // 
            if (!this->initialized) { this->setupRenderer(); };

            // Use Current Command Swapness
            auto& currentCmd = this->cmdBuf[this->current^=1];

            // 
            const bool hasBuf = cmdBuf.has() && *cmdBuf;
            if (!hasBuf) {
                //if (currentCmd) { VkDevice(*thread).freeCommandBuffers(VkCommandPool(*thread), { currentCmd }); currentCmd = VkCommandBuffer{}; };
                //if (!currentCmd) { currentCmd = vkt::createCommandBuffer(VkDevice(*thread), VkCommandPool(*thread), false, once); };
                currentCmd = vkt::createCommandBuffer(this->driver->getDeviceDispatch(), VkCommandPool(*thread), false, once);
            } else {
                currentCmd = cmdBuf;
            };

            // 
            if (this->node->meshDataDescriptorSet) { this->context->descriptorSets[0] = this->node->meshDataDescriptorSet; };
            if (this->node->bindingsDescriptorSet) { this->context->descriptorSets[1] = this->node->bindingsDescriptorSet; };
            if (this->materials->descriptorSet) {  this->context->descriptorSets[4] = this->materials->descriptorSet; };

            // 
            this->context->descriptorSets[3] = this->context->smpFlip0DescriptorSet;

            // prepare meshes for ray-tracing
            auto I = 0u;
            if (parameters->eEnableCopyMeta) {
                this->driver->getDeviceDispatch()->CmdCopyBuffer(currentCmd, context->uniformGPUData, context->uniformGPUData, 1u, vkh::VkBufferCopy{ context->uniformGPUData.offset() + offsetof(Matrices, modelview), context->uniformGPUData.offset() + offsetof(Matrices, modelviewPrev), 96ull }); // reserve previous projection (for adaptive denoise)
                this->driver->getDeviceDispatch()->CmdCopyBuffer(currentCmd, context->uniformRawData, context->uniformGPUData, 1u, vkh::VkBufferCopy{ context->uniformRawData.offset() + offsetof(Matrices, projection), context->uniformGPUData.offset() + offsetof(Matrices, projection), 224ull });
                this->driver->getDeviceDispatch()->CmdCopyBuffer(currentCmd, context->uniformRawData, context->uniformGPUData, 1u, vkh::VkBufferCopy{ context->uniformRawData.offset() + offsetof(Matrices, mdata), context->uniformGPUData.offset() + offsetof(Matrices, mdata),  32ull });

                I = 0u; for (auto& M : this->node->meshes) { M->copyBuffers(currentCmd); }; vkt::commandBarrier(this->driver->getDeviceDispatch(), currentCmd);
                this->materials->copyBuffers(currentCmd);
                //this->node->copyMeta(currentCmd);
                vkt::commandBarrier(this->driver->getDeviceDispatch(), currentCmd);
            };

            // 
            if (parameters->eEnableBuildGeometry) { this->node->copyMeta(currentCmd); };
            if (parameters->eEnableBuildAccelerationStructure) { this->node->buildAccelerationStructure(currentCmd); };
            if (parameters->eEnableMapping) { this->node->mappingGeometry(currentCmd); };
            if (parameters->eEnableRasterization) { this->node->rasterizeGeometry(currentCmd); };

            // Compute ray-tracing (RTX)
            auto pstage = vkh::VkShaderStageFlags{.eVertex = 1, .eGeometry = 1, .eFragment = 1, .eCompute = 1, .eRaygen = 1, .eClosestHit = 1, .eMiss = 1 };
            if (parameters->eEnableRayTracing) {
                const auto vect0 = glm::uvec4(0u);
                this->context->descriptorSets[3] = this->context->smpFlip0DescriptorSet;
                this->driver->getDeviceDispatch()->CmdBindPipeline(currentCmd, VK_PIPELINE_BIND_POINT_COMPUTE, this->raytraceState);
                this->driver->getDeviceDispatch()->CmdBindDescriptorSets(currentCmd, VK_PIPELINE_BIND_POINT_COMPUTE, this->context->unifiedPipelineLayout, 0u, this->context->descriptorSets.size(), this->context->descriptorSets.data(), 0u, nullptr);
                this->driver->getDeviceDispatch()->CmdPushConstants(currentCmd, this->context->unifiedPipelineLayout, pstage, 0u, sizeof(glm::uvec4), &vect0);
                this->driver->getDeviceDispatch()->CmdDispatch(currentCmd, vkt::tiled(renderArea.extent.width, 64u), vkt::tiled(renderArea.extent.height, 24u), 1u);
                vkt::commandBarrier(this->driver->getDeviceDispatch(), currentCmd);

                //this->driver->getDeviceDispatch()->CmdBindPipeline(currentCmd, VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR, this->raytraceTypeState);
                //this->driver->getDeviceDispatch()->CmdBindDescriptorSets(currentCmd, VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR, this->context->unifiedPipelineLayout, 0u, this->context->descriptorSets.size(), this->context->descriptorSets.data(), 0u, nullptr);
                //this->driver->getDeviceDispatch()->CmdPushConstants(currentCmd, this->context->unifiedPipelineLayout, vkh::VkShaderStageFlags{.eVertex = 1, .eGeometry = 1, .eFragment = 1, .eCompute = 1, .eRaygen = 1, .eClosestHit = 1, .eMiss = 1 }, 0u, sizeof(glm::uvec4), & glm::uvec4(0u));
                //this->driver->getDeviceDispatch()->CmdTraceRaysKHR(currentCmd, this->rgenSbtBuffer.getRegion(), this->rmissSbtBuffer.getRegion(), this->rchitSbtBuffer.getRegion(), vkh::VkStridedBufferRegionKHR{}, renderArea.extent.width, renderArea.extent.height, 1u);
                //vkt::commandBarrier(this->driver->getDeviceDispatch(), currentCmd);
            };

            // Make resampling pipeline 
            if (parameters->eEnableResampling) {
                this->setupResampleCommand(currentCmd);
                vkt::commandBarrier(this->driver->getDeviceDispatch(), currentCmd);
            };

            if (parameters->eEnableDenoise) {
                const auto vect0 = glm::uvec4(0u);
                auto pstages = vkh::VkShaderStageFlags{.eVertex = 1, .eGeometry = 1, .eFragment = 1, .eCompute = 1, .eRaygen = 1, .eClosestHit = 1, .eMiss = 1 };

                // Denoise diffuse data
                this->driver->getDeviceDispatch()->CmdBindPipeline(currentCmd, VK_PIPELINE_BIND_POINT_COMPUTE, this->denoiseState);
                this->driver->getDeviceDispatch()->CmdBindDescriptorSets(currentCmd, VK_PIPELINE_BIND_POINT_COMPUTE, this->context->unifiedPipelineLayout, 0u, this->context->descriptorSets.size(), this->context->descriptorSets.data(), 0u, nullptr);
                this->driver->getDeviceDispatch()->CmdPushConstants(currentCmd, this->context->unifiedPipelineLayout, pstages, 0u, sizeof(glm::uvec4), &vect0);
                this->driver->getDeviceDispatch()->CmdDispatch(currentCmd, vkt::tiled(renderArea.extent.width, 32u), vkt::tiled(renderArea.extent.height, 24u), 1u);
                vkt::commandBarrier(this->driver->getDeviceDispatch(), currentCmd);

                // Denoise reflection data
                this->driver->getDeviceDispatch()->CmdBindPipeline(currentCmd, VK_PIPELINE_BIND_POINT_COMPUTE, this->reflectState);
                this->driver->getDeviceDispatch()->CmdBindDescriptorSets(currentCmd, VK_PIPELINE_BIND_POINT_COMPUTE, this->context->unifiedPipelineLayout, 0u, this->context->descriptorSets.size(), this->context->descriptorSets.data(), 0u, nullptr);
                this->driver->getDeviceDispatch()->CmdPushConstants(currentCmd, this->context->unifiedPipelineLayout, pstages, 0u, sizeof(glm::uvec4), &vect0);
                this->driver->getDeviceDispatch()->CmdDispatch(currentCmd, vkt::tiled(renderArea.extent.width, 32u), vkt::tiled(renderArea.extent.height, 24u), 1u);
                vkt::commandBarrier(this->driver->getDeviceDispatch(), currentCmd);
            };

            if (!hasBuf) { // 
                this->driver->getDeviceDispatch()->EndCommandBuffer(currentCmd);
            };

            return uTHIS;
        };

        // 
        public: VkCommandBuffer& refCommandBuffer() { return this->cmdBuf[current]; };
        public: const VkCommandBuffer& refCommandBuffer() const { return this->cmdBuf[current]; };

    protected: // 
        //std::vector<VkCommandBuffer> commands = {};
        //vkt::uni_arg<VkCommandBuffer> cmdbuf = {};

        //std::array<vkt::uni_arg<VkCommandBuffer>, 2> cmdBuf = {}; uint8_t current = 1u;
        std::array<VkCommandBuffer, 2> cmdBuf = {VK_NULL_HANDLE,VK_NULL_HANDLE}; uint8_t current = 1u; // TODO: Comfort SWAP Class

        // binding data
        vkt::uni_ptr<Material> materials = {}; // materials
        vkt::uni_ptr<Node> node = {}; // currently only one node... 

        // 
        vkt::Vector<glm::u64vec4> sbtBuffer = {};
        vkt::Vector<glm::u64vec4> rgenSbtBuffer = {};
        vkt::Vector<glm::u64vec4> rchitSbtBuffer = {};
        vkt::Vector<glm::u64vec4> rmissSbtBuffer = {};

        // 
        vkh::VsRayTracingPipelineCreateInfoHelper raytraceInfo = {};
        vkh::VsGraphicsPipelineCreateInfoConstruction pipelineInfo = {};
        std::vector<vkh::VkPipelineShaderStageCreateInfo> resampStages = {};
        std::vector<vkh::VkPipelineShaderStageCreateInfo> raytraceStages = {};
        vkh::VkPipelineShaderStageCreateInfo denoiseStage = {};
        vkh::VkPipelineShaderStageCreateInfo reflectStage = {};
        vkh::VkPipelineShaderStageCreateInfo raytraceStage = {};
         
        // 
        VkPipeline resamplingState = VK_NULL_HANDLE;
        VkPipeline denoiseState = VK_NULL_HANDLE;
        VkPipeline reflectState = VK_NULL_HANDLE;
        VkPipeline raytraceState = VK_NULL_HANDLE;
        VkPipeline raytraceTypeState = VK_NULL_HANDLE;

        // 
        vkt::uni_ptr<Context> context = {};
        vkt::uni_ptr<Driver> driver = {};
        vkt::uni_ptr<Thread> thread = {};

        // 
        vkh::VkPhysicalDeviceRayTracingPropertiesKHR rayTracingProperties = {};
        vkh::VkPhysicalDeviceProperties2 properties = {};

        // 
        bool initialized = false;
    };

};
