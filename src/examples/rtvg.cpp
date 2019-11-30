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

    const uint32_t blockWidth = 8, blockheight = 8;

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

        try { parser.ParseCLI(argc, argv); }
        catch (args::Help) { std::cout << parser; glfwTerminate(); exit(1); };

        // read arguments
        if (deviceflag) gpuID = args::get(deviceflag);
        if (shaderflag) shaderPrefix = args::get(shaderflag);
    };

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

        //cameraController = std::make_shared<CameraController>();
        //cameraController->canvasSize = (glm::uvec2*) & this->windowWidth;
        //cameraController->eyePos = &this->eyePos;
        //cameraController->upVector = &this->upVector;
        //cameraController->viewVector = &this->viewVector;

        // create VK instance
        auto instance = appBase->createInstance();
        if (!instance) { glfwTerminate(); exit(EXIT_FAILURE); }

        // get physical devices
        //auto physicalDevices = instance.enumeratePhysicalDevices();
        //if (physicalDevices.size() < 0) { glfwTerminate(); std::cerr << "Vulkan does not supported, or driver broken." << std::endl; exit(0); }

        // choice device
        //if (gpuID >= physicalDevices.size()) { gpuID = physicalDevices.size() - 1; }
        //if (gpuID < 0 || gpuID == -1) gpuID = 0;

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
        shaderPack = shaderPrefix + "intrusive/universal";

        // create radix sort application (RadX C++)
        physicalHelper = std::make_shared<lancer::PhysicalDevice_T>(appBase->getPhysicalDevice(0));
        device = appBase->createDevice(false, shaderPack, true);

        // create image output
        const auto SuperSampling = enableSuperSampling ? 2.0 : 1.0; // super sampling image
        this->canvasWidth = this->windowWidth * SuperSampling;
        this->canvasHeight = this->windowHeight * SuperSampling;

        // create framebuffers 
        framebuffers = appBase->createSwapchainFramebuffer(swapchain = appBase->createSwapchain(), appBase->createRenderPass());
    };

    void Renderer::InitPipeline() {
        // create pipeline
        //vk::Pipeline trianglePipeline = {};

        { // TODO: Update Descriptor Layout Maker (That Able Workaround Flags API Code Problem)
            const auto pbindings = vk::DescriptorBindingFlagBitsEXT::ePartiallyBound | vk::DescriptorBindingFlagBitsEXT::eUpdateAfterBind | vk::DescriptorBindingFlagBitsEXT::eVariableDescriptorCount | vk::DescriptorBindingFlagBitsEXT::eUpdateUnusedWhilePending;
            const auto vkfl = vk::DescriptorSetLayoutBindingFlagsCreateInfoEXT().setPBindingFlags(&pbindings);
            const auto vkpi = vk::DescriptorSetLayoutCreateInfo().setPNext(&vkfl);
            const std::vector<vk::DescriptorSetLayoutBinding> _bindings = {
                //vk::DescriptorSetLayoutBinding(0, vk::DescriptorType::eStorageBuffer, 1, vk::ShaderStageFlagBits::eAll),
                //vk::DescriptorSetLayoutBinding(1, vk::DescriptorType::eStorageBuffer, 1, vk::ShaderStageFlagBits::eAll),
                vk::DescriptorSetLayoutBinding(2, vk::DescriptorType::eStorageImage, 1, vk::ShaderStageFlagBits::eAll),
                //vk::DescriptorSetLayoutBinding(3, vk::DescriptorType::eAccelerationStructureNV, 1, vk::ShaderStageFlagBits::eAll),
            };
            inputDescriptorLayout = device->least().createDescriptorSetLayout(vk::DescriptorSetLayoutCreateInfo(vkpi).setPBindings(_bindings.data()).setBindingCount(_bindings.size()));
        };

        {
            lancer::GraphicsPipelineMaker maker = device->createGraphicsPipelineMaker(vk::GraphicsPipelineCreateInfo(),&trianglePipeline);
            lancer::PipelineLayoutMaker dlayout = device->createPipelineLayoutMaker(vk::PipelineLayoutCreateInfo(),&trianglePipelineLayout);
            dlayout->pushDescriptorSetLayout(inputDescriptorLayout)->create();

            maker->pushShaderModule(vk::PipelineShaderStageCreateInfo().setModule(lancer::createShaderModule(*device,lancer::readBinary(shaderPack + "/render/render.vert.spv"))).setPName("main").setStage(vk::ShaderStageFlagBits::eVertex));
            maker->pushShaderModule(vk::PipelineShaderStageCreateInfo().setModule(lancer::createShaderModule(*device,lancer::readBinary(shaderPack + "/render/render.frag.spv"))).setPName("main").setStage(vk::ShaderStageFlagBits::eFragment));
            maker->pushDynamicState(vk::DynamicState::eViewport)->pushDynamicState(vk::DynamicState::eScissor);
            maker->link(&trianglePipeline)->linkPipelineLayout(&trianglePipelineLayout)->linkRenderPass(&appBase->renderPass)->create(true);
        };

        { // TODO: Update Descriptor Set Maker
            //outputImage = std::make_shared<>(device, vk::ImageViewType::e2D, vk::Format::eR32G32B32A32Sfloat, appBase->applicationWindow.surfaceSize, vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eStorage);
            outputImage = device->createImageMaker(api::ImageCreateInfo().setFormat(vk::Format::eR32G32B32A32Sfloat).setUsage(vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eStorage));
            outputImage->link(&outputImage_)->create2D(vk::Format::eR32G32B32A32Sfloat,appBase->applicationWindow.surfaceSize.width,appBase->applicationWindow.surfaceSize.height);

            // Legacy Sampler Creator
            vk::SamplerCreateInfo samplerInfo = {};
            samplerInfo.addressModeU = vk::SamplerAddressMode::eClampToEdge;
            samplerInfo.addressModeV = vk::SamplerAddressMode::eClampToEdge;
            samplerInfo.minFilter = vk::Filter::eLinear;
            samplerInfo.magFilter = vk::Filter::eLinear;
            samplerInfo.compareEnable = false;
            auto sampler = device->least().createSampler(samplerInfo); // create sampler

            // desc texture texture
            auto imageDesc = vk::DescriptorImageInfo();//.setSampler(sampler);
            outputImage->createImageView(&imageDesc.imageView,api::ImageViewType::e2D,vk::Format::eR32G32B32A32Sfloat);

            // submit as secondary
            lancer::submitOnce(*device, appBase->queue, appBase->commandPool, [&](vk::CommandBuffer& cmd) { outputImage->imageBarrier(cmd); });

            // TODO: Update Descriptor Set Maker
            std::vector<vk::DescriptorSetLayout> dsLayouts = { vk::DescriptorSetLayout(inputDescriptorLayout) };
            auto dsc = device->least().allocateDescriptorSets(vk::DescriptorSetAllocateInfo().setDescriptorPool(device->getDescriptorPool()).setPSetLayouts(&dsLayouts[0]).setDescriptorSetCount(1));
            auto writeTmpl = vk::WriteDescriptorSet(inputDescriptorSet = dsc[0], 0, 0, 1, vk::DescriptorType::eStorageBuffer);

            device->least().updateDescriptorSets(std::vector<vk::WriteDescriptorSet>{
                api::WriteDescriptorSet(writeTmpl).setDescriptorType(vk::DescriptorType::eStorageImage).setDstBinding(2).setPImageInfo(&imageDesc),
            }, {});

        };
    };



    void Renderer::Draw() {
        auto n_semaphore = currSemaphore;
        auto c_semaphore = int32_t((size_t(currSemaphore) + 1ull) % framebuffers.size());
        currSemaphore = c_semaphore;

        // acquire next image where will rendered (and get semaphore when will presented finally)
        n_semaphore = (n_semaphore >= 0 ? n_semaphore : (framebuffers.size() - 1));
        device->least().acquireNextImageKHR(swapchain, std::numeric_limits<uint64_t>::max(), framebuffers[n_semaphore].semaphore, nullptr, &currentBuffer);

        { // submit rendering (and wait presentation in device)
            std::vector<vk::ClearValue> clearValues = { vk::ClearColorValue(std::array<float,4>{0.2f, 0.2f, 0.2f, 1.0f}), vk::ClearDepthStencilValue(1.0f, 0) };
            auto renderArea = vk::Rect2D(vk::Offset2D(0, 0), appBase->size());
            auto viewport = vk::Viewport(0.0f, 0.0f, appBase->size().width, appBase->size().height, 0, 1.0f);


            // create ray-tracing command (i.e. render vector graphics as is)




            // create command buffer (with rewrite)
            vk::CommandBuffer& commandBuffer = framebuffers[n_semaphore].commandBuffer;
            if (!commandBuffer) {
                commandBuffer = lancer::createCommandBuffer(*device, appBase->commandPool, false, false); // do reference of cmd buffer
                commandBuffer.beginRenderPass(vk::RenderPassBeginInfo(appBase->renderPass, framebuffers[currentBuffer].frameBuffer, renderArea, clearValues.size(), clearValues.data()), vk::SubpassContents::eInline);
                commandBuffer.setViewport(0, std::vector<vk::Viewport> { viewport });
                commandBuffer.setScissor(0, std::vector<vk::Rect2D> { renderArea });
                commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, trianglePipeline);
                commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, trianglePipelineLayout, 0, inputDescriptorSet, nullptr);
                commandBuffer.draw(4, 1, 0, 0);
                commandBuffer.endRenderPass();
                commandBuffer.end();
            };

            // submit as secondary
            //radx::submitOnce(*device, appBase->queue, appBase->commandPool, [&](VkCommandBuffer cmd) {
            //    vk::CommandBuffer(cmd).executeCommands({ rtCmdBuf });
            //});


            // create render submission 
            std::vector<vk::Semaphore>
                waitSemaphores = { framebuffers[n_semaphore].semaphore },
                signalSemaphores = { framebuffers[c_semaphore].semaphore };
            std::vector<vk::PipelineStageFlags> waitStages = { vk::PipelineStageFlagBits::eColorAttachmentOutput };

            // 
            std::array<vk::CommandBuffer, 1> XPEH = { commandBuffer };
            auto smbi = vk::SubmitInfo()
                .setPCommandBuffers(XPEH.data()).setCommandBufferCount(XPEH.size())
                .setPWaitDstStageMask(waitStages.data()).setPWaitSemaphores(waitSemaphores.data()).setWaitSemaphoreCount(waitSemaphores.size())
                .setPSignalSemaphores(signalSemaphores.data()).setSignalSemaphoreCount(signalSemaphores.size());

            // submit command once
            lancer::submitCmd(*device, appBase->queue, { commandBuffer }, smbi);

            // delete command buffer 
            //{ currentContext->queue->device->logical.freeCommandBuffers(currentContext->queue->commandPool, { commandBuffer }); commandBuffer = nullptr; };
        };

        // present for displaying of this image
        vk::Queue(appBase->queue).presentKHR(vk::PresentInfoKHR(
            1, &framebuffers[c_semaphore].semaphore,
            1, &swapchain,
            &currentBuffer, nullptr
        ));
    };


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
