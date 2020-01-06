#pragma once
#include "./config.hpp"
#include "./driver.hpp"
#include "./thread.hpp"
#include "./context.hpp"

namespace lancer {

    // TODO: Descriptor Sets
    class Renderer : public std::enable_shared_from_this<Renderer> { public: // 
        Renderer(const std::shared_ptr<Context>& context, const std::shared_ptr<Driver>& driver) {
            this->driver = driver;
            this->thread = std::make_shared<Thread>(this->driver);
            this->context = std::make_shared<Context>();

            // get ray-tracing properties
            this->properties.pNext = &this->rayTracingProperties;
            vkGetPhysicalDeviceProperties2(driver->getPhysicalDevice(), &(VkPhysicalDeviceProperties2&)this->properties);
            //driver->getPhysicalDevice().getProperties2(this->properties); // Vulkan-HPP Bugged

            // 
            const auto& rtxp = rayTracingProperties;
            this->rawSBTBuffer = vkt::Vector<uint64_t>(std::make_shared<vkt::VmaBufferAllocation>(this->driver->getAllocator(), vkh::VkBufferCreateInfo{ .size = rtxp.shaderGroupBaseAlignment*8u, .usage = { .eUniformBuffer = 1, .eRayTracing = 1 } }, VMA_MEMORY_USAGE_CPU_TO_GPU));
            this->gpuSBTBuffer = vkt::Vector<uint64_t>(std::make_shared<vkt::VmaBufferAllocation>(this->driver->getAllocator(), vkh::VkBufferCreateInfo{ .size = rtxp.shaderGroupBaseAlignment*8u, .usage = { .eUniformBuffer = 1, .eRayTracing = 1 } }, VMA_MEMORY_USAGE_GPU_ONLY));
        };

        // 
        std::shared_ptr<Renderer> setupRayTracingPipeline() {
            const std::vector<vkh::VkPipelineShaderStageCreateInfo> stages = vkt::vector_cast<vkh::VkPipelineShaderStageCreateInfo, vk::PipelineShaderStageCreateInfo>({
                vkt::makePipelineStageInfo(driver->getDevice(), vkt::readBinary("./shaders/raytrace.rgen.spv" ), vk::ShaderStageFlagBits::eRaygenNV),
                vkt::makePipelineStageInfo(driver->getDevice(), vkt::readBinary("./shaders/raytrace.rchit.spv"), vk::ShaderStageFlagBits::eClosestHitNV),
                vkt::makePipelineStageInfo(driver->getDevice(), vkt::readBinary("./shaders/raytrace.rmiss.spv"), vk::ShaderStageFlagBits::eMissNV)
            });
            
            //this->rayTraceInfo = vkh::VsRayTracingPipelineCreateInfoHelper{};
            this->rayTraceInfo.vkInfo.layout = this->context->unifiedPipelineLayout;
            this->rayTraceInfo.addShaderStages(stages);
            this->rayTracingState = driver->getDevice().createRayTracingPipelineNV(driver->getPipelineCache(),this->rayTraceInfo);

            // get ray-tracing properties
            const auto& rtxp = rayTracingProperties;

            // SBT helped for buffer
            this->driver->getDevice().getRayTracingShaderGroupHandlesNV(this->rayTracingState,0u,this->rayTraceInfo.groupCount(),this->rayTraceInfo.groupCount()*rtxp.shaderGroupBaseAlignment,this->rawSBTBuffer.data());
            
            // 
            return shared_from_this();
        };

        // 
        std::shared_ptr<Renderer> setupRayTraceCommand(const vk::CommandBuffer& rayTraceCommand = {}) { 
            // get ray-tracing properties
            const auto& rtxp = rayTracingProperties;
            
            // 
            const auto& renderArea = this->context->refScissor();
            //this->rayTraceCommand = vkt::createCommandBuffer(*thread, *thread);
            rayTraceCommand.copyBuffer(this->rawSBTBuffer, this->gpuSBTBuffer, { vk::BufferCopy(this->rawSBTBuffer.offset(),this->gpuSBTBuffer.offset(),this->rayTraceInfo.groupCount()*rtxp.shaderGroupBaseAlignment) });
            vkt::commandBarrier(rayTraceCommand);
            rayTraceCommand.bindPipeline(vk::PipelineBindPoint::eRayTracingNV, this->rayTracingState);
            rayTraceCommand.bindDescriptorSets(vk::PipelineBindPoint::eRayTracingNV, this->context->unifiedPipelineLayout, 0ull, this->context->descriptorSets, {});
            rayTraceCommand.traceRaysNV(
                this->gpuSBTBuffer, this->gpuSBTBuffer.offset(), 
                this->gpuSBTBuffer, this->gpuSBTBuffer.offset()+this->rayTraceInfo.missOffsetIndex() * rtxp.shaderGroupBaseAlignment, rtxp.shaderGroupBaseAlignment,
                this->gpuSBTBuffer, this->gpuSBTBuffer.offset()+this->rayTraceInfo.hitOffsetIndex()  * rtxp.shaderGroupBaseAlignment, rtxp.shaderGroupBaseAlignment,
                {},0u,0u,
                renderArea.extent.width, renderArea.extent.height, 1u
            );
            //rayTraceCommand.end();
            
            // 
            return shared_from_this();
        };

        // 
        std::shared_ptr<Renderer> setupResampleCommand(const vk::CommandBuffer& resampleCommand = {}) {
            const auto& viewport  = this->context->refViewport();
            const auto& renderArea = this->context->refScissor();
            const auto clearValues = std::vector<vk::ClearValue>{ 
                vk::ClearColorValue(std::array<float,4>{0.f, 0.f, 0.f, 0.0f}), 
                vk::ClearColorValue(std::array<float,4>{0.f, 0.f, 0.f, 0.0f}), 
                vk::ClearColorValue(std::array<float,4>{0.f, 0.f, 0.f, 0.0f}), 
                vk::ClearColorValue(std::array<float,4>{0.f, 0.f, 0.f, 0.0f}), 
                vk::ClearDepthStencilValue(1.0f, 0)
            };

            // 
            //resampleCommand = vkt::createCommandBuffer(*thread, *thread);
            resampleCommand.beginRenderPass(vk::RenderPassBeginInfo(this->context->refRenderPass(), this->context->samplingFramebuffer, renderArea, clearValues.size(), clearValues.data()), vk::SubpassContents::eInline);
            resampleCommand.bindPipeline(vk::PipelineBindPoint::eGraphics, this->resamplingState);
            resampleCommand.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, this->context->unifiedPipelineLayout, 0ull, this->context->descriptorSets, {});
            resampleCommand.setViewport(0, { viewport });
            resampleCommand.setScissor(0, { renderArea });
            resampleCommand.draw(renderArea.extent.width * renderArea.extent.height, 1u, 0u, 0u);
            resampleCommand.endRenderPass();
            vkt::commandBarrier(resampleCommand);
            //resampleCommand.end();

            return shared_from_this();
        };

        // 
        std::shared_ptr<Renderer> setupResamplingPipeline() {
            const auto& viewport  = this->context->refViewport();
            const auto& renderArea = this->context->refScissor();

            for (uint32_t i=0u;i<4u;i++) { // 
                this->pipelineInfo.colorBlendAttachmentStates.push_back(vkh::VkPipelineColorBlendAttachmentState{
                    .blendEnable = true,
                    .dstColorBlendFactor = VK_BLEND_FACTOR_ONE,
                    .dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE
                });
            };

            this->pipelineInfo.stages = vkt::vector_cast<vkh::VkPipelineShaderStageCreateInfo, vk::PipelineShaderStageCreateInfo>({ // 
                vkt::makePipelineStageInfo(this->driver->getDevice(), vkt::readBinary("./shaders/resample.vert.spv"), vk::ShaderStageFlagBits::eVertex),
                vkt::makePipelineStageInfo(this->driver->getDevice(), vkt::readBinary("./shaders/resample.frag.spv"), vk::ShaderStageFlagBits::eFragment)
            });

            this->pipelineInfo.depthStencilState = vkh::VkPipelineDepthStencilStateCreateInfo{
                .depthTestEnable = false,
                .depthWriteEnable = false
            };

            this->pipelineInfo.inputAssemblyState.topology = VK_PRIMITIVE_TOPOLOGY_POINT_LIST;
            this->pipelineInfo.graphicsPipelineCreateInfo.renderPass = this->context->renderPass;
            this->pipelineInfo.graphicsPipelineCreateInfo.layout = this->context->unifiedPipelineLayout;
            this->pipelineInfo.viewportState.pViewports = &(vkh::VkViewport&)viewport;
            this->pipelineInfo.viewportState.pScissors = &(vkh::VkRect2D&)renderArea;
            this->pipelineInfo.dynamicStates.push_back(VK_DYNAMIC_STATE_VIEWPORT);
            this->pipelineInfo.dynamicStates.push_back(VK_DYNAMIC_STATE_SCISSOR);
            this->resamplingState = driver->getDevice().createGraphicsPipeline(driver->getPipelineCache(),this->pipelineInfo);

            return shared_from_this();
        };

        // 
        std::shared_ptr<Renderer> setupCommands() { // Setup Commands
            this->cmdbuf = vkt::createCommandBuffer(*thread, *thread);
            this->context->createDescriptorSets();

            // prepare meshes for ray-tracing
            for (auto& M : this->instances->meshes) {
                M->buildAccelerationStructure(this->cmdbuf);
            };

            // setup instanced and material data
            this->materials->createDescriptorSet();
            this->instances->buildAccelerationStructure(this->cmdbuf)->createDescriptorSet();

            // first-step rendering
            for (auto& M : this->instances->meshes) {
                M->createRasterizePipeline()->createRasterizeCommand(this->cmdbuf);
            };

            // 
            this->setupResamplingPipeline()->setupResampleCommand(this->cmdbuf);
            this->setupRayTracingPipeline()->setupRayTraceCommand(this->cmdbuf);

            // 
            this->cmdbuf.end();
            return shared_from_this();
        };

    protected: // 
        std::vector<vk::CommandBuffer> commands = {};
        vk::CommandBuffer cmdbuf = {};
        //vk::CommandBuffer preparedCommand = {};
        //vk::CommandBuffer resampleCommand = {};
        //vk::CommandBuffer rayTraceCommand = {};
        vk::Pipeline rayTracingStage = {};
        vk::Pipeline resamplingStage = {};
        vk::Pipeline denoiseStage = {};
        vk::Pipeline compileStage = {};
        
        // binding data
        std::shared_ptr<Material> materials = {}; // materials
        std::shared_ptr<Instance> instances = {}; // instances

        // 
        vkh::VsGraphicsPipelineCreateInfoConstruction pipelineInfo = {};
        vkh::VsRayTracingPipelineCreateInfoHelper rayTraceInfo = {};

        // 
        vk::Pipeline resamplingState = {};
        vk::Pipeline rayTracingState = {};

        // 
        vkt::Vector<uint64_t> gpuSBTBuffer = {};
        vkt::Vector<uint64_t> rawSBTBuffer = {};

        // 
        std::shared_ptr<Context> context = {};
        std::shared_ptr<Driver> driver = {};
        std::shared_ptr<Thread> thread = {};
        
        // 
        vk::PhysicalDeviceRayTracingPropertiesNV rayTracingProperties = {};
        vk::PhysicalDeviceProperties2 properties = {};

        //std::vector<std::shared_ptr<Instance>> instances = {};
        //std::vector<std::shared_ptr<Material>> materials = {};
    };

};
