#pragma once
#include "./config.hpp"
#include "./driver.hpp"
#include "./thread.hpp"
#include "./context.hpp"

namespace lancer {
    class Renderer : public std::enable_shared_from_this<Renderer> { public: 
        Renderer(const std::shared_ptr<Driver>& driver) {
            this->driver = driver;
            this->thread = std::make_shared<Thread>(this->driver);
            this->context = std::make_shared<Context>();

            // get ray-tracing properties
            auto  prop = driver->getPhysicalDevice().getProperties2<vk::PhysicalDeviceRayTracingPropertiesNV>();
            auto& rtxp = *(vk::PhysicalDeviceRayTracingPropertiesNV*)prop.pNext;

            // 
            this->rawSBTBuffer = vkt::Vector<uint64_t>(std::make_shared<vkt::VmaBufferAllocation>(this->driver->getAllocator(), vkh::VkBufferCreateInfo{ .size = rtxp.shaderGroupBaseAlignment*8u, .usage = { .eUniformBuffer = 1, .eRayTracing = 1 } }, VMA_MEMORY_USAGE_CPU_TO_GPU));
            this->gpuSBTBuffer = vkt::Vector<uint64_t>(std::make_shared<vkt::VmaBufferAllocation>(this->driver->getAllocator(), vkh::VkBufferCreateInfo{ .size = rtxp.shaderGroupBaseAlignment*8u, .usage = { .eUniformBuffer = 1, .eRayTracing = 1 } }, VMA_MEMORY_USAGE_GPU_ONLY));
        };

        // 
        std::shared_ptr<Renderer> setupRayTracingStages() {
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
            auto  prop = driver->getPhysicalDevice().getProperties2<vk::PhysicalDeviceRayTracingPropertiesNV>();
            auto& rtxp = *(vk::PhysicalDeviceRayTracingPropertiesNV*)prop.pNext;

            // SBT helped for buffer
            this->driver->getDevice().getRayTracingShaderGroupHandlesNV(this->rayTracingState,0u,this->rayTraceInfo.groupCount(),this->rayTraceInfo.groupCount()*rtxp.shaderGroupBaseAlignment,this->rawSBTBuffer.data());

            // 
            return shared_from_this();
        };

        // 
        std::shared_ptr<Renderer> setupRayTraceCommand(){
            
            // get ray-tracing properties
            auto  prop = driver->getPhysicalDevice().getProperties2<vk::PhysicalDeviceRayTracingPropertiesNV>();
            auto& rtxp = *(vk::PhysicalDeviceRayTracingPropertiesNV*)prop.pNext;

            // 
            const auto& renderArea = this->context->refScissor();
            this->rayTraceCommand = vkt::createCommandBuffer(*thread, *thread, true, false);
            this->rayTraceCommand.copyBuffer(this->rawSBTBuffer, this->gpuSBTBuffer, { vk::BufferCopy(this->rawSBTBuffer.offset(),this->gpuSBTBuffer.offset(),this->rayTraceInfo.groupCount()*rtxp.shaderGroupBaseAlignment) });
            vkt::commandBarrier(this->rayTraceCommand);
            this->rayTraceCommand.bindPipeline(vk::PipelineBindPoint::eRayTracingNV, this->rayTracingState);
            this->rayTraceCommand.traceRaysNV(
                this->gpuSBTBuffer, this->gpuSBTBuffer.offset(), 
                this->gpuSBTBuffer, this->gpuSBTBuffer.offset()+this->rayTraceInfo.missOffsetIndex() * rtxp.shaderGroupBaseAlignment, rtxp.shaderGroupBaseAlignment,
                this->gpuSBTBuffer, this->gpuSBTBuffer.offset()+this->rayTraceInfo.hitOffsetIndex()  * rtxp.shaderGroupBaseAlignment, rtxp.shaderGroupBaseAlignment,
                {},0u,0u,
                renderArea.extent.width, renderArea.extent.height, 1u
            );
            this->rayTraceCommand.end();

            // 
            return shared_from_this();
        };

        // 
        std::shared_ptr<Renderer> setupResamplingStages() {
            const auto& viewport  = this->context->refViewport();
            const auto& renderArea = this->context->refScissor();

            for (uint32_t i=0u;i<4u;i++) {
                this->pipelineInfo.colorBlendAttachmentStates.push_back(vkh::VkPipelineColorBlendAttachmentState{
                    .blendEnable = true,
                    .dstColorBlendFactor = VK_BLEND_FACTOR_ONE,
                    .dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE
                });
            };

            this->pipelineInfo.stages = vkt::vector_cast<vkh::VkPipelineShaderStageCreateInfo, vk::PipelineShaderStageCreateInfo>({
                vkt::makePipelineStageInfo(this->driver->getDevice(), vkt::readBinary("./shaders/resample.vert.spv"), vk::ShaderStageFlagBits::eVertex),
                vkt::makePipelineStageInfo(this->driver->getDevice(), vkt::readBinary("./shaders/resample.frag.spv"), vk::ShaderStageFlagBits::eFragment)
            });

            this->pipelineInfo.depthStencilState = vkh::VkPipelineDepthStencilStateCreateInfo{
                .depthTestEnable = false,
                .depthWriteEnable = false
            };

            this->pipelineInfo.graphicsPipelineCreateInfo.renderPass = this->context->renderPass;
            this->pipelineInfo.graphicsPipelineCreateInfo.layout = this->context->unifiedPipelineLayout;
            this->pipelineInfo.viewportState.pViewports = &(vkh::VkViewport&)viewport;
            this->pipelineInfo.viewportState.pScissors = &(vkh::VkRect2D&)renderArea;
            this->resamplingState = driver->getDevice().createGraphicsPipeline(driver->getPipelineCache(),this->pipelineInfo);

            return shared_from_this();
        };

    protected: // 
        vk::CommandBuffer deferredCommand = {};
        vk::CommandBuffer rayTraceCommand = {};
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
        std::vector<std::shared_ptr<Instance>> instances = {};
        std::vector<std::shared_ptr<Material>> materials = {};
    };

};
