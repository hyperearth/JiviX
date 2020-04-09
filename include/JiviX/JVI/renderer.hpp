#pragma once // #

#include "./config.hpp"
#include "./driver.hpp"
#include "./thread.hpp"
#include "./context.hpp"
#include "./mesh.hpp"
#include "./node.hpp"

namespace jvi {

    // TODO: Descriptor Sets
    class Renderer : public std::enable_shared_from_this<Renderer> { public: // 
        Renderer(){};
        Renderer(vkt::uni_ptr<Context> context) : context(context) { this->construct(); };
        //Renderer(Context* context) { this->context = vkt::uni_ptr<Context>(context); this->construct(); };
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
            vkGetPhysicalDeviceProperties2(driver->getPhysicalDevice(), &(VkPhysicalDeviceProperties2&)this->properties);
            //driver->getPhysicalDevice().getProperties2(this->properties); // Vulkan-HPP Bugged
            //driver->getPhysicalDevice().getProperties2(&(VkPhysicalDeviceProperties2&)this->properties);

            // 
            this->raytraceStage = vkt::makePipelineStageInfo(this->driver->getDevice(), vkt::readBinary("./shaders/rtrace/raytrace.comp.spv"), vk::ShaderStageFlagBits::eCompute);
            this->denoiseStage = vkt::makePipelineStageInfo(this->driver->getDevice(), vkt::readBinary("./shaders/rtrace/denoise.comp.spv"), vk::ShaderStageFlagBits::eCompute);
            this->reflectStage = vkt::makePipelineStageInfo(this->driver->getDevice(), vkt::readBinary("./shaders/rtrace/reflect.comp.spv"), vk::ShaderStageFlagBits::eCompute);

            // 
            this->resampStages = vkt::vector_cast<vkh::VkPipelineShaderStageCreateInfo, vk::PipelineShaderStageCreateInfo>({ // 
                vkt::makePipelineStageInfo(this->driver->getDevice(), vkt::readBinary("./shaders/rtrace/resample.vert.spv"), vk::ShaderStageFlagBits::eVertex),
                vkt::makePipelineStageInfo(this->driver->getDevice(), vkt::readBinary("./shaders/rtrace/resample.frag.spv"), vk::ShaderStageFlagBits::eFragment)
            });

            return uTHIS;
        }

        // 
        virtual uPTR(Renderer) linkMaterial(vkt::uni_ptr<Material> materials) {
            this->materials = materials;
            if (this->materials->descriptorSet) {
                this->context->descriptorSets[4] = this->materials->descriptorSet;
            };
            return uTHIS;
        };

        // 
        virtual uPTR(Renderer) linkNode(vkt::uni_ptr<Node> node) {
            this->node = node;
            if (this->node->meshDataDescriptorSet) { this->context->descriptorSets[0] = this->node->meshDataDescriptorSet; };
            if (this->node->bindingsDescriptorSet) { this->context->descriptorSets[1] = this->node->bindingsDescriptorSet; };
            return uTHIS;
        };

        // 
        virtual uPTR(Renderer) setupSkyboxedCommand(const vk::CommandBuffer& rasterCommand = {}, const glm::uvec4& meshData = glm::uvec4(0u)) { // 
            this->denoiseState = vkt::handleHpp(vkt::createCompute(driver->getDevice(), vkt::FixConstruction(this->denoiseStage), vk::PipelineLayout(this->context->unifiedPipelineLayout), driver->getPipelineCache()));
            this->reflectState = vkt::handleHpp(vkt::createCompute(driver->getDevice(), vkt::FixConstruction(this->reflectStage), vk::PipelineLayout(this->context->unifiedPipelineLayout), driver->getPipelineCache()));
            this->raytraceState = vkt::handleHpp(vkt::createCompute(driver->getDevice(), vkt::FixConstruction(this->raytraceStage), vk::PipelineLayout(this->context->unifiedPipelineLayout), driver->getPipelineCache()));
            return uTHIS;
        };

        // 
        virtual uPTR(Renderer) saveDiffuseColor(const vk::CommandBuffer& saveCommand = {}) {
            const auto& viewport = this->context->refViewport();
            const auto& renderArea = this->context->refScissor();

            // 
            saveCommand.copyImage(
                this->context->frameBfImages[0u], this->context->frameBfImages[0u], 
                this->context->smFlip1Images[4u], this->context->smFlip1Images[4u], 
            { vk::ImageCopy(
                this->context->frameBfImages[0u], vk::Offset3D{0u,0u,0u}, 
                this->context->smFlip1Images[4u], vk::Offset3D{0u,0u,0u}, 
                vk::Extent3D{renderArea.extent.width, renderArea.extent.height, 1u}
            ) });
            vkt::commandBarrier(saveCommand);

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
            this->resamplingState = vkt::handleHpp(driver->getDevice().createGraphicsPipeline(driver->getPipelineCache(), this->pipelineInfo));

            // 
            this->reflectState  = vkt::handleHpp(vkt::createCompute(driver->getDevice(), vkt::FixConstruction(this->reflectStage ), vk::PipelineLayout(this->context->unifiedPipelineLayout), driver->getPipelineCache()));
            this->denoiseState  = vkt::handleHpp(vkt::createCompute(driver->getDevice(), vkt::FixConstruction(this->denoiseStage ), vk::PipelineLayout(this->context->unifiedPipelineLayout), driver->getPipelineCache()));
            this->raytraceState = vkt::handleHpp(vkt::createCompute(driver->getDevice(), vkt::FixConstruction(this->raytraceStage), vk::PipelineLayout(this->context->unifiedPipelineLayout), driver->getPipelineCache()));

            // 
            return uTHIS;
        };

        // 
        virtual uPTR(Renderer) setupResampleCommand(const vk::CommandBuffer& resampleCommand = {}, const glm::uvec4& meshData = glm::uvec4(0u)) {
            const auto& viewport = this->context->refViewport();
            const auto& renderArea = this->context->refScissor();
            const auto clearValues = std::vector<vk::ClearValue>{
                vk::ClearColorValue(std::array<float,4>{0.f, 0.f, 0.f, 0.0f}),
                vk::ClearColorValue(std::array<float,4>{0.f, 0.f, 0.f, 0.0f}),
                vk::ClearColorValue(std::array<float,4>{0.f, 0.f, 0.f, 0.0f}),
                vk::ClearColorValue(std::array<float,4>{0.f, 0.f, 0.f, 0.0f}),
                vk::ClearColorValue(std::array<float,4>{0.f, 0.f, 0.f, 0.0f}),
                vk::ClearColorValue(std::array<float,4>{0.f, 0.f, 0.f, 0.0f}),
                vk::ClearColorValue(std::array<float,4>{0.f, 0.f, 0.f, 0.0f}),
                vk::ClearColorValue(std::array<float,4>{0.f, 0.f, 0.f, 0.0f}),
                vk::ClearDepthStencilValue(1.0f, 0)
            };

            // 
            this->context->descriptorSets[3] = this->context->smpFlip1DescriptorSet;

            // 
            resampleCommand.beginRenderPass(vk::RenderPassBeginInfo(this->context->refRenderPass(), this->context->smpFlip0Framebuffer, renderArea, static_cast<uint32_t>(clearValues.size()), clearValues.data()), vk::SubpassContents::eInline);
            resampleCommand.bindPipeline(vk::PipelineBindPoint::eGraphics, this->resamplingState);
            resampleCommand.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, this->context->unifiedPipelineLayout, 0ull, this->context->descriptorSets, {});
            resampleCommand.pushConstants<glm::uvec4>(this->context->unifiedPipelineLayout, vk::ShaderStageFlags(VkShaderStageFlags(vkh::VkShaderStageFlags{ .eVertex = 1, .eGeometry = 1, .eFragment = 1, .eCompute = 1, .eRaygen = 1, .eClosestHit = 1, .eMiss = 1 })), 0u, { meshData });
            resampleCommand.setViewport(0, { viewport });
            resampleCommand.setScissor(0, { renderArea });
            resampleCommand.draw(renderArea.extent.width, renderArea.extent.height, 0u, 0u);
            resampleCommand.endRenderPass();
            //vkt::commandBarrier(resampleCommand);

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

            // TODO: Dedicated Command
            auto I = 0u; for (auto& M : this->node->meshes) { M->createRasterizePipeline()->createAccelerationStructure(); };

            // create sampling points
            this->materials->createDescriptorSet();
            this->node->createAccelerationStructure()->createDescriptorSet();

            // 
            this->setupResamplingPipeline();

            // 
            this->initialized = true;
            return uTHIS;
        };

        // 
        virtual uPTR(Renderer) setupCommands(vkt::uni_arg<vk::CommandBuffer> cmdBuf = {}, CommandOptions parameters = {1u,1u,1u,1u,1u,1u}) { // setup Commands
            const auto& viewport = this->context->refViewport();
            const auto& renderArea = this->context->refScissor();

            // 
            if (!initialized) {
                this->setupRenderer();
            };

            // 
            bool cmdArg = !(!cmdBuf || !(*cmdBuf));
            if (!cmdArg && !cmdbuf) { this->cmdbuf = vkt::createCommandBuffer(vk::Device(*thread), vk::CommandPool(*thread)); };
            if (!cmdArg) { cmdBuf = this->cmdbuf; };

            // 
            cmdBuf->copyBuffer(context->uniformRawData, context->uniformGPUData, { vk::BufferCopy(context->uniformRawData.offset(), context->uniformGPUData.offset(), context->uniformGPUData.range()) });
            this->context->descriptorSets[3] = this->context->smpFlip0DescriptorSet;

            // 
            if (parameters.eEnableRasterization) {
                // TODO: RE-ENABLE Rasterization Stage
            };

            // prepare meshes for ray-tracing
            this->materials->copyBuffers(cmdBuf);
            auto I = 0u; this->node->copyMeta(cmdBuf);
            {
                I = 0u; for (auto& M : this->node->meshes) { M->copyBuffers(cmdBuf); }; vkt::commandBarrier(cmdBuf);
            };
            if (parameters.eEnableBuildGeometry) {
                I = 0u; for (auto& M : this->node->meshes) { M->buildGeometry(cmdBuf, glm::uvec4(I++, 0u, 0u, 0u)); }; vkt::commandBarrier(cmdBuf);
            };
            if (parameters.eEnableBuildAccelerationStructure) {
                I = 0u; for (auto& M : this->node->meshes) { M->buildAccelerationStructure(cmdBuf, glm::uvec4(I++, 0u, 0u, 0u)); }; vkt::commandBarrier(cmdBuf);
                this->node->buildAccelerationStructure(cmdBuf);
            };

            // first-step rendering
            for (auto& M : this->node->meshes) { M->mapCount = 0u; };
            for (uint32_t i = 0; i < this->node->instanceCounter; i++) {
                const auto I = this->node->rawInstances[i].instanceId;
                this->node->meshes[I]->linkWithInstance(i);
            };

            // Compute ray-tracing (RTX)
            if (parameters.eEnableRayTracing) {
                this->context->descriptorSets[3] = this->context->smpFlip0DescriptorSet;
                cmdBuf->bindDescriptorSets(vk::PipelineBindPoint::eCompute, this->context->unifiedPipelineLayout, 0ull, this->context->descriptorSets, {});
                cmdBuf->bindPipeline(vk::PipelineBindPoint::eCompute, this->raytraceState);
                cmdBuf->pushConstants<glm::uvec4>(this->context->unifiedPipelineLayout, vkh::VkShaderStageFlags{.eVertex = 1, .eGeometry = 1, .eFragment = 1, .eCompute = 1, .eRaygen = 1, .eClosestHit = 1, .eMiss = 1 }.hpp(), 0u, { glm::uvec4(0u) });
                cmdBuf->dispatch(vkt::tiled(renderArea.extent.width, 32u), vkt::tiled(renderArea.extent.height, 24u), 1u);
                vkt::commandBarrier(cmdBuf);
            };

            // Make resampling pipeline 
            if (parameters.eEnableResampling) {
                this->setupResampleCommand(cmdBuf);
                vkt::commandBarrier(cmdBuf);
            };

            // Denoise diffuse data
            this->context->descriptorSets[3] = this->context->smpFlip0DescriptorSet;
            cmdBuf->bindDescriptorSets(vk::PipelineBindPoint::eCompute, this->context->unifiedPipelineLayout, 0ull, this->context->descriptorSets, {});
            cmdBuf->bindPipeline(vk::PipelineBindPoint::eCompute, this->denoiseState);
            cmdBuf->pushConstants<glm::uvec4>(this->context->unifiedPipelineLayout, vkh::VkShaderStageFlags{.eVertex = 1, .eGeometry = 1, .eFragment = 1, .eCompute = 1, .eRaygen = 1, .eClosestHit = 1, .eMiss = 1 }.hpp(), 0u, { glm::uvec4(0u) });
            cmdBuf->dispatch(vkt::tiled(renderArea.extent.width, 32u), vkt::tiled(renderArea.extent.height, 24u), 1u);
            vkt::commandBarrier(cmdBuf);

            // Use that version as previous frame
            if (parameters.eEnableResampling) {
                for (uint32_t i = 0; i < 8; i++) {
                    cmdBuf->copyImage(this->context->smFlip0Images[i], this->context->smFlip0Images[i], this->context->smFlip1Images[i], this->context->smFlip1Images[i], { vk::ImageCopy(
                        this->context->smFlip0Images[i], vk::Offset3D{0u,0u,0u}, this->context->smFlip1Images[i], vk::Offset3D{0u,0u,0u}, vk::Extent3D{renderArea.extent.width, renderArea.extent.height, 1u}
                    ) });
                };
                vkt::commandBarrier(cmdBuf);
            };

            // Denoise reflection data
            cmdBuf->bindDescriptorSets(vk::PipelineBindPoint::eCompute, this->context->unifiedPipelineLayout, 0ull, this->context->descriptorSets, {});
            cmdBuf->bindPipeline(vk::PipelineBindPoint::eCompute, this->reflectState);
            cmdBuf->pushConstants<glm::uvec4>(this->context->unifiedPipelineLayout, vkh::VkShaderStageFlags{.eVertex = 1, .eGeometry = 1, .eFragment = 1, .eCompute = 1, .eRaygen = 1, .eClosestHit = 1, .eMiss = 1 }.hpp(), 0u, { glm::uvec4(0u) });
            cmdBuf->dispatch(vkt::tiled(renderArea.extent.width, 32u), vkt::tiled(renderArea.extent.height, 24u), 1u);
            vkt::commandBarrier(cmdBuf);

            // 
            if (!cmdArg) { cmdBuf->end(); };
            return uTHIS;
        };

        // 
        vk::CommandBuffer& refCommandBuffer() { return cmdbuf; };
        const vk::CommandBuffer& refCommandBuffer() const { return cmdbuf; };

    protected: // 
        //std::vector<vk::CommandBuffer> commands = {};
        vk::CommandBuffer cmdbuf = {};

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
        vk::Pipeline resamplingState = {};
        vk::Pipeline denoiseState = {};
        vk::Pipeline reflectState = {};
        vk::Pipeline raytraceState = {};

        // 
        vkt::uni_ptr<Context> context = {};
        vkt::uni_ptr<Driver> driver = {};
        vkt::uni_ptr<Thread> thread = {};

        // 
        vk::PhysicalDeviceRayTracingPropertiesKHR rayTracingProperties = {};
        vk::PhysicalDeviceProperties2 properties = {};

        // 
        bool initialized = false;
    };

};
