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
        virtual vkt::uni_ptr<Renderer> sharedPtr() { return shared_from_this(); };
        virtual vkt::uni_ptr<const Renderer> sharedPtr() const { return shared_from_this(); };

        // 
        virtual uPTR(Renderer) construct() {
            this->driver = context->getDriver();
            this->thread = std::make_shared<Thread>(this->driver);

            // get ray-tracing properties
            this->properties.pNext = &this->rayTracingProperties;
            vkGetPhysicalDeviceProperties2(driver->getPhysicalDevice(), &(VkPhysicalDeviceProperties2&)this->properties);
            //driver->getPhysicalDevice().getProperties2(this->properties); // Vulkan-HPP Bugged
            //driver->getPhysicalDevice().getProperties2(&(VkPhysicalDeviceProperties2&)this->properties);

            // Pre-Initialize Stages For FASTER CODE
            this->skyboxStages = vkt::vector_cast<vkh::VkPipelineShaderStageCreateInfo, vk::PipelineShaderStageCreateInfo>({
                vkt::makePipelineStageInfo(this->driver->getDevice(), vkt::readBinary("./shaders/rtrace/background.vert.spv"), vk::ShaderStageFlagBits::eVertex),
                vkt::makePipelineStageInfo(this->driver->getDevice(), vkt::readBinary("./shaders/rtrace/background.frag.spv"), vk::ShaderStageFlagBits::eFragment)
                });

            // 
            this->rtStages = vkt::vector_cast<vkh::VkPipelineShaderStageCreateInfo, vk::PipelineShaderStageCreateInfo>({
                vkt::makePipelineStageInfo(driver->getDevice(), vkt::readBinary("./shaders/rtrace/pathtrace.rgen.spv"), vk::ShaderStageFlagBits::eRaygenKHR),
                vkt::makePipelineStageInfo(driver->getDevice(), vkt::readBinary("./shaders/rtrace/pathtrace.rchit.spv"), vk::ShaderStageFlagBits::eClosestHitKHR),
                vkt::makePipelineStageInfo(driver->getDevice(), vkt::readBinary("./shaders/rtrace/pathtrace.rmiss.spv"), vk::ShaderStageFlagBits::eMissKHR)
                });

            // 
            this->resampStages = vkt::vector_cast<vkh::VkPipelineShaderStageCreateInfo, vk::PipelineShaderStageCreateInfo>({ // 
                vkt::makePipelineStageInfo(this->driver->getDevice(), vkt::readBinary("./shaders/rtrace/resample.vert.spv"), vk::ShaderStageFlagBits::eVertex),
                vkt::makePipelineStageInfo(this->driver->getDevice(), vkt::readBinary("./shaders/rtrace/resample.frag.spv"), vk::ShaderStageFlagBits::eFragment)
                });

            // 
            this->denoiseStage = vkt::makePipelineStageInfo(this->driver->getDevice(), vkt::readBinary("./shaders/rtrace/denoise.comp.spv"), vk::ShaderStageFlagBits::eCompute);

            return uTHIS;
        }

        // 
        virtual uPTR(Renderer) linkMaterial(vkt::uni_ptr<Material>& materials) {
            this->materials = materials;
            return uTHIS;
        };

        // 
        virtual uPTR(Renderer) linkNode(vkt::uni_ptr<Node>& node) {
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

            // 
            const auto& rtxp = rayTracingProperties;
            this->rawSBTBuffer = vkt::Vector<glm::u64vec4>(this->driver->getAllocator(), vkh::VkBufferCreateInfo{ .size = rtxp.shaderGroupHandleSize * 8u, .usage = {.eTransferSrc = 1, .eUniformBuffer = 1, .eRayTracing = 1, .eSharedDeviceAddress = 1 } }, VMA_MEMORY_USAGE_CPU_TO_GPU);
            this->gpuSBTBuffer = vkt::Vector<glm::u64vec4>(this->driver->getAllocator(), vkh::VkBufferCreateInfo{ .size = rtxp.shaderGroupHandleSize * 8u, .usage = {.eTransferDst = 1, .eUniformBuffer = 1, .eRayTracing = 1, .eSharedDeviceAddress = 1 } }, VMA_MEMORY_USAGE_GPU_ONLY);

            // create for KHR compatible and comfort
            this->rgenSBTPtr = vkt::Vector<glm::u64vec4>(this->gpuSBTBuffer.getAllocation(), this->gpuSBTBuffer.offset(), this->gpuSBTBuffer.stride());
            this->rhitSBTPtr = vkt::Vector<glm::u64vec4>(this->gpuSBTBuffer.getAllocation(), this->gpuSBTBuffer.offset()+ this->gpuSBTBuffer.stride() * this->rayTraceInfo. hitOffsetIndex(), this->gpuSBTBuffer.stride());
            this->rmisSBTPtr = vkt::Vector<glm::u64vec4>(this->gpuSBTBuffer.getAllocation(), this->gpuSBTBuffer.offset()+ this->gpuSBTBuffer.stride() * this->rayTraceInfo.missOffsetIndex(), this->gpuSBTBuffer.stride());

            // get ray-tracing properties
            this->rayTracingState = driver->getDevice().createRayTracingPipelineKHR(driver->getPipelineCache(), this->rayTraceInfo.vkInfo.hpp(), vk::AllocationCallbacks{}, this->driver->getDispatch()).value;
            this->driver->getDevice().getRayTracingShaderGroupHandlesKHR(this->rayTracingState,0u, static_cast<uint32_t>(this->rayTraceInfo.groupCount()),this->rayTraceInfo.groupCount()*rayTracingProperties.shaderGroupHandleSize,this->rawSBTBuffer.data(),this->driver->getDispatch());
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
                rasterCommand.clearColorImage(this->context->smFlip0Images[i], vk::ImageLayout::eGeneral, vk::ClearColorValue().setFloat32({ 0.f,0.f,0.f,0.f }), { this->context->smFlip0Images[i] });
                rasterCommand.clearColorImage(this->context->frameBfImages[i], vk::ImageLayout::eGeneral, vk::ClearColorValue().setFloat32({ 0.f,0.f,0.f,0.f }), { this->context->frameBfImages[i] });
            };

            rasterCommand.clearDepthStencilImage(this->context->depthImage, vk::ImageLayout::eGeneral, clearValues[8u].depthStencil, (vk::ImageSubresourceRange&)this->context->depthImage.subresourceRange);
            vkt::commandBarrier(this->cmdbuf);
            rasterCommand.beginRenderPass(vk::RenderPassBeginInfo(this->context->refRenderPass(), this->context->deferredFramebuffer, renderArea, static_cast<uint32_t>(clearValues.size()), clearValues.data()), vk::SubpassContents::eInline);
            rasterCommand.setViewport(0, { viewport });
            rasterCommand.setScissor(0, { renderArea });
            rasterCommand.pushConstants<glm::uvec4>(this->context->unifiedPipelineLayout, vk::ShaderStageFlags(VkShaderStageFlags(vkh::VkShaderStageFlags{ .eVertex = 1, .eGeometry = 1, .eFragment = 1, .eCompute = 1, .eRaygen = 1, .eClosestHit = 1, .eMiss = 1 })), 0u, { meshData });
            rasterCommand.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, this->context->unifiedPipelineLayout, 0ull, this->context->descriptorSets, {});
            rasterCommand.bindPipeline(vk::PipelineBindPoint::eGraphics, this->backgroundState);
            rasterCommand.draw(4u, 1u, 0u, 0u);
            rasterCommand.endRenderPass();
            vkt::commandBarrier(rasterCommand);

            // 
            {
                //vkh::VkComputePipelineCreateInfo denoiseInfo = {};
                //denoiseInfo.layout = this->context->unifiedPipelineLayout;
                //denoiseInfo.stage = this->denoiseStage;
                this->denoiseState = vkt::createCompute(driver->getDevice(), vkt::FixConstruction(this->denoiseStage), vk::PipelineLayout(this->context->unifiedPipelineLayout), driver->getPipelineCache());
            }

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
                //rayTraceCommand.clearColorImage(this->context->frameBfImages[i], vk::ImageLayout::eGeneral, vk::ClearColorValue().setFloat32({ 0.f,0.f,0.f,0.f }), { this->context->frameBfImages[i] });
                rayTraceCommand.clearColorImage(this->context->smFlip0Images[i], vk::ImageLayout::eGeneral, vk::ClearColorValue().setFloat32({ 0.f,0.f,0.f,0.f }), { this->context->smFlip0Images[i] });
            };

            //std::cout << rtxp.shaderGroupHandleSize << std::endl;

            vkt::commandBarrier(rayTraceCommand);
            rayTraceCommand.bindPipeline(vk::PipelineBindPoint::eRayTracingKHR, this->rayTracingState);
            rayTraceCommand.bindDescriptorSets(vk::PipelineBindPoint::eRayTracingKHR, this->context->unifiedPipelineLayout, 0ull, this->context->descriptorSets, {});
            rayTraceCommand.pushConstants<glm::uvec4>(this->context->unifiedPipelineLayout, vk::ShaderStageFlags(VkShaderStageFlags(vkh::VkShaderStageFlags{ .eVertex = 1, .eGeometry = 1, .eFragment = 1, .eCompute = 1, .eRaygen = 1, .eClosestHit = 1, .eMiss = 1 })), 0u, { meshData });
            rayTraceCommand.traceRaysKHR(this->rgenSBTPtr.getRegion(), this->rmisSBTPtr.getRegion(), this->rhitSBTPtr.getRegion(), vk::StridedBufferRegionKHR{}, renderArea.extent.width, renderArea.extent.height, 1u, this->driver->getDispatch());
            /*
                this->rgenSBTPtr.getRegion()
                this->rmisSBTPtr.getRegion(),
                this->rhitSBTPtr.getRegion(),
                {}, 0u, 0u,
                renderArea.extent.width, renderArea.extent.height, 1u,
                this->driver->getDispatch()
            );*/
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
            resampleCommand.beginRenderPass(vk::RenderPassBeginInfo(this->context->refRenderPass(), this->context->smpFlip0Framebuffer, renderArea, static_cast<uint32_t>(clearValues.size()), clearValues.data()), vk::SubpassContents::eInline);
            resampleCommand.bindPipeline(vk::PipelineBindPoint::eGraphics, this->resamplingState);
            resampleCommand.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, this->context->unifiedPipelineLayout, 0ull, this->context->descriptorSets, {});
            resampleCommand.pushConstants<glm::uvec4>(this->context->unifiedPipelineLayout, vk::ShaderStageFlags(VkShaderStageFlags(vkh::VkShaderStageFlags{ .eVertex = 1, .eGeometry = 1, .eFragment = 1, .eCompute = 1, .eRaygen = 1, .eClosestHit = 1, .eMiss = 1 })), 0u, { meshData });
            resampleCommand.setViewport(0, { viewport });
            resampleCommand.setScissor(0, { renderArea });
            resampleCommand.draw(renderArea.extent.width, renderArea.extent.height, 0u, 0u);
            resampleCommand.endRenderPass();
            vkt::commandBarrier(resampleCommand);

            // 
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
        virtual uPTR(Renderer) setupCommands() { // setup Commands
            if (!this->context->refRenderPass()) {
                this->context->createRenderPass();
            };

            // 
            this->cmdbuf = vkt::createCommandBuffer(vk::Device(*thread), vk::CommandPool(*thread));
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

            // create sampling points
            auto I = 0u;

            // make covergence (depth) map
            this->cmdbuf.clearDepthStencilImage(this->context->depthImage, vk::ImageLayout::eGeneral, vk::ClearDepthStencilValue(1.0f, 0), (vk::ImageSubresourceRange&)this->context->depthImage.subresourceRange);
            I = 0u; for (auto& M : this->node->meshes) {
                M->createRasterizePipeline()->createCovergenceCommand(this->cmdbuf, glm::uvec4(I++, 0u, 0u, 0u));
                //M->createRasterizePipeline()->createRasterizeCommand(this->cmdbuf, glm::uvec4(I++, 0u, 0u, 0u));
            };
            vkt::commandBarrier(this->cmdbuf);

            // 
            this->cmdbuf.clearDepthStencilImage(this->context->depthImage, vk::ImageLayout::eGeneral, vk::ClearDepthStencilValue(1.0f, 0), (vk::ImageSubresourceRange&)this->context->depthImage.subresourceRange);
            I = 0u; for (auto& M : this->node->meshes) {
                M->createRasterizeCommand(this->cmdbuf, glm::uvec4(I++, 0u, 0u, 0u));
                //M->createCovergenceCommand(this->cmdbuf, glm::uvec4(I++, 0u, 0u, 0u));
            };
            vkt::commandBarrier(this->cmdbuf);

            // 
            this->setupResamplingPipeline()->setupResampleCommand(this->cmdbuf);
            this->setupRayTracingPipeline()->setupRayTraceCommand(this->cmdbuf); // FIXED FINALLY 
            //this->saveDiffuseColor(this->cmdbuf);

            //
            const auto& viewport = this->context->refViewport();
            const auto& renderArea = this->context->refScissor();

            // 
            this->cmdbuf.bindDescriptorSets(vk::PipelineBindPoint::eCompute, this->context->unifiedPipelineLayout, 0ull, this->context->descriptorSets, {});
            this->cmdbuf.bindPipeline(vk::PipelineBindPoint::eCompute, this->denoiseState);
            this->cmdbuf.pushConstants<glm::uvec4>(this->context->unifiedPipelineLayout, vkh::VkShaderStageFlags{.eVertex = 1, .eGeometry = 1, .eFragment = 1, .eCompute = 1, .eRaygen = 1, .eClosestHit = 1, .eMiss = 1 }.hpp(), 0u, { glm::uvec4(0u) });
            this->cmdbuf.dispatch(vkt::tiled(renderArea.extent.width,16u), vkt::tiled(renderArea.extent.height,12u), 1u);
            vkt::commandBarrier(this->cmdbuf);

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
        vkh::VkPipelineShaderStageCreateInfo denoiseStage = {};

        // 
        std::vector<vkh::VkPipelineShaderStageCreateInfo> rtStages = {};
        std::vector<vkh::VkPipelineShaderStageCreateInfo> bgStages = {};

        // 
        vk::Pipeline backgroundState = {};
        vk::Pipeline resamplingState = {};
        vk::Pipeline rayTracingState = {};
        vk::Pipeline denoiseState = {};

        // 
        vkt::Vector<glm::u64vec4> rgenSBTPtr = {};
        vkt::Vector<glm::u64vec4> rhitSBTPtr = {};
        vkt::Vector<glm::u64vec4> rmisSBTPtr = {};

        vkt::Vector<glm::u64vec4> gpuSBTBuffer = {};
        vkt::Vector<glm::u64vec4> rawSBTBuffer = {};

        // 
        vkt::uni_ptr<Context> context = {};
        vkt::uni_ptr<Driver> driver = {};
        vkt::uni_ptr<Thread> thread = {};
        
        // 
        vk::PhysicalDeviceRayTracingPropertiesKHR rayTracingProperties = {};
        vk::PhysicalDeviceProperties2 properties = {};

        //std::vector<vkt::uni_ptr<Instance>> instances = {};
        //std::vector<vkt::uni_ptr<Material>> materials = {};
    };

};
