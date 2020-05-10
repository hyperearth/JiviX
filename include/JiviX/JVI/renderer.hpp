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
        virtual vkt::uni_ptr<Renderer> sharedPtr() { return shared_from_this(); };
        //virtual vkt::uni_ptr<Renderer> sharedPtr() const { return std::shared_ptr<Renderer>(shared_from_this()); };

        // 
        virtual uPTR(Renderer) construct() {
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
            return uTHIS;
        };

        // 
        virtual uPTR(Renderer) linkMaterial(const std::shared_ptr<Material>& materials) { return this->linkMaterial(vkt::uni_ptr<Material>(materials)); };
        virtual uPTR(Renderer) linkNode(const std::shared_ptr<Node>& node) { return this->linkNode(vkt::uni_ptr<Node>(node)); };

        // 
        virtual uPTR(Renderer) linkMaterial(const vkt::uni_ptr<Material>& materials) {
            this->materials = materials;
            if (this->materials->descriptorSet) {
                this->context->descriptorSets[4] = this->materials->descriptorSet;
            };
            return uTHIS;
        };

        // 
        virtual uPTR(Renderer) linkNode(const vkt::uni_ptr<Node>& node) {
            this->node = node;
            if (this->node->meshDataDescriptorSet) { this->context->descriptorSets[0] = this->node->meshDataDescriptorSet; };
            if (this->node->bindingsDescriptorSet) { this->context->descriptorSets[1] = this->node->bindingsDescriptorSet; };
            return uTHIS;
        };

        // 
        virtual uPTR(Renderer) setupSkyboxedState(const VkCommandBuffer& rasterCommand = {}, const glm::uvec4& meshData = glm::uvec4(0u)) { // 
            this->denoiseState = vkt::createCompute(this->driver->getDeviceDispatch(), vkt::FixConstruction(this->denoiseStage), VkPipelineLayout(this->context->unifiedPipelineLayout), this->driver->getPipelineCache());
            this->reflectState = vkt::createCompute(this->driver->getDeviceDispatch(), vkt::FixConstruction(this->reflectStage), VkPipelineLayout(this->context->unifiedPipelineLayout), this->driver->getPipelineCache());
            this->raytraceState = vkt::createCompute(this->driver->getDeviceDispatch(), vkt::FixConstruction(this->raytraceStage), VkPipelineLayout(this->context->unifiedPipelineLayout), this->driver->getPipelineCache());
            //this->denoiseState = vkt::handleHpp(vkt::createCompute(driver->getDevice(), vkt::FixConstruction(this->denoiseStage), VkPipelineLayout(this->context->unifiedPipelineLayout), driver->getPipelineCache()));
            //this->reflectState = vkt::handleHpp(vkt::createCompute(driver->getDevice(), vkt::FixConstruction(this->reflectStage), VkPipelineLayout(this->context->unifiedPipelineLayout), driver->getPipelineCache()));
            //this->raytraceState = vkt::handleHpp(vkt::createCompute(driver->getDevice(), vkt::FixConstruction(this->raytraceStage), VkPipelineLayout(this->context->unifiedPipelineLayout), driver->getPipelineCache()));
            return uTHIS;
        };

        // 
        virtual uPTR(Renderer) saveDiffuseColor(const VkCommandBuffer& saveCommand = {}) {
            const auto& viewport = this->context->refViewport();
            const auto& renderArea = this->context->refScissor();

            // 
            this->driver->getDeviceDispatch()->CmdCopyImage(saveCommand, this->context->frameBfImages[0u], this->context->frameBfImages[0u], this->context->smFlip1Images[4u], this->context->smFlip1Images[4u], 1u, vkh::VkImageCopy{
                .srcSubresource = this->context->frameBfImages[0u], .srcOffset = vkh::VkOffset3D{ 0u,0u,0u },
                .dstSubresource = this->context->smFlip1Images[4u], .dstOffset = vkh::VkOffset3D{ 0u,0u,0u },
                .extent = vkh::VkExtent3D{ renderArea.extent.width, renderArea.extent.height, 1u }
            });

            // 
            return uTHIS;
        };

        // 
        virtual uPTR(Renderer) setupResamplingPipeline() {
            const auto& viewport = this->context->refViewport();
            const auto& renderArea = this->context->refScissor();

            // 
            this->pipelineInfo = vkh::VsGraphicsPipelineCreateInfoConstruction();
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
            this->driver->getDeviceDispatch()->CreateGraphicsPipelines(driver->getPipelineCache(), 1u, this->pipelineInfo, nullptr, &this->resamplingState);

            // 
            this->reflectState  = vkt::createCompute(this->driver->getDeviceDispatch(), vkt::FixConstruction(this->reflectStage ), VkPipelineLayout(this->context->unifiedPipelineLayout), this->driver->getPipelineCache());
            this->denoiseState  = vkt::createCompute(this->driver->getDeviceDispatch(), vkt::FixConstruction(this->denoiseStage ), VkPipelineLayout(this->context->unifiedPipelineLayout), this->driver->getPipelineCache());
            this->raytraceState = vkt::createCompute(this->driver->getDeviceDispatch(), vkt::FixConstruction(this->raytraceStage), VkPipelineLayout(this->context->unifiedPipelineLayout), this->driver->getPipelineCache());

            // 
            return uTHIS;
        };

        // 
        virtual uPTR(Renderer) setupResampleCommand(const VkCommandBuffer& resampleCommand = {}, const glm::uvec4& meshData = glm::uvec4(0u)) {
            const auto& viewport = reinterpret_cast<vkh::VkViewport&>(this->context->refViewport());
            const auto& renderArea = reinterpret_cast<vkh::VkRect2D&>(this->context->refScissor());
            const auto clearValues = std::vector<vkh::VkClearValue>{
                { .color = vkh::VkClearColorValue{.float32 = glm::vec4(0.f, 0.f, 0.f, 0.0f)} },
                { .color = vkh::VkClearColorValue{.float32 = glm::vec4(0.f, 0.f, 0.f, 0.0f)} },
                { .color = vkh::VkClearColorValue{.float32 = glm::vec4(0.f, 0.f, 0.f, 0.0f)} },
                { .color = vkh::VkClearColorValue{.float32 = glm::vec4(0.f, 0.f, 0.f, 0.0f)} },
                { .color = vkh::VkClearColorValue{.float32 = glm::vec4(0.f, 0.f, 0.f, 0.0f)} },
                { .color = vkh::VkClearColorValue{.float32 = glm::vec4(0.f, 0.f, 0.f, 0.0f)} },
                { .color = vkh::VkClearColorValue{.float32 = glm::vec4(0.f, 0.f, 0.f, 0.0f)} },
                { .color = vkh::VkClearColorValue{.float32 = glm::vec4(0.f, 0.f, 0.f, 0.0f)} },
                { .depthStencil = vkh::VkClearDepthStencilValue{.depth = 1.0f, .stencil = 0} }
            };

            // 
            this->context->descriptorSets[3] = this->context->smpFlip1DescriptorSet;

            // 
            this->driver->getDeviceDispatch()->CmdBeginRenderPass(resampleCommand, vkh::VkRenderPassBeginInfo{ .renderPass = this->context->refRenderPass(), .framebuffer = this->context->smpFlip0Framebuffer, .renderArea = renderArea, .clearValueCount = static_cast<uint32_t>(clearValues.size()), .pClearValues = clearValues.data() }, VK_SUBPASS_CONTENTS_INLINE);
            this->driver->getDeviceDispatch()->CmdBindPipeline(resampleCommand, VK_PIPELINE_BIND_POINT_GRAPHICS, this->resamplingState);
            this->driver->getDeviceDispatch()->CmdBindDescriptorSets(resampleCommand, VK_PIPELINE_BIND_POINT_GRAPHICS, this->context->unifiedPipelineLayout, 0u, this->context->descriptorSets.size(), this->context->descriptorSets.data(), 0u, nullptr);
            this->driver->getDeviceDispatch()->CmdPushConstants(resampleCommand, this->context->unifiedPipelineLayout, vkh::VkShaderStageFlags{.eVertex = 1, .eGeometry = 1, .eFragment = 1, .eCompute = 1, .eRaygen = 1, .eClosestHit = 1, .eMiss = 1}, 0u, sizeof(meshData), &meshData);
            this->driver->getDeviceDispatch()->CmdSetViewport(resampleCommand, 0u, 1u, viewport);
            this->driver->getDeviceDispatch()->CmdSetScissor(resampleCommand, 0u, 1u, renderArea);
            this->driver->getDeviceDispatch()->CmdDraw(resampleCommand, renderArea.extent.width, renderArea.extent.height, 0u, 0u);
            this->driver->getDeviceDispatch()->CmdEndRenderPass(resampleCommand);
            vkt::commandBarrier(this->driver->getDeviceDispatch(), resampleCommand);

            // 
            this->context->descriptorSets[3] = this->context->smpFlip0DescriptorSet;

            // 
            return uTHIS;
        };

        // 
        virtual uPTR(Renderer) setupRenderer() {
            const auto& viewport = this->context->refViewport();
            const auto& renderArea = this->context->refScissor();

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
        virtual uPTR(Renderer) setupCommands(vkt::uni_arg<VkCommandBuffer> cmdBuf = {}, vkt::uni_arg<CommandOptions> parameters = CommandOptions{1u,1u,1u,1u,1u,1u,1u}) { // setup Commands
            const auto& viewport = this->context->refViewport();
            const auto& renderArea = this->context->refScissor();
            const bool once = true;

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

            // 
            if (parameters->eEnableRasterization) {
                // TODO: RE-ENABLE Rasterization Stage
            };

            // prepare meshes for ray-tracing
            auto I = 0u;
            if (parameters->eEnableCopyMeta) {
                this->driver->getDeviceDispatch()->CmdCopyBuffer(currentCmd, context->uniformGPUData, context->uniformGPUData, 1u, vkh::VkBufferCopy{ context->uniformGPUData.offset() + offsetof(Matrices, modelview), context->uniformGPUData.offset() + offsetof(Matrices, modelviewPrev), 96ull }); // reserve previous projection (for adaptive denoise)
                this->driver->getDeviceDispatch()->CmdCopyBuffer(currentCmd, context->uniformRawData, context->uniformGPUData, 1u, vkh::VkBufferCopy{ context->uniformRawData.offset() + offsetof(Matrices, projection), context->uniformGPUData.offset() + offsetof(Matrices, projection), 224ull });
                this->driver->getDeviceDispatch()->CmdCopyBuffer(currentCmd, context->uniformRawData, context->uniformGPUData, 1u, vkh::VkBufferCopy{ context->uniformRawData.offset() + offsetof(Matrices, mdata), context->uniformGPUData.offset() + offsetof(Matrices, mdata),  32ull });

                I = 0u; for (auto& M : this->node->meshes) { M->copyBuffers(currentCmd); }; vkt::commandBarrier(this->driver->getDeviceDispatch(), currentCmd);
                this->materials->copyBuffers(currentCmd);
                this->node->copyMeta(currentCmd);
            };
            if (parameters->eEnableBuildGeometry) {
                I = 0u; for (auto& M : this->node->meshes) { M->buildGeometry(currentCmd, glm::uvec4(I++, 0u, 0u, 0u)); }; vkt::commandBarrier(this->driver->getDeviceDispatch(), currentCmd);
            };
            if (parameters->eEnableBuildAccelerationStructure) {
                I = 0u; for (auto& M : this->node->meshes) { M->buildAccelerationStructure(currentCmd, glm::uvec4(I++, 0u, 0u, 0u)); }; vkt::commandBarrier(this->driver->getDeviceDispatch(), currentCmd);
                this->node->buildAccelerationStructure(currentCmd);
            };

            // Compute ray-tracing (RTX)
            if (parameters->eEnableRayTracing) {
                this->context->descriptorSets[3] = this->context->smpFlip0DescriptorSet;

                this->driver->getDeviceDispatch()->CmdBindDescriptorSets(currentCmd, VK_PIPELINE_BIND_POINT_COMPUTE, this->context->unifiedPipelineLayout, 0u, this->context->descriptorSets.size(), this->context->descriptorSets.data(), 0u, nullptr);
                this->driver->getDeviceDispatch()->CmdBindPipeline(currentCmd, VK_PIPELINE_BIND_POINT_COMPUTE, this->raytraceState);
                this->driver->getDeviceDispatch()->CmdPushConstants(currentCmd, this->context->unifiedPipelineLayout, vkh::VkShaderStageFlags{.eVertex = 1, .eGeometry = 1, .eFragment = 1, .eCompute = 1, .eRaygen = 1, .eClosestHit = 1, .eMiss = 1 }, 0u, sizeof(glm::uvec4), &glm::uvec4(0u));
                this->driver->getDeviceDispatch()->CmdDispatch(currentCmd, vkt::tiled(renderArea.extent.width, 32u), vkt::tiled(renderArea.extent.height, 24u), 1u);
                vkt::commandBarrier(this->driver->getDeviceDispatch(), currentCmd);
            };

            // Make resampling pipeline 
            if (parameters->eEnableResampling) {
                this->setupResampleCommand(currentCmd);
                vkt::commandBarrier(this->driver->getDeviceDispatch(), currentCmd);
            };

            // Denoise diffuse data
            this->driver->getDeviceDispatch()->CmdBindDescriptorSets(currentCmd, VK_PIPELINE_BIND_POINT_COMPUTE, this->context->unifiedPipelineLayout, 0u, this->context->descriptorSets.size(), this->context->descriptorSets.data(), 0u, nullptr);
            this->driver->getDeviceDispatch()->CmdBindPipeline(currentCmd, VK_PIPELINE_BIND_POINT_COMPUTE, this->denoiseState);
            this->driver->getDeviceDispatch()->CmdPushConstants(currentCmd, this->context->unifiedPipelineLayout, vkh::VkShaderStageFlags{.eVertex = 1, .eGeometry = 1, .eFragment = 1, .eCompute = 1, .eRaygen = 1, .eClosestHit = 1, .eMiss = 1 }, 0u, sizeof(glm::uvec4), & glm::uvec4(0u));
            this->driver->getDeviceDispatch()->CmdDispatch(currentCmd, vkt::tiled(renderArea.extent.width, 32u), vkt::tiled(renderArea.extent.height, 24u), 1u);
            vkt::commandBarrier(this->driver->getDeviceDispatch(), currentCmd);

            // Denoise reflection data
            this->driver->getDeviceDispatch()->CmdBindDescriptorSets(currentCmd, VK_PIPELINE_BIND_POINT_COMPUTE, this->context->unifiedPipelineLayout, 0u, this->context->descriptorSets.size(), this->context->descriptorSets.data(), 0u, nullptr);
            this->driver->getDeviceDispatch()->CmdBindPipeline(currentCmd, VK_PIPELINE_BIND_POINT_COMPUTE, this->reflectState);
            this->driver->getDeviceDispatch()->CmdPushConstants(currentCmd, this->context->unifiedPipelineLayout, vkh::VkShaderStageFlags{.eVertex = 1, .eGeometry = 1, .eFragment = 1, .eCompute = 1, .eRaygen = 1, .eClosestHit = 1, .eMiss = 1 }, 0u, sizeof(glm::uvec4), & glm::uvec4(0u));
            this->driver->getDeviceDispatch()->CmdDispatch(currentCmd, vkt::tiled(renderArea.extent.width, 32u), vkt::tiled(renderArea.extent.height, 24u), 1u);
            vkt::commandBarrier(this->driver->getDeviceDispatch(), currentCmd);

            // 
            if (!hasBuf) {
                this->driver->getDeviceDispatch()->EndCommandBuffer(currentCmd);
            };
            return uTHIS;
        };

        // 
        VkCommandBuffer& refCommandBuffer() { return this->cmdBuf[current]; };
        const VkCommandBuffer& refCommandBuffer() const { return this->cmdBuf[current]; };

    protected: // 
        //std::vector<VkCommandBuffer> commands = {};
        //vkt::uni_arg<VkCommandBuffer> cmdbuf = {};

        //std::array<vkt::uni_arg<VkCommandBuffer>, 2> cmdBuf = {}; uint8_t current = 1u;
        std::array<VkCommandBuffer, 2> cmdBuf = {}; uint8_t current = 1u; // TODO: Comfort SWAP Class

        // binding data
        vkt::uni_ptr<Material> materials = {}; // materials
        vkt::uni_ptr<Node> node = {}; // currently only one node... 

        // 
        vkh::VsGraphicsPipelineCreateInfoConstruction pipelineInfo = {};
        std::vector<vkh::VkPipelineShaderStageCreateInfo> resampStages = {};
        vkh::VkPipelineShaderStageCreateInfo denoiseStage = {};
        vkh::VkPipelineShaderStageCreateInfo reflectStage = {};
        vkh::VkPipelineShaderStageCreateInfo raytraceStage = {};
         
        // 
        VkPipeline resamplingState = {};
        VkPipeline denoiseState = {};
        VkPipeline reflectState = {};
        VkPipeline raytraceState = {};

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
