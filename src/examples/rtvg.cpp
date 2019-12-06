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

                // Allocate Geometry Buffers
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
                rtAccelTop.pushGeometry(rtAccelLow, instance);
            };

            // Upload And Create Cmd
            lancer::submitOnce(*device, appBase->queue, appBase->commandPool, [&](vk::CommandBuffer& cmd) {
                rtAccelTop.allocate();
                rtAccelTop.updateCmd(cmd);
            });

        };

        {   // == Pinning Lake == 
            mUnifiedDescriptorLayout = device->createDescriptorSetLayoutMaker(vk::DescriptorSetLayoutCreateInfo(), &unifiedDescriptorLayout)
                ->pushBinding(vk::DescriptorSetLayoutBinding(0u, vk::DescriptorType::eUniformBuffer, 1u, vk::ShaderStageFlagBits::eAll), vk::DescriptorBindingFlagBitsEXT::ePartiallyBound | vk::DescriptorBindingFlagBitsEXT::eUpdateUnusedWhilePending) // Constant-Based Dynamic Buffer
                ->pushBinding(vk::DescriptorSetLayoutBinding(1u, vk::DescriptorType::eAccelerationStructureNV, 1u, vk::ShaderStageFlagBits::eClosestHitNV | vk::ShaderStageFlagBits::eAnyHitNV | vk::ShaderStageFlagBits::eRaygenNV), vk::DescriptorBindingFlagBitsEXT::ePartiallyBound | vk::DescriptorBindingFlagBitsEXT::eUpdateUnusedWhilePending) // Acceleration Structure
                ->pushBinding(vk::DescriptorSetLayoutBinding(2u, vk::DescriptorType::eCombinedImageSampler, 16u, vk::ShaderStageFlagBits::eAll), vk::DescriptorBindingFlagBitsEXT::ePartiallyBound | vk::DescriptorBindingFlagBitsEXT::eUpdateUnusedWhilePending) // Sampling Images from Render Passes (Samples, Hi-Z, Colors, Normals, Diffuses)
                ->pushBinding(vk::DescriptorSetLayoutBinding(3u, vk::DescriptorType::eStorageBuffer, 16, vk::ShaderStageFlagBits::eAll), vk::DescriptorBindingFlagBitsEXT::ePartiallyBound | vk::DescriptorBindingFlagBitsEXT::eUpdateUnusedWhilePending) // Attribute Data (for Ray-Tracers or Unified Rasterizers)
                ->pushBinding(vk::DescriptorSetLayoutBinding(4u, vk::DescriptorType::eStorageImage, 16u, vk::ShaderStageFlagBits::eAll), vk::DescriptorBindingFlagBitsEXT::ePartiallyBound | vk::DescriptorBindingFlagBitsEXT::eUpdateUnusedWhilePending)  // Writable Output Images
                ->pushBinding(vk::DescriptorSetLayoutBinding(5u, vk::DescriptorType::eSampledImage, 256u, vk::ShaderStageFlagBits::eAll), vk::DescriptorBindingFlagBitsEXT::ePartiallyBound | vk::DescriptorBindingFlagBitsEXT::eUpdateUnusedWhilePending)  // Textures 
                ->pushBinding(vk::DescriptorSetLayoutBinding(6u, vk::DescriptorType::eSampler, 64u, vk::ShaderStageFlagBits::eAll), vk::DescriptorBindingFlagBitsEXT::ePartiallyBound | vk::DescriptorBindingFlagBitsEXT::eUpdateUnusedWhilePending)  // Samplers 
                ->create();

            // 
            mUnifiedPipelineLayout = device->createPipelineLayoutMaker(vk::PipelineLayoutCreateInfo(), &unifiedPipelineLayout);
            mUnifiedPipelineLayout->pushDescriptorSetLayout(unifiedDescriptorLayout)->create();
        };

        {   // == Output Image Initialization And Binding == 
            // Should Live Before Allocation, BUT NOT ERASE BEFORE ALLOCATION
            VmaAllocationCreateInfo vmac = {};
            vmac.usage = VMA_MEMORY_USAGE_GPU_ONLY;
            vmac.flags = {};

            // Create Depth Images
            mDepthStBuffer = device->createImageMaker(api::ImageCreateInfo().setFormat(vk::Format::eD32SfloatS8Uint).setUsage(vk::ImageUsageFlagBits::eDepthStencilAttachment | api::ImageUsageFlagBits::eTransferSrc), &depthStBuffer)
                ->setImageSubresourceRange(api::ImageSubresourceRange{ api::ImageAspectFlagBits::eDepth | api::ImageAspectFlagBits::eStencil, 0, 1, 0, 1 })
                ->create2D(vk::Format::eD32SfloatS8Uint, this->canvasWidth, this->canvasHeight)
                ->allocate((uintptr_t)(&vmac));

            // Create Single Images
            mOutputsBuffer = device->createImageMaker(api::ImageCreateInfo().setFormat(vk::Format::eR32G32B32A32Sfloat).setUsage(vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eStorage | vk::ImageUsageFlagBits::eColorAttachment));
            mOutputsBuffer->link(&outputsBuffer)->create2D(vk::Format::eR32G32B32A32Sfloat, appBase->applicationWindow.surfaceSize.width, appBase->applicationWindow.surfaceSize.height)->allocate(uintptr_t(&vmac));

            // 
            mColoredBuffer = device->createImageMaker(api::ImageCreateInfo().setFormat(vk::Format::eR32G32B32A32Sfloat).setUsage(vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eStorage | vk::ImageUsageFlagBits::eColorAttachment));
            mColoredBuffer->link(&coloredBuffer)->create2D(vk::Format::eR32G32B32A32Sfloat, appBase->applicationWindow.surfaceSize.width, appBase->applicationWindow.surfaceSize.height)->allocate(uintptr_t(&vmac));

            // 
            mDenoiseBuffer = device->createImageMaker(api::ImageCreateInfo().setFormat(vk::Format::eR32G32B32A32Sfloat).setUsage(vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eStorage | vk::ImageUsageFlagBits::eColorAttachment));
            mDenoiseBuffer->link(&denoiseBuffer)->create2D(vk::Format::eR32G32B32A32Sfloat, appBase->applicationWindow.surfaceSize.width, appBase->applicationWindow.surfaceSize.height)->allocate(uintptr_t(&vmac));

            // 
            mNormalsBuffer = device->createImageMaker(api::ImageCreateInfo().setFormat(vk::Format::eR32G32B32A32Sfloat).setUsage(vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eStorage | vk::ImageUsageFlagBits::eColorAttachment));
            mNormalsBuffer->link(&normalsBuffer)->create2D(vk::Format::eR32G32B32A32Sfloat, appBase->applicationWindow.surfaceSize.width, appBase->applicationWindow.surfaceSize.height)->allocate(uintptr_t(&vmac));

            // 
            mNormmodBuffer = device->createImageMaker(api::ImageCreateInfo().setFormat(vk::Format::eR32G32B32A32Sfloat).setUsage(vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eStorage | vk::ImageUsageFlagBits::eColorAttachment));
            mNormmodBuffer->link(&normmodBuffer)->create2D(vk::Format::eR32G32B32A32Sfloat, appBase->applicationWindow.surfaceSize.width, appBase->applicationWindow.surfaceSize.height)->allocate(uintptr_t(&vmac));

            // 
            mParametBuffer = device->createImageMaker(api::ImageCreateInfo().setFormat(vk::Format::eR32G32B32A32Sfloat).setUsage(vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eStorage | vk::ImageUsageFlagBits::eColorAttachment));
            mParametBuffer->link(&parametBuffer)->create2D(vk::Format::eR32G32B32A32Sfloat, appBase->applicationWindow.surfaceSize.width, appBase->applicationWindow.surfaceSize.height)->allocate(uintptr_t(&vmac));

            // Create Swap Images
            mDiffuseBuffer[0] = device->createImageMaker(api::ImageCreateInfo().setFormat(vk::Format::eR32G32B32A32Sfloat).setUsage(vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eStorage | vk::ImageUsageFlagBits::eColorAttachment));
            mDiffuseBuffer[1] = device->createImageMaker(api::ImageCreateInfo().setFormat(vk::Format::eR32G32B32A32Sfloat).setUsage(vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eStorage | vk::ImageUsageFlagBits::eColorAttachment));
            mDiffuseBuffer[0]->link(&diffuseBuffer[0])->create2D(vk::Format::eR32G32B32A32Sfloat, appBase->applicationWindow.surfaceSize.width, appBase->applicationWindow.surfaceSize.height)->allocate(uintptr_t(&vmac));
            mDiffuseBuffer[1]->link(&diffuseBuffer[1])->create2D(vk::Format::eR32G32B32A32Sfloat, appBase->applicationWindow.surfaceSize.width, appBase->applicationWindow.surfaceSize.height)->allocate(uintptr_t(&vmac));

            // 
            mReflectBuffer[0] = device->createImageMaker(api::ImageCreateInfo().setFormat(vk::Format::eR32G32B32A32Sfloat).setUsage(vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eStorage | vk::ImageUsageFlagBits::eColorAttachment));
            mReflectBuffer[1] = device->createImageMaker(api::ImageCreateInfo().setFormat(vk::Format::eR32G32B32A32Sfloat).setUsage(vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eStorage | vk::ImageUsageFlagBits::eColorAttachment));
            mReflectBuffer[0]->link(&reflectBuffer[0])->create2D(vk::Format::eR32G32B32A32Sfloat, appBase->applicationWindow.surfaceSize.width, appBase->applicationWindow.surfaceSize.height)->allocate(uintptr_t(&vmac));
            mReflectBuffer[1]->link(&reflectBuffer[1])->create2D(vk::Format::eR32G32B32A32Sfloat, appBase->applicationWindow.surfaceSize.width, appBase->applicationWindow.surfaceSize.height)->allocate(uintptr_t(&vmac));

            // 
            mSamplesBuffer[0] = device->createImageMaker(api::ImageCreateInfo().setFormat(vk::Format::eR32G32B32A32Sfloat).setUsage(vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eStorage | vk::ImageUsageFlagBits::eColorAttachment));
            mSamplesBuffer[1] = device->createImageMaker(api::ImageCreateInfo().setFormat(vk::Format::eR32G32B32A32Sfloat).setUsage(vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eStorage | vk::ImageUsageFlagBits::eColorAttachment));
            mSamplesBuffer[0]->link(&samplesBuffer[0])->create2D(vk::Format::eR32G32B32A32Sfloat, appBase->applicationWindow.surfaceSize.width, appBase->applicationWindow.surfaceSize.height)->allocate(uintptr_t(&vmac));
            mSamplesBuffer[1]->link(&samplesBuffer[1])->create2D(vk::Format::eR32G32B32A32Sfloat, appBase->applicationWindow.surfaceSize.width, appBase->applicationWindow.surfaceSize.height)->allocate(uintptr_t(&vmac));

            // Run Layout Transition Command 
            lancer::submitOnce(*device, appBase->queue, appBase->commandPool, [&](vk::CommandBuffer& cmd) {
                mDiffuseBuffer[0]->imageBarrier(cmd);
                mReflectBuffer[0]->imageBarrier(cmd);
                mSamplesBuffer[0]->imageBarrier(cmd);
                mDiffuseBuffer[1]->imageBarrier(cmd);
                mReflectBuffer[1]->imageBarrier(cmd);
                mSamplesBuffer[1]->imageBarrier(cmd);
                mOutputsBuffer->imageBarrier(cmd);
                mDenoiseBuffer->imageBarrier(cmd);
                //mDepthStBuffer->imageBarrier(cmd);
                mColoredBuffer->imageBarrier(cmd);
                mNormalsBuffer->imageBarrier(cmd);
                mNormmodBuffer->imageBarrier(cmd);
                mParametBuffer->imageBarrier(cmd);
            });

            // Create sampler and description
            //vk::SamplerCreateInfo samplerInfo = {};
            //samplerInfo.addressModeU = vk::SamplerAddressMode::eClampToEdge;
            //samplerInfo.addressModeV = vk::SamplerAddressMode::eClampToEdge;
            //samplerInfo.minFilter = vk::Filter::eLinear;
            //samplerInfo.magFilter = vk::Filter::eLinear;
            //samplerInfo.compareEnable = false;
            //auto sampler = device->createSamplerMaker(samplerInfo);
            mDescriptorSetSwap[0] = device->createDescriptorSet(vk::DescriptorSetAllocateInfo(), &descriptorSetSwap[0])->linkLayout(&unifiedDescriptorLayout);
            mDescriptorSetSwap[1] = device->createDescriptorSet(vk::DescriptorSetAllocateInfo(), &descriptorSetSwap[1])->linkLayout(&unifiedDescriptorLayout);
        };

        { // Rasterization RenderPass
            mFirstPassRenderPass = device->createRenderPassMaker({}, &firstPassRenderPass);
            mFirstPassRenderPass->addSubpass(api::PipelineBindPoint::eGraphics)->getSubpassDescription();

            for (uint32_t i = 0u; i < 8u; i++) {
                mFirstPassRenderPass->addAttachment(vk::Format::eR32G32B32A32Sfloat)->getAttachmentDescription()
                    .setSamples(api::SampleCountFlagBits::e1)
                    .setLoadOp(api::AttachmentLoadOp::eLoad)
                    .setStoreOp(api::AttachmentStoreOp::eStore)
                    .setStencilLoadOp(api::AttachmentLoadOp::eDontCare)
                    .setStencilStoreOp(api::AttachmentStoreOp::eDontCare)
                    .setInitialLayout(api::ImageLayout::eGeneral)
                    .setFinalLayout(api::ImageLayout::eGeneral);
                mFirstPassRenderPass->subpassColorAttachment(i, api::ImageLayout::eColorAttachmentOptimal);
            };

            // 
            mFirstPassRenderPass->addAttachment(vk::Format::eD32SfloatS8Uint)->getAttachmentDescription()
                .setSamples(api::SampleCountFlagBits::e1)
                .setLoadOp(api::AttachmentLoadOp::eDontCare)
                .setStoreOp(api::AttachmentStoreOp::eDontCare)
                .setStencilLoadOp(api::AttachmentLoadOp::eDontCare)
                .setStencilStoreOp(api::AttachmentStoreOp::eDontCare)
                .setInitialLayout(api::ImageLayout::eUndefined)
                .setFinalLayout(api::ImageLayout::eDepthStencilAttachmentOptimal);
            mFirstPassRenderPass->subpassDepthStencilAttachment(8u, api::ImageLayout::eDepthStencilAttachmentOptimal);

            // 
            mFirstPassRenderPass->addDependency(VK_SUBPASS_EXTERNAL, 0u)->getSubpassDependency()
                .setDependencyFlags(api::DependencyFlagBits::eByRegion)
                .setSrcStageMask(api::PipelineStageFlagBits::eColorAttachmentOutput | api::PipelineStageFlagBits::eBottomOfPipe | api::PipelineStageFlagBits::eTransfer)
                .setSrcAccessMask(api::AccessFlagBits::eColorAttachmentWrite)
                .setDstStageMask(api::PipelineStageFlagBits::eColorAttachmentOutput)
                .setDstAccessMask(api::AccessFlagBits::eColorAttachmentRead | api::AccessFlagBits::eColorAttachmentWrite);

            // 
            mFirstPassRenderPass->addDependency(0u, VK_SUBPASS_EXTERNAL)->getSubpassDependency()
                .setDependencyFlags(api::DependencyFlagBits::eByRegion)
                .setSrcStageMask(api::PipelineStageFlagBits::eColorAttachmentOutput)
                .setSrcAccessMask(api::AccessFlagBits::eColorAttachmentRead | api::AccessFlagBits::eColorAttachmentWrite)
                .setDstStageMask(api::PipelineStageFlagBits::eColorAttachmentOutput | api::PipelineStageFlagBits::eTopOfPipe | api::PipelineStageFlagBits::eTransfer)
                .setDstAccessMask(api::AccessFlagBits::eColorAttachmentRead | api::AccessFlagBits::eColorAttachmentWrite);

            // 
            mFirstPassRenderPass->create()->getRenderPass();

            // Create Framebuffer For Rasterization 
            std::array<api::ImageView, 9u> views = {}; // predeclare views
            mColoredBuffer->createImageView(&views[0]);
            mNormalsBuffer->createImageView(&views[1]);
            mNormmodBuffer->createImageView(&views[2]);
            mParametBuffer->createImageView(&views[3]);
            mSamplesBuffer[0]->createImageView(&views[4]);
            mDiffuseBuffer[0]->createImageView(&views[5]);
            mReflectBuffer[0]->createImageView(&views[6]);
            mOutputsBuffer->createImageView(&views[7]);
            mDepthStBuffer->createImageView(&views[8]);
            firstPassFramebuffer = device->least().createFramebuffer(api::FramebufferCreateInfo{ {}, firstPassRenderPass, uint32_t(views.size()), views.data(), this->canvasWidth, this->canvasHeight, 1u });
        };

        { // Reprojection RenderPass
            mReprojectRenderPass = device->createRenderPassMaker({}, &reprojectRenderPass);
            mReprojectRenderPass->addSubpass(api::PipelineBindPoint::eGraphics)->getSubpassDescription();

            // Ispanec Hohotun (Risitas)
            for (uint32_t i = 0u; i < 1u; i++) {
                mReprojectRenderPass->addAttachment(vk::Format::eR32G32B32A32Sfloat)->getAttachmentDescription()
                    .setSamples(api::SampleCountFlagBits::e1)
                    .setLoadOp(api::AttachmentLoadOp::eLoad)
                    .setStoreOp(api::AttachmentStoreOp::eStore)
                    .setStencilLoadOp(api::AttachmentLoadOp::eDontCare)
                    .setStencilStoreOp(api::AttachmentStoreOp::eDontCare)
                    .setInitialLayout(api::ImageLayout::eGeneral)
                    .setFinalLayout(api::ImageLayout::eColorAttachmentOptimal);
                mReprojectRenderPass->subpassColorAttachment(i, api::ImageLayout::eColorAttachmentOptimal);
            };

            // 
            mReprojectRenderPass->addAttachment(vk::Format::eD32SfloatS8Uint)->getAttachmentDescription()
                .setSamples(api::SampleCountFlagBits::e1)
                .setLoadOp(api::AttachmentLoadOp::eDontCare)
                .setStoreOp(api::AttachmentStoreOp::eDontCare)
                .setStencilLoadOp(api::AttachmentLoadOp::eDontCare)
                .setStencilStoreOp(api::AttachmentStoreOp::eDontCare)
                .setInitialLayout(api::ImageLayout::eGeneral)
                .setFinalLayout(api::ImageLayout::eDepthStencilAttachmentOptimal);
            mReprojectRenderPass->subpassDepthStencilAttachment(1u, api::ImageLayout::eDepthStencilAttachmentOptimal);

            // 
            mReprojectRenderPass->addDependency(VK_SUBPASS_EXTERNAL, 0u)->getSubpassDependency()
                .setDependencyFlags(api::DependencyFlagBits::eByRegion)
                .setSrcStageMask(api::PipelineStageFlagBits::eColorAttachmentOutput | api::PipelineStageFlagBits::eBottomOfPipe | api::PipelineStageFlagBits::eTransfer)
                .setSrcAccessMask(api::AccessFlagBits::eColorAttachmentWrite)
                .setDstStageMask(api::PipelineStageFlagBits::eColorAttachmentOutput)
                .setDstAccessMask(api::AccessFlagBits::eColorAttachmentRead | api::AccessFlagBits::eColorAttachmentWrite);

            // 
            mReprojectRenderPass->addDependency(0u, VK_SUBPASS_EXTERNAL)->getSubpassDependency()
                .setDependencyFlags(api::DependencyFlagBits::eByRegion)
                .setSrcStageMask(api::PipelineStageFlagBits::eColorAttachmentOutput)
                .setSrcAccessMask(api::AccessFlagBits::eColorAttachmentRead | api::AccessFlagBits::eColorAttachmentWrite)
                .setDstStageMask(api::PipelineStageFlagBits::eColorAttachmentOutput | api::PipelineStageFlagBits::eTopOfPipe | api::PipelineStageFlagBits::eTransfer)
                .setDstAccessMask(api::AccessFlagBits::eColorAttachmentRead | api::AccessFlagBits::eColorAttachmentWrite);

            // 
            mReprojectRenderPass->create()->getRenderPass();

            // Create Framebuffer For Rasterization 
            std::array<api::ImageView, 2> views = {}; // predeclare views
            mDiffuseBuffer[1]->createImageView(&views[0]);
            mDepthStBuffer->createImageView(&views[1]);
            reprojectFramebuffer = device->least().createFramebuffer(api::FramebufferCreateInfo{ {}, reprojectRenderPass, uint32_t(views.size()), views.data(), this->canvasWidth, this->canvasHeight, 1u });
        };


        {   // == Ray Tracing Pipeline == 
            // Create Ray Tracing Pipeline and SBT
            mRaytracedPipeline = device->createSBTHelper(api::RayTracingPipelineCreateInfoNV(), &raytracedPipeline)->linkBuffer(&rtSBT)->linkPipelineLayout(&unifiedPipelineLayout)->initialize();
            mRaytracedPipeline->setRaygenStage(vk::PipelineShaderStageCreateInfo().setModule(lancer::createShaderModule(*device, lancer::readBinary(shaderPack + "/rtrace/pathtrace.rgen.spv"))).setPName("main").setStage(vk::ShaderStageFlagBits::eRaygenNV));
            mRaytracedPipeline->addStageToHitGroup(vk::PipelineShaderStageCreateInfo().setModule(lancer::createShaderModule(*device, lancer::readBinary(shaderPack + "/rtrace/pathtrace.rchit.spv"))).setPName("main").setStage(vk::ShaderStageFlagBits::eClosestHitNV));
            mRaytracedPipeline->addStageToMissGroup(vk::PipelineShaderStageCreateInfo().setModule(lancer::createShaderModule(*device, lancer::readBinary(shaderPack + "/rtrace/pathtrace.rmiss.spv"))).setPName("main").setStage(vk::ShaderStageFlagBits::eMissNV));
            mRaytracedPipeline->create();
        };

        {   // == Mesh Rasterization Shader == 
            mRasterizePipeline = device->createGraphicsPipelineMaker(vk::GraphicsPipelineCreateInfo(), &rasterizePipeline);
            mRasterizePipeline->getInputAssemblyState().setTopology(vk::PrimitiveTopology::eTriangleStrip);
            mRasterizePipeline->pushShaderModule(vk::PipelineShaderStageCreateInfo().setModule(lancer::createShaderModule(*device, lancer::readBinary(shaderPack + "/rtrace/rasterize.vert.spv"))).setPName("main").setStage(vk::ShaderStageFlagBits::eVertex));
            mRasterizePipeline->pushShaderModule(vk::PipelineShaderStageCreateInfo().setModule(lancer::createShaderModule(*device, lancer::readBinary(shaderPack + "/rtrace/rasterize.frag.spv"))).setPName("main").setStage(vk::ShaderStageFlagBits::eFragment));
            mRasterizePipeline->pushDynamicState(vk::DynamicState::eViewport)->pushDynamicState(vk::DynamicState::eScissor);
            mRasterizePipeline->getScissor().setExtent(api::Extent2D{ this->canvasWidth, this->canvasHeight });
            mRasterizePipeline->getViewport().setWidth(this->canvasWidth).setHeight(this->canvasHeight);

            // 
            mRasterizePipeline->pushVertexBinding(0u, sizeof(glm::vec4), vk::VertexInputRate::eVertex);
            mRasterizePipeline->pushVertexBinding(1u, sizeof(glm::vec4), vk::VertexInputRate::eVertex);
            mRasterizePipeline->pushVertexBinding(2u, sizeof(glm::vec4), vk::VertexInputRate::eVertex);
            mRasterizePipeline->pushVertexBinding(3u, sizeof(glm::vec4), vk::VertexInputRate::eVertex);
            mRasterizePipeline->pushVertexBinding(4u, sizeof(glm::ivec4), vk::VertexInputRate::eVertex); // Index Data

            // 
            mRasterizePipeline->pushVertexAttribute(0u, 0u, vk::Format::eR32G32B32Sfloat, 0u);
            mRasterizePipeline->pushVertexAttribute(1u, 1u, vk::Format::eR32G32B32A32Sfloat, 0u);
            mRasterizePipeline->pushVertexAttribute(2u, 2u, vk::Format::eR32G32B32A32Sfloat, 0u);
            mRasterizePipeline->pushVertexAttribute(3u, 3u, vk::Format::eR32G32B32A32Sfloat, 0u);
            mRasterizePipeline->pushVertexAttribute(4u, 4u, vk::Format::eR32G32B32A32Sint, 0u); // Index Data

            for (uint32_t i = 0; i < 8; i++) {
                mRasterizePipeline->pushColorBlendAttachment(vk::PipelineColorBlendAttachmentState()
                    .setBlendEnable(false)
                    .setSrcAlphaBlendFactor(vk::BlendFactor::eOne).setAlphaBlendOp(vk::BlendOp::eAdd).setDstAlphaBlendFactor(vk::BlendFactor::eOne)
                    .setSrcColorBlendFactor(vk::BlendFactor::eOne).setColorBlendOp(vk::BlendOp::eAdd).setDstColorBlendFactor(vk::BlendFactor::eOne)
                );
            };

            // 
            mRasterizePipeline->linkPipelineLayout(&unifiedPipelineLayout)->linkRenderPass(&firstPassRenderPass)->create(true);
        };

        {   // == Reprojection Rasterization Shader == 
            mReprojectPipeline = device->createGraphicsPipelineMaker(vk::GraphicsPipelineCreateInfo(), &reprojectPipeline);
            mReprojectPipeline->getInputAssemblyState().setTopology(vk::PrimitiveTopology::eTriangleStrip);
            mReprojectPipeline->pushShaderModule(vk::PipelineShaderStageCreateInfo().setModule(lancer::createShaderModule(*device, lancer::readBinary(shaderPack + "/rtrace/reproject.vert.spv"))).setPName("main").setStage(vk::ShaderStageFlagBits::eVertex));
            mReprojectPipeline->pushShaderModule(vk::PipelineShaderStageCreateInfo().setModule(lancer::createShaderModule(*device, lancer::readBinary(shaderPack + "/rtrace/reproject.frag.spv"))).setPName("main").setStage(vk::ShaderStageFlagBits::eFragment));
            mReprojectPipeline->pushDynamicState(vk::DynamicState::eViewport)->pushDynamicState(vk::DynamicState::eScissor);
            mReprojectPipeline->getScissor().setExtent(api::Extent2D{ this->canvasWidth, this->canvasHeight });
            mReprojectPipeline->getViewport().setWidth(this->canvasWidth).setHeight(this->canvasHeight);
            mReprojectPipeline->getDepthStencilState().setDepthTestEnable(true).setDepthWriteEnable(true).setDepthCompareOp(vk::CompareOp::eLessOrEqual);
            mReprojectPipeline->getInputAssemblyState().setTopology(vk::PrimitiveTopology::ePointList);
            mReprojectPipeline->pushColorBlendAttachment(vk::PipelineColorBlendAttachmentState()
                .setBlendEnable(true)
                .setSrcAlphaBlendFactor(vk::BlendFactor::eOne).setAlphaBlendOp(vk::BlendOp::eAdd).setDstAlphaBlendFactor(vk::BlendFactor::eOne)
                .setSrcColorBlendFactor(vk::BlendFactor::eOne).setColorBlendOp(vk::BlendOp::eAdd).setDstColorBlendFactor(vk::BlendFactor::eOne)
            );
            mReprojectPipeline->linkPipelineLayout(&unifiedPipelineLayout)->linkRenderPass(&reprojectRenderPass)->create(true);
        };

        {   // == Final Rasterization Shader (Planned To Replace) == 
            mFinalDrawPipeline = device->createGraphicsPipelineMaker(vk::GraphicsPipelineCreateInfo(), &finalDrawPipeline);
            mFinalDrawPipeline->getInputAssemblyState().setTopology(vk::PrimitiveTopology::eTriangleStrip);
            mFinalDrawPipeline->pushShaderModule(vk::PipelineShaderStageCreateInfo().setModule(lancer::createShaderModule(*device, lancer::readBinary(shaderPack + "/rtrace/render.vert.spv"))).setPName("main").setStage(vk::ShaderStageFlagBits::eVertex));
            mFinalDrawPipeline->pushShaderModule(vk::PipelineShaderStageCreateInfo().setModule(lancer::createShaderModule(*device, lancer::readBinary(shaderPack + "/rtrace/render.frag.spv"))).setPName("main").setStage(vk::ShaderStageFlagBits::eFragment));
            mFinalDrawPipeline->pushDynamicState(vk::DynamicState::eViewport)->pushDynamicState(vk::DynamicState::eScissor);
            mFinalDrawPipeline->getScissor().setExtent(api::Extent2D{ this->canvasWidth, this->canvasHeight });
            mFinalDrawPipeline->getViewport().setWidth(this->canvasWidth).setHeight(this->canvasHeight);
            mFinalDrawPipeline->linkPipelineLayout(&unifiedPipelineLayout)->linkRenderPass(&appBase->renderPass)->create(true);
        };




        // Fill Descriptor Sets With Images
        std::array<uint32_t, 4> indices = { 4, 2, 4, 2 }; // 4 is write, 2 read-only
        std::array<uint32_t, 4> pinpong = { 0, 0, 1, 1 };
        std::array<uint32_t, 4> swaps   = { 1, 0, 0, 1 };
        std::array<bool, 4> sampler = { false, true, false, true };
        std::array<bool, 4> uniform = { false, true, false, true };

        // 
        for (uint32_t i = 0u; i < 4u; i++) { // Swap 0 Image Stores
            const auto I = indices[i];
            const auto S = swaps[i];

            // 
            auto mImages = mDescriptorSetSwap[pinpong[i]]->addImageDesc(I, 0, 9, uniform[i], sampler[i]);

            // 
            mSamplesBuffer[S]->createImageView(&(mImages + 0u)->imageView, api::ImageViewType::e2D, vk::Format::eR32G32B32A32Sfloat);
            mDiffuseBuffer[S]->createImageView(&(mImages + 1u)->imageView, api::ImageViewType::e2D, vk::Format::eR32G32B32A32Sfloat);
            mReflectBuffer[S]->createImageView(&(mImages + 2u)->imageView, api::ImageViewType::e2D, vk::Format::eR32G32B32A32Sfloat);
            mColoredBuffer->createImageView(&(mImages + 3u)->imageView, api::ImageViewType::e2D, vk::Format::eR32G32B32A32Sfloat);
            mNormalsBuffer->createImageView(&(mImages + 4u)->imageView, api::ImageViewType::e2D, vk::Format::eR32G32B32A32Sfloat);
            mNormmodBuffer->createImageView(&(mImages + 5u)->imageView, api::ImageViewType::e2D, vk::Format::eR32G32B32A32Sfloat);
            mParametBuffer->createImageView(&(mImages + 6u)->imageView, api::ImageViewType::e2D, vk::Format::eR32G32B32A32Sfloat);
            mDenoiseBuffer->createImageView(&(mImages + 7u)->imageView, api::ImageViewType::e2D, vk::Format::eR32G32B32A32Sfloat);
            mOutputsBuffer->createImageView(&(mImages + 8u)->imageView, api::ImageViewType::e2D, vk::Format::eR32G32B32A32Sfloat);

            // Create Output Image View
            (mImages + 0u)->imageLayout = mSamplesBuffer[S]->getTargetLayout();
            (mImages + 1u)->imageLayout = mDiffuseBuffer[S]->getTargetLayout();
            (mImages + 2u)->imageLayout = mReflectBuffer[S]->getTargetLayout();
            (mImages + 3u)->imageLayout = mColoredBuffer->getTargetLayout();
            (mImages + 4u)->imageLayout = mNormalsBuffer->getTargetLayout();
            (mImages + 5u)->imageLayout = mNormmodBuffer->getTargetLayout();
            (mImages + 6u)->imageLayout = mParametBuffer->getTargetLayout();
            (mImages + 7u)->imageLayout = mDenoiseBuffer->getTargetLayout();
            (mImages + 8u)->imageLayout = mOutputsBuffer->getTargetLayout();

            // 
            if (sampler[i]) {
                vk::SamplerCreateInfo samplerInfo = {};
                samplerInfo.addressModeU = vk::SamplerAddressMode::eClampToEdge;
                samplerInfo.addressModeV = vk::SamplerAddressMode::eClampToEdge;
                samplerInfo.minFilter = vk::Filter::eLinear;
                samplerInfo.magFilter = vk::Filter::eLinear;
                samplerInfo.compareEnable = false;

                // Combine With Samplers
                (mImages + 0u)->sampler = device->least().createSampler(samplerInfo);
                (mImages + 1u)->sampler = device->least().createSampler(samplerInfo);
                (mImages + 2u)->sampler = device->least().createSampler(samplerInfo);
                (mImages + 3u)->sampler = device->least().createSampler(samplerInfo);
                (mImages + 4u)->sampler = device->least().createSampler(samplerInfo);
                (mImages + 5u)->sampler = device->least().createSampler(samplerInfo);
                (mImages + 6u)->sampler = device->least().createSampler(samplerInfo);
                (mImages + 7u)->sampler = device->least().createSampler(samplerInfo);
                (mImages + 8u)->sampler = device->least().createSampler(samplerInfo);
            };
        };



        {   // == Ray Tracing Descriptor Set ==
            // Create and apply descriptor set 

            // Add Ray Tracing Object
            rtAccelTop->writeForDescription(mDescriptorSetSwap[0]->addAccelerationStructureDesc(1, 0, 1));
            rtAccelTop->writeForDescription(mDescriptorSetSwap[1]->addAccelerationStructureDesc(1, 0, 1));

            // Apply Descriptor Set
            mDescriptorSetSwap[0]->create()->apply();
            mDescriptorSetSwap[1]->create()->apply();
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
                cmd.bindDescriptorSets(vk::PipelineBindPoint::eRayTracingNV, unifiedPipelineLayout, 0u, descriptorSetSwap[0], nullptr);
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
                commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, unifiedPipelineLayout, 0, descriptorSetSwap[0], nullptr);
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
