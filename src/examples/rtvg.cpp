#pragma once

#define TINYGLTF_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#define VKT_RENDERER_IMPLEMENTATION
#define VMA_IMPLEMENTATION
#define RADX_IMPLEMENTATION


#undef small
#define small char
#undef small
#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include "../base/appRenderer.hpp"
#undef small

#include "../../include/vkt/fw.hpp"
#include <glm/gtc/matrix_transform.hpp>

namespace rnd {

    // 
    const uint32_t blockWidth = 8, blockheight = 8;

    // 
    void Renderer::Arguments(int argc, char** argv) {
        args::ArgumentParser parser("This is a test rendering program.", "");
        args::HelpFlag help(parser, "help", "Available flags", { 'h', "help" });
        args::ValueFlag<int32_t> computeflag(parser, "compute-device-id", "Vulkan compute device (UNDER CONSIDERATION)", { 'c' });
        args::ValueFlag<int32_t> deviceflag(parser, "graphics-device-id", "Vulkan graphics device to use (also should support compute)", { 'g' });
        args::ValueFlag<float> scaleflag(parser, "scale", "Scaling of model object", { 's' });
        args::ValueFlag<std::string> directoryflag(parser, "directory", "Directory of resources", { 'd' });
        args::ValueFlag<std::string> shaderflag(parser, "shaders", "Used SPIR-V shader pack", { 'p' });
        args::ValueFlag<std::string> bgflag(parser, "background", "Environment background", { 'b' });
        args::ValueFlag<std::string> modelflag(parser, "model", "Model to view (planned multiple models support)", { 'm' });
        args::Flag forceCompute(parser, "force-compute", "Force enable compute shader backend", { 'F' });

        args::ValueFlag<int32_t> reflLV(parser, "reflection-level", "Level of reflections", { 'R' });
        args::ValueFlag<int32_t> trnsLV(parser, "transparency-level", "Level of transparency", { 'T' });

        /* TESTING
        try { parser.ParseCLI(argc, argv); }
        catch (args::Help) { std::cout << parser; glfwTerminate(); exit(1); };

        // read arguments
        if (deviceflag) gpuID = args::get(deviceflag);
        if (shaderflag) shaderPrefix = args::get(shaderflag);
        */
    };

    // 
    void Renderer::Init(uint32_t windowWidth, uint32_t windowHeight, bool enableSuperSampling) {
        // create GLFW window
        this->windowWidth = windowWidth, this->windowHeight = windowHeight;
        this->window = glfwCreateWindow(windowWidth, windowHeight, "vRt early test", NULL, NULL);
        if (!this->window) { glfwTerminate(); exit(EXIT_FAILURE); }

        // get DPI
        glfwGetWindowContentScale(this->window, &guiScale, nullptr);
        glfwSetWindowSize(this->window, this->realWidth = windowWidth * guiScale, this->realHeight = windowHeight * guiScale); // set real size of window

        // create vulkan and ray tracing instance
        appBase = std::make_shared<vkt::GPUFramework>();

        // create VK instance
        auto instance = appBase->createInstance();
        if (!instance) { glfwTerminate(); exit(EXIT_FAILURE); }

        // create surface and get format by physical device
        auto gpu = appBase->getPhysicalDevice(gpuID);
        appBase->createWindowSurface(this->window, this->realWidth, this->realHeight, title);
        appBase->format(appBase->getSurfaceFormat(gpu));

        // write physical device name
        vk::PhysicalDeviceProperties devProperties = gpu.getProperties();
        std::cout << "Current Device: ";
        std::cout << devProperties.deviceName << std::endl;
        std::cout << devProperties.vendorID << std::endl;

        // create combined device object
        // TODO: Vendor Dependent Shaders
        shaderPack = shaderPrefix + "intrusive/turing";

        // create radix sort application (RadX C++)
        physicalHelper = std::make_shared<lancer::PhysicalDevice_T>(instance, appBase->getPhysicalDevice(0));
        device = appBase->createDevice(false, shaderPack, true);

        // create image output
        const auto SuperSampling = enableSuperSampling ? 2.0 : 1.0; // super sampling image
        this->canvasWidth = this->windowWidth * SuperSampling;
        this->canvasHeight = this->windowHeight * SuperSampling;

        // create framebuffers 
        framebuffers = appBase->createSwapchainFramebuffer(swapchain = appBase->createSwapchain(), appBase->createRenderPass());
    };

    // 
    void Renderer::UpdateFramebuffers(uint32_t width, uint32_t height) {
        device->least().waitIdle();
        appBase->updateSwapchainFramebuffer(framebuffers, swapchain, appBase->renderPass);
    };

    // 
    void Renderer::InitPipeline() {

        {   // == Pinning Lake == 
            mUnifiedDescriptorLayout = device->createDescriptorSetLayoutMaker(vk::DescriptorSetLayoutCreateInfo(), &unifiedDescriptorLayout)
                ->pushBinding(vk::DescriptorSetLayoutBinding(1, vk::DescriptorType::eAccelerationStructureNV, 1, vk::ShaderStageFlagBits::eClosestHitNV | vk::ShaderStageFlagBits::eAnyHitNV | vk::ShaderStageFlagBits::eRaygenNV), vk::DescriptorBindingFlagBitsEXT::ePartiallyBound | vk::DescriptorBindingFlagBitsEXT::eUpdateUnusedWhilePending) // Acceleration Structure
                ->pushBinding(vk::DescriptorSetLayoutBinding(0, vk::DescriptorType::eSampledImage, 1, vk::ShaderStageFlagBits::eAll), vk::DescriptorBindingFlagBitsEXT::ePartiallyBound |  vk::DescriptorBindingFlagBitsEXT::eUpdateUnusedWhilePending) // Sampling Images from Render Passes (Samples, Hi-Z, Colors, Normals, Diffuses)
                ->pushBinding(vk::DescriptorSetLayoutBinding(2, vk::DescriptorType::eUniformBuffer, 1, vk::ShaderStageFlagBits::eAll), vk::DescriptorBindingFlagBitsEXT::ePartiallyBound | vk::DescriptorBindingFlagBitsEXT::eUpdateUnusedWhilePending) // Constant-Based Dynamic Buffer
                ->pushBinding(vk::DescriptorSetLayoutBinding(3, vk::DescriptorType::eStorageBuffer, 1, vk::ShaderStageFlagBits::eAll), vk::DescriptorBindingFlagBitsEXT::ePartiallyBound | vk::DescriptorBindingFlagBitsEXT::eUpdateUnusedWhilePending) // Attribute Data (for Ray-Tracers or Unified Rasterizers)
                ->pushBinding(vk::DescriptorSetLayoutBinding(4, vk::DescriptorType::eStorageImage, 1, vk::ShaderStageFlagBits::eAll), vk::DescriptorBindingFlagBitsEXT::ePartiallyBound | vk::DescriptorBindingFlagBitsEXT::eUpdateUnusedWhilePending)  // Writable Output Images
                ->create();

            // 
            mUnifiedPipelineLayout = device->createPipelineLayoutMaker(vk::PipelineLayoutCreateInfo(), &unifiedPipelineLayout);
            mUnifiedPipelineLayout->pushDescriptorSetLayout(unifiedDescriptorLayout)->create();
        };

        {   // == Ray Tracing Pipeline == 
            // Create Ray Tracing Pipeline and SBT
            mRaytracedPipeline = device->createSBTHelper(api::RayTracingPipelineCreateInfoNV(), &raytracedPipeline)->linkBuffer(&rtSBT)->linkPipelineLayout(&unifiedPipelineLayout)->initialize();
            mRaytracedPipeline->setRaygenStage(vk::PipelineShaderStageCreateInfo().setModule(lancer::createShaderModule(*device, lancer::readBinary(shaderPack + "/rtrace/rtrace.rgen.spv"))).setPName("main").setStage(vk::ShaderStageFlagBits::eRaygenNV));
            mRaytracedPipeline->addStageToHitGroup(vk::PipelineShaderStageCreateInfo().setModule(lancer::createShaderModule(*device, lancer::readBinary(shaderPack + "/rtrace/handle.rchit.spv"))).setPName("main").setStage(vk::ShaderStageFlagBits::eClosestHitNV));
            mRaytracedPipeline->addStageToMissGroup(vk::PipelineShaderStageCreateInfo().setModule(lancer::createShaderModule(*device, lancer::readBinary(shaderPack + "/rtrace/bgfill.rmiss.spv"))).setPName("main").setStage(vk::ShaderStageFlagBits::eMissNV));
            mRaytracedPipeline->create();
        };

        {   // == Acceleration Structures (RTX Only) == 
            rtAccelTop = vkt::AccelerationInstanced(device, vk::AccelerationStructureCreateInfoNV());

            // TODO : Real Geometry Loader
            for (uint32_t i = 0u; i < 1u; i++) {
                indices = {0u,1u,2u};
                vertices = { glm::vec4(1.f,-1.f,1.f,1.f),glm::vec4(-1.f,-1.f,1.f,1.f),glm::vec4(0.f,1.f,1.f,1.f) };
                transform = { glm::mat3x4(1.f) };

                // 
                mGeometry = vkt::GeometryBuffer<uint32_t, glm::vec4>(device, 1024u);
                uTransform = vkt::BufferUploader<glm::mat3x4>(device, &mGeometry.mTransform, &transform);
                uVertices = vkt::BufferUploader<glm::vec4>(device, &mGeometry.mVertices, &vertices);
                uIndices = vkt::BufferUploader<uint32_t>(device, &mGeometry.mIndices, &indices);

                // Allocate Geometry
                mGeometry.allocate();

                // 
                lancer::submitOnce(*device, appBase->queue, appBase->commandPool, [&](vk::CommandBuffer& cmd) {
                    uTransform.uploadCmd(cmd);
                    uVertices.uploadCmd(cmd);
                    uIndices.uploadCmd(cmd);
                });

                // Construct Low Acceleration Structure
                rtAccelLow = vkt::AccelerationGeometry(device);
                rtAccelLow.pGeometry(mGeometry).allocate();

                // Upload And Create Cmd
                lancer::submitOnce(*device, appBase->queue, appBase->commandPool, [&](vk::CommandBuffer& cmd) {
                    rtAccelLow.updateCmd(cmd);
                });


                // == TOP LEVEL UPLOAD == 
                lancer::GeometryInstance instance = {};
                instance.transform = glm::mat3x4(1.f);
                instance.instanceId = static_cast<uint32_t>(i);
                instance.mask = 0xff;
                instance.instanceOffset = 0;
                instance.flags = uint32_t(vk::GeometryInstanceFlagBitsNV::eTriangleCullDisable); // TODO: Better Type
                rtAccelTop.pushGeometry(rtAccelLow, instance).allocate();
            };

            // Upload And Create Cmd
            lancer::submitOnce(*device, appBase->queue, appBase->commandPool, [&](vk::CommandBuffer& cmd) {
                rtAccelTop.updateCmd(cmd);
            });

        };

        {   // == Final Rasterization Shader (Planned To Replace) == 
            mFinalDrawPipeline = device->createGraphicsPipelineMaker(vk::GraphicsPipelineCreateInfo(),&finalDrawPipeline);
            mFinalDrawPipeline->getInputAssemblyState().setTopology(vk::PrimitiveTopology::eTriangleStrip);
            mFinalDrawPipeline->pushShaderModule(vk::PipelineShaderStageCreateInfo().setModule(lancer::createShaderModule(*device,lancer::readBinary(shaderPack + "/render/render.vert.spv"))).setPName("main").setStage(vk::ShaderStageFlagBits::eVertex));
            mFinalDrawPipeline->pushShaderModule(vk::PipelineShaderStageCreateInfo().setModule(lancer::createShaderModule(*device,lancer::readBinary(shaderPack + "/render/render.frag.spv"))).setPName("main").setStage(vk::ShaderStageFlagBits::eFragment));
            mFinalDrawPipeline->pushDynamicState(vk::DynamicState::eViewport)->pushDynamicState(vk::DynamicState::eScissor);
            mFinalDrawPipeline->getScissor().setExtent(api::Extent2D{ this->canvasWidth, this->canvasHeight });
            mFinalDrawPipeline->getViewport().setWidth(this->canvasWidth).setHeight(this->canvasHeight);
            mFinalDrawPipeline->link(&finalDrawPipeline)->linkPipelineLayout(&unifiedPipelineLayout)->linkRenderPass(&appBase->renderPass)->create(true);
        };

        {   // == Output Image Initialization And Binding == 
            // Should Live Before Allocation, BUT NOT ERASE BEFORE ALLOCATION
            VmaAllocationCreateInfo vmac = {};
            vmac.usage = VMA_MEMORY_USAGE_GPU_ONLY;
            vmac.flags = {};

            // Create output image
            mOutputImage = device->createImageMaker(api::ImageCreateInfo().setFormat(vk::Format::eR32G32B32A32Sfloat).setUsage(vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eStorage));
            mOutputImage->link(&outputImage)->create2D(vk::Format::eR32G32B32A32Sfloat, appBase->applicationWindow.surfaceSize.width, appBase->applicationWindow.surfaceSize.height)->allocate((uintptr_t)(&vmac));
            lancer::submitOnce(*device, appBase->queue, appBase->commandPool, [&](vk::CommandBuffer& cmd) { mOutputImage->imageBarrier(cmd); });

            // Create sampler and description
            //vk::SamplerCreateInfo samplerInfo = {};
            //samplerInfo.addressModeU = vk::SamplerAddressMode::eClampToEdge;
            //samplerInfo.addressModeV = vk::SamplerAddressMode::eClampToEdge;
            //samplerInfo.minFilter = vk::Filter::eLinear;
            //samplerInfo.magFilter = vk::Filter::eLinear;
            //samplerInfo.compareEnable = false;
            //auto sampler = device->createSamplerMaker(samplerInfo);
            mFinalizedDescriptorSet = device->createDescriptorSet(vk::DescriptorSetAllocateInfo(), &finalizedDescriptorSet)->linkLayout(&unifiedDescriptorLayout);
            mRayTracedDescriptorSet = device->createDescriptorSet(vk::DescriptorSetAllocateInfo(), &rayTracedDescriptorSet)->linkLayout(&unifiedDescriptorLayout);
        };

        {   // == Finalize Descriptor Set == 
            // Create and apply descriptor set 
            auto dOutputImage = mFinalizedDescriptorSet->addImageDesc(4, 0, 1, false);
            mOutputImage->createImageView(&dOutputImage->imageView, api::ImageViewType::e2D, vk::Format::eR32G32B32A32Sfloat);
            dOutputImage->imageLayout = mOutputImage->getTargetLayout();

            // Apply Descriptor Set
            mFinalizedDescriptorSet->create()->apply();
        };

        {   // == Ray Tracing Descriptor Set ==
            // Create and apply descriptor set 
            auto dOutputImage = mRayTracedDescriptorSet->addImageDesc(4, 0, 1, false);
            mOutputImage->createImageView(&dOutputImage->imageView, api::ImageViewType::e2D, vk::Format::eR32G32B32A32Sfloat);
            dOutputImage->imageLayout = mOutputImage->getTargetLayout();

            // Add Ray Tracing Object
            rtAccelTop->writeForDescription(mRayTracedDescriptorSet->addAccelerationStructureDesc(1, 0, 1));

            // Apply Descriptor Set
            mRayTracedDescriptorSet->create()->apply();
        };
    };

    // 
    void Renderer::Draw() {
        auto n_semaphore = currSemaphore;
        auto c_semaphore = int32_t((size_t(currSemaphore) + 1ull) % framebuffers.size());
        currSemaphore = c_semaphore;

        // acquire next image where will rendered (and get semaphore when will presented finally)
        n_semaphore = (n_semaphore >= 0 ? n_semaphore : (framebuffers.size() - 1));
        device->least().acquireNextImageKHR(swapchain, std::numeric_limits<uint64_t>::max(), framebuffers[n_semaphore].semaphore, nullptr, &currentBuffer);

        { // Submit rendering (and wait presentation in device)
            std::vector<vk::ClearValue> clearValues = { vk::ClearColorValue(std::array<float,4>{1.f, 1.f, 1.f, 1.0f}), vk::ClearDepthStencilValue(1.0f, 0) };
            auto renderArea = vk::Rect2D(vk::Offset2D(0, 0), appBase->size());
            auto viewport = vk::Viewport(0.0f, 0.0f, appBase->size().width, appBase->size().height, 0, 1.0f);

            // Ray-Trace Command 
            lancer::submitOnce(*device, appBase->queue, appBase->commandPool, [&](vk::CommandBuffer& cmd) {
                cmd.bindDescriptorSets(vk::PipelineBindPoint::eRayTracingNV, unifiedPipelineLayout, 0u, rayTracedDescriptorSet, nullptr);
                cmd.bindPipeline(api::PipelineBindPoint::eRayTracingNV, raytracedPipeline);
                cmd.traceRaysNV(
                    mRaytracedPipeline->getSBTBuffer(), 0u,                                                                  // Ray-Gen Groups
                    mRaytracedPipeline->getSBTBuffer(), mRaytracedPipeline->getMissGroupsOffset(), mRaytracedPipeline->getGroupsStride(),  // Miss Groups
                    mRaytracedPipeline->getSBTBuffer(), mRaytracedPipeline->getHitGroupsOffset(), mRaytracedPipeline->getGroupsStride(),   // Hit Groups
                    {}, 0u, 0u,                                                                                       // Callable Groups
                    this->canvasWidth, this->canvasHeight, 1u,                                                        // Dispatch
                    this->device->getDispatcher()                                                                     // RTX Extension Issue FIX
                );
                lancer::commandBarrier(cmd);
            });

            // CSreate command buffer (with rewrite)
            vk::CommandBuffer& commandBuffer = framebuffers[n_semaphore].commandBuffer;
            if (!commandBuffer) {
                commandBuffer = lancer::createCommandBuffer(*device, appBase->commandPool, false, false); // do reference of cmd buffer
                commandBuffer.beginRenderPass(vk::RenderPassBeginInfo(appBase->renderPass, framebuffers[currentBuffer].frameBuffer, renderArea, clearValues.size(), clearValues.data()), vk::SubpassContents::eInline);
                commandBuffer.setViewport(0, std::vector<vk::Viewport> { viewport });
                commandBuffer.setScissor(0, std::vector<vk::Rect2D> { renderArea });
                commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, finalDrawPipeline);
                commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, unifiedPipelineLayout, 0, finalizedDescriptorSet, nullptr);
                commandBuffer.draw(4, 1, 0, 0);
                commandBuffer.endRenderPass();
                commandBuffer.end();
            };

            // Create render submission 
            std::vector<vk::Semaphore>
                waitSemaphores = { framebuffers[n_semaphore].semaphore },
                signalSemaphores = { framebuffers[c_semaphore].semaphore };
            std::vector<vk::PipelineStageFlags> waitStages = { vk::PipelineStageFlagBits::eColorAttachmentOutput };

            // 
            std::array<vk::CommandBuffer, 1> XPEHb = { commandBuffer };

            // Submit command once
            lancer::submitCmd(*device, appBase->queue, { commandBuffer }, vk::SubmitInfo()
                    .setPCommandBuffers(XPEHb.data()).setCommandBufferCount(XPEHb.size())
                    .setPWaitDstStageMask(waitStages.data()).setPWaitSemaphores(waitSemaphores.data()).setWaitSemaphoreCount(waitSemaphores.size())
                    .setPSignalSemaphores(signalSemaphores.data()).setSignalSemaphoreCount(signalSemaphores.size()));

        };

        // present for displaying of this image
        vk::Queue(appBase->queue).presentKHR(vk::PresentInfoKHR(
            1, &framebuffers[c_semaphore].semaphore,
            1, &swapchain,
            &currentBuffer, nullptr
        ));
    };

    // 
    void Renderer::HandleData() {
        const auto tFrameTime = glfwGetTime();
        const auto tDiff = tFrameTime - tPastFrameTime, tFPS = 1.0 / tDiff; // get computed time difference ( TODO: rounding tFPS )
        std::stringstream tFrameTimeStream{ "" }; tFrameTimeStream << std::fixed << std::setprecision(2) << (tDiff*1000.0);
        std::stringstream tFrameRateStream{ "" }; tFrameRateStream << std::fixed << std::setprecision(2) << tFPS;
        tPastFrameTime = tFrameTime;

        auto wTitle = "rtvg : " + tFrameTimeStream.str() + "ms / " + tFrameRateStream.str() + "fps";
        glfwSetWindowTitle(window, wTitle.c_str());
    };



};
