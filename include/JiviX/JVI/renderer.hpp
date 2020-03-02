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
        Renderer(const vkt::uni_ptr<Context>& context) : context(context) { this->construct(); };
        //Renderer(Context* context) { this->context = vkt::uni_ptr<Context>(context); this->construct(); };
        ~Renderer() {};

        // 
        virtual std::shared_ptr<Renderer> sharedPtr() { return shared_from_this(); };
        virtual std::shared_ptr<const Renderer> sharedPtr() const { return shared_from_this(); };

        // 
        virtual uPTR(Renderer) construct() {
            this->driver = context->getDriver();
            this->thread = std::make_shared<Thread>(this->driver);

            // get ray-tracing properties
            this->properties.pNext = &this->rayTracingProperties;
            vkGetPhysicalDeviceProperties2(driver->getPhysicalDevice(), &(VkPhysicalDeviceProperties2&)this->properties);
            //driver->getPhysicalDevice().getProperties2(this->properties); // Vulkan-HPP Bugged

            // 
            const auto& rtxp = rayTracingProperties;
            this->rawSBTBuffer = vkt::Vector<uint64_t>(this->driver->getAllocator(), vkh::VkBufferCreateInfo{ .size = rtxp.shaderGroupHandleSize * 8u, .usage = {.eTransferSrc = 1, .eUniformBuffer = 1, .eRayTracing = 1 } }, VMA_MEMORY_USAGE_CPU_TO_GPU);
            this->gpuSBTBuffer = vkt::Vector<uint64_t>(this->driver->getAllocator(), vkh::VkBufferCreateInfo{ .size = rtxp.shaderGroupHandleSize * 8u, .usage = {.eTransferDst = 1, .eUniformBuffer = 1, .eRayTracing = 1 } }, VMA_MEMORY_USAGE_GPU_ONLY);

            // Pre-Initialize Stages For FASTER CODE
            this->skyboxStages = vkt::vector_cast<vkh::VkPipelineShaderStageCreateInfo, vk::PipelineShaderStageCreateInfo>({
                vkt::makePipelineStageInfo(this->driver->getDevice(), vkt::readBinary("./shaders/rtrace/background.vert.spv"), vk::ShaderStageFlagBits::eVertex),
                vkt::makePipelineStageInfo(this->driver->getDevice(), vkt::readBinary("./shaders/rtrace/background.frag.spv"), vk::ShaderStageFlagBits::eFragment)
                });

            // 
            this->rtStages = vkt::vector_cast<vkh::VkPipelineShaderStageCreateInfo, vk::PipelineShaderStageCreateInfo>({
                vkt::makePipelineStageInfo(driver->getDevice(), vkt::readBinary("./shaders/rtrace/pathtrace.rgen.spv"), vk::ShaderStageFlagBits::eRaygenNV),
                vkt::makePipelineStageInfo(driver->getDevice(), vkt::readBinary("./shaders/rtrace/pathtrace.rchit.spv"), vk::ShaderStageFlagBits::eClosestHitNV),
                vkt::makePipelineStageInfo(driver->getDevice(), vkt::readBinary("./shaders/rtrace/pathtrace.rmiss.spv"), vk::ShaderStageFlagBits::eMissNV)
                });

            // 
            this->resampStages = vkt::vector_cast<vkh::VkPipelineShaderStageCreateInfo, vk::PipelineShaderStageCreateInfo>({ // 
                vkt::makePipelineStageInfo(this->driver->getDevice(), vkt::readBinary("./shaders/rtrace/resample.vert.spv"), vk::ShaderStageFlagBits::eVertex),
                vkt::makePipelineStageInfo(this->driver->getDevice(), vkt::readBinary("./shaders/rtrace/resample.frag.spv"), vk::ShaderStageFlagBits::eFragment)
                });

            return uTHIS;
        }

        // 
        virtual uPTR(Renderer) linkMaterial(const vkt::uni_ptr<Material>& materials = {}) {
            this->materials = materials;
            return uTHIS;
        };

        // 
        virtual uPTR(Renderer) linkNode(const vkt::uni_ptr<Node>& node = {}) {
            this->node = node;
            return uTHIS;
        };

        // 
        virtual uPTR(Renderer) setupRayTracingPipeline() { // 
            this->rayTraceInfo = vkh::VsRayTracingPipelineCreateInfoHelper();
            this->rayTraceInfo.vkInfo.layout = this->context->unifiedPipelineLayout;
            this->rayTraceInfo.vkInfo.maxRecursionDepth = 4u;
            this->rayTraceInfo.addShaderStages(this->rtStages);
            this->rayTraceInfo.addShaderStages(this->bgStages);
            this->rayTracingState = driver->getDevice().createRayTracingPipelineNV(driver->getPipelineCache(),this->rayTraceInfo,nullptr,this->driver->getDispatch());

            // get ray-tracing properties
            this->driver->getDevice().getRayTracingShaderGroupHandlesNV(this->rayTracingState,0u,this->rayTraceInfo.groupCount(),this->rayTraceInfo.groupCount()*rayTracingProperties.shaderGroupHandleSize,this->rawSBTBuffer.data(),this->driver->getDispatch());
            return uTHIS;
        };

        // 
        virtual uPTR(Renderer) setupBackgroundPipeline() {
            const auto& viewport = this->context->refViewport();
            const auto& renderArea = this->context->refScissor();

            this->skyboxedInfo = vkh::VsGraphicsPipelineCreateInfoConstruction();
            for (uint32_t i = 0u; i < 8u; i++) {
                this->skyboxedInfo.colorBlendAttachmentStates.push_back(vkh::VkPipelineColorBlendAttachmentState{ .blendEnable = false }); // transparency will generated by ray-tracing
            };

            this->skyboxedInfo.stages = this->skyboxStages;
            this->skyboxedInfo.depthStencilState = vkh::VkPipelineDepthStencilStateCreateInfo{
                .depthTestEnable = true,
                .depthWriteEnable = true
            };

            this->skyboxedInfo.inputAssemblyState.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;
            this->skyboxedInfo.dynamicStates.push_back(VK_DYNAMIC_STATE_VIEWPORT);
            this->skyboxedInfo.dynamicStates.push_back(VK_DYNAMIC_STATE_SCISSOR);
            this->skyboxedInfo.graphicsPipelineCreateInfo.renderPass = this->context->renderPass;
            this->skyboxedInfo.graphicsPipelineCreateInfo.layout = this->context->unifiedPipelineLayout;
            this->skyboxedInfo.viewportState.pViewports = &(vkh::VkViewport&)viewport;
            this->skyboxedInfo.viewportState.pScissors = &(vkh::VkRect2D&)renderArea;
            this->backgroundState = driver->getDevice().createGraphicsPipeline(driver->getPipelineCache(), this->skyboxedInfo);

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
            this->resamplingState = driver->getDevice().createGraphicsPipeline(driver->getPipelineCache(), this->pipelineInfo);

            // 
            return uTHIS;
        };

        // 
        virtual uPTR(Renderer) setupSkyboxedCommand(const vk::CommandBuffer& rasterCommand = {}, const glm::uvec4& meshData = glm::uvec4(0u)) { // 
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
            this->context->descriptorSets[3] = this->context->smpFlip0DescriptorSet;

            // 
            for (uint32_t i = 0u; i < 8u; i++) { // Definitely Not an Hotel
                //rasterCommand.clearColorImage(this->context->smFlip1Images[i], vk::ImageLayout::eGeneral, vk::ClearColorValue().setFloat32({ 1.f,1.f,1.f,1.f }), { this->context->smFlip1Images[i] });
                rasterCommand.clearColorImage(this->context->smFlip0Images[i], vk::ImageLayout::eGeneral, vk::ClearColorValue().setFloat32({ 0.f,0.f,0.f,0.f }), { this->context->smFlip0Images[i] });
                rasterCommand.clearColorImage(this->context->frameBfImages[i], vk::ImageLayout::eGeneral, vk::ClearColorValue().setFloat32({ 0.f,0.f,0.f,0.f }), { this->context->frameBfImages[i] });
            };

            rasterCommand.clearDepthStencilImage(this->context->depthImage, vk::ImageLayout::eGeneral, clearValues[8u].depthStencil, (vk::ImageSubresourceRange&)this->context->depthImage.subresourceRange);
            vkt::commandBarrier(this->cmdbuf);
            rasterCommand.beginRenderPass(vk::RenderPassBeginInfo(this->context->refRenderPass(), this->context->deferredFramebuffer, renderArea, clearValues.size(), clearValues.data()), vk::SubpassContents::eInline);
            rasterCommand.setViewport(0, { viewport });
            rasterCommand.setScissor(0, { renderArea });
            rasterCommand.pushConstants<glm::uvec4>(this->context->unifiedPipelineLayout, vk::ShaderStageFlags(VkShaderStageFlags(vkh::VkShaderStageFlags{ .eVertex = 1, .eGeometry = 1, .eFragment = 1, .eRaygen = 1, .eClosestHit = 1, .eMiss = 1 })), 0u, { meshData });
            rasterCommand.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, this->context->unifiedPipelineLayout, 0ull, this->context->descriptorSets, {});
            rasterCommand.bindPipeline(vk::PipelineBindPoint::eGraphics, this->backgroundState);
            rasterCommand.draw(4u, 1u, 0u, 0u);
            rasterCommand.endRenderPass();
            vkt::commandBarrier(rasterCommand);

            // 
            return uTHIS;
        };

        // 
        virtual uPTR(Renderer) setupRayTraceCommand(const vk::CommandBuffer& rayTraceCommand = {}, const glm::uvec4& meshData = glm::uvec4(0u)) { // get ray-tracing properties
            const auto& rtxp = this->rayTracingProperties;
            const auto& renderArea = this->context->refScissor();

            this->context->descriptorSets[3] = this->context->smpFlip1DescriptorSet;

            // copy resampled data into ray tracing samples
            for (uint32_t i = 0; i < 8; i++) {
                rayTraceCommand.copyImage(this->context->smFlip0Images[i], this->context->smFlip0Images[i], this->context->smFlip1Images[i], this->context->smFlip1Images[i], { vk::ImageCopy(
                    this->context->smFlip0Images[i], vk::Offset3D{0u,0u,0u}, this->context->smFlip1Images[i], vk::Offset3D{0u,0u,0u}, vk::Extent3D{renderArea.extent.width, renderArea.extent.height, 1u}
                ) });
            };
            rayTraceCommand.copyBuffer(this->rawSBTBuffer, this->gpuSBTBuffer, { vk::BufferCopy(this->rawSBTBuffer.offset(),this->gpuSBTBuffer.offset(),this->rayTraceInfo.groupCount() * rtxp.shaderGroupHandleSize) });

            // Clear NO anymore needed data
            vkt::commandBarrier(rayTraceCommand);
            for (uint32_t i = 0u; i < 8u; i++) { // Definitely Not an Hotel
                //rayTraceCommand.clearColorImage(this->context->smFlip1Images[i], vk::ImageLayout::eGeneral, vk::ClearColorValue().setFloat32({ 1.f,1.f,1.f,1.f }), { this->context->smFlip1Images[i] });
                rayTraceCommand.clearColorImage(this->context->smFlip0Images[i], vk::ImageLayout::eGeneral, vk::ClearColorValue().setFloat32({ 0.f,0.f,0.f,0.f }), { this->context->smFlip0Images[i] });
            };

            vkt::commandBarrier(rayTraceCommand);
            rayTraceCommand.bindPipeline(vk::PipelineBindPoint::eRayTracingNV, this->rayTracingState);
            rayTraceCommand.bindDescriptorSets(vk::PipelineBindPoint::eRayTracingNV, this->context->unifiedPipelineLayout, 0ull, this->context->descriptorSets, {});
            rayTraceCommand.pushConstants<glm::uvec4>(this->context->unifiedPipelineLayout, vk::ShaderStageFlags(VkShaderStageFlags(vkh::VkShaderStageFlags{ .eVertex = 1, .eGeometry = 1, .eFragment = 1, .eRaygen = 1, .eClosestHit = 1, .eMiss = 1 })), 0u, { meshData });
            rayTraceCommand.traceRaysNV(
                this->gpuSBTBuffer, this->gpuSBTBuffer.offset(),
                this->gpuSBTBuffer, this->gpuSBTBuffer.offset() + this->rayTraceInfo.missOffsetIndex() * rtxp.shaderGroupHandleSize, rtxp.shaderGroupHandleSize,
                this->gpuSBTBuffer, this->gpuSBTBuffer.offset() + this->rayTraceInfo.hitOffsetIndex() * rtxp.shaderGroupHandleSize, rtxp.shaderGroupHandleSize,
                {}, 0u, 0u,
                renderArea.extent.width, renderArea.extent.height, 1u,
                this->driver->getDispatch()
            );
            vkt::commandBarrier(rayTraceCommand);
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

            this->context->descriptorSets[3] = this->context->smpFlip1DescriptorSet;

            // 
            resampleCommand.beginRenderPass(vk::RenderPassBeginInfo(this->context->refRenderPass(), this->context->smpFlip0Framebuffer, renderArea, clearValues.size(), clearValues.data()), vk::SubpassContents::eInline);
            resampleCommand.bindPipeline(vk::PipelineBindPoint::eGraphics, this->resamplingState);
            resampleCommand.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, this->context->unifiedPipelineLayout, 0ull, this->context->descriptorSets, {});
            resampleCommand.pushConstants<glm::uvec4>(this->context->unifiedPipelineLayout, vk::ShaderStageFlags(VkShaderStageFlags(vkh::VkShaderStageFlags{ .eVertex = 1, .eGeometry = 1, .eFragment = 1, .eRaygen = 1, .eClosestHit = 1, .eMiss = 1 })), 0u, { meshData });
            resampleCommand.setViewport(0, { viewport });
            resampleCommand.setScissor(0, { renderArea });
            resampleCommand.draw(renderArea.extent.width, renderArea.extent.height, 0u, 0u);
            resampleCommand.endRenderPass();
            vkt::commandBarrier(resampleCommand);

            // 
            return uTHIS;
        };

        // 
        virtual uPTR(Renderer) setupCommands() { // setup Commands
            if (!this->context->refRenderPass()) {
                this->context->createRenderPass();
            };

            // 
            this->cmdbuf = vkt::createCommandBuffer(*thread, *thread);
            this->cmdbuf.copyBuffer(context->uniformRawData, context->uniformGPUData, { vk::BufferCopy(context->uniformRawData.offset(), context->uniformGPUData.offset(), context->uniformGPUData.range()) });

            // prepare meshes for ray-tracing
            for (auto& M : this->node->meshes) { M->copyBuffers(this->cmdbuf); };
            vkt::commandBarrier(this->cmdbuf);
            for (auto& M : this->node->meshes) { M->buildAccelerationStructure(this->cmdbuf); };
            vkt::commandBarrier(this->cmdbuf);

            // setup instanced and material data
            this->materials->copyBuffers(this->cmdbuf)->createDescriptorSet();
            this->node->buildAccelerationStructure(this->cmdbuf)->createDescriptorSet();

            // first-step rendering
            this->setupBackgroundPipeline()->setupSkyboxedCommand(this->cmdbuf);
            for (auto& M : this->node->meshes) { M->instanceCount = 0u; };

            // draw concurrently
            for (uint32_t i = 0; i < this->node->instanceCounter; i++) {
                const auto I = this->node->rawInstances[i].instanceId;
                this->node->meshes[I]->increaseInstanceCount(i);
            };

            // 
            auto I = 0u; for (auto& M : this->node->meshes) { 
                M->createRasterizePipeline()->createRasterizeCommand(this->cmdbuf, glm::uvec4(I++, 0u, 0u, 0u)); // FIXED FINALLY
            };
            vkt::commandBarrier(this->cmdbuf);

            // 
            this->setupResamplingPipeline()->setupResampleCommand(this->cmdbuf);
            this->setupRayTracingPipeline()->setupRayTraceCommand(this->cmdbuf); // FIXED FINALLY 

            // 
            this->cmdbuf.end();
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
        vkh::VsGraphicsPipelineCreateInfoConstruction skyboxedInfo = {};
        vkh::VsGraphicsPipelineCreateInfoConstruction pipelineInfo = {};
        vkh::VsRayTracingPipelineCreateInfoHelper rayTraceInfo = {};

        // 
        std::vector<vkh::VkPipelineShaderStageCreateInfo> skyboxStages = {};
        std::vector<vkh::VkPipelineShaderStageCreateInfo> resampStages = {};

        // 
        std::vector<vkh::VkPipelineShaderStageCreateInfo> rtStages = {};
        std::vector<vkh::VkPipelineShaderStageCreateInfo> bgStages = {};

        // 
        vk::Pipeline backgroundState = {};
        vk::Pipeline resamplingState = {};
        vk::Pipeline rayTracingState = {};

        // 
        vkt::Vector<uint64_t> gpuSBTBuffer = {};
        vkt::Vector<uint64_t> rawSBTBuffer = {};

        // 
        vkt::uni_ptr<Context> context = {};
        vkt::uni_ptr<Driver> driver = {};
        vkt::uni_ptr<Thread> thread = {};
        
        // 
        vk::PhysicalDeviceRayTracingPropertiesNV rayTracingProperties = {};
        vk::PhysicalDeviceProperties2 properties = {};

        //std::vector<vkt::uni_ptr<Instance>> instances = {};
        //std::vector<vkt::uni_ptr<Material>> materials = {};
    };

};
