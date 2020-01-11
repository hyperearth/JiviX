#define VMA_IMPLEMENTATION
#include <vkt2/fw.hpp>
#include <lancer-vk/lancer.hpp>

std::shared_ptr<vkt::GPUFramework> fw = {};

int main() {
    glfwInit();

    // 
	if (GLFW_FALSE == glfwVulkanSupported()) {
		glfwTerminate(); return -1;
	};

    // 
	uint32_t canvasWidth = 800, canvasHeight = 600;
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

	// initialize Vulkan
    auto fw = std::make_shared<vkt::GPUFramework>();
	auto instance = fw->createInstance();
	auto manager = fw->createWindowSurface(canvasWidth, canvasHeight);
	auto physicalDevice = fw->getPhysicalDevice(0u);
	auto device = fw->createDevice(true,"./",false);
	auto swapchain = fw->createSwapchain();
	auto renderPass = fw->createRenderPass();
	auto framebuffers = fw->createSwapchainFramebuffer(swapchain, renderPass);
	auto queue = fw->getQueue();
	auto commandPool = fw->getCommandPool();

    // 
    auto renderArea = vk::Rect2D{ vk::Offset2D(0, 0), vk::Extent2D(canvasWidth, canvasHeight) };
    auto viewport = vk::Viewport{ 0.0f, 0.0f, static_cast<float>(renderArea.extent.width), static_cast<float>(renderArea.extent.height), 0.f, 1.f };

    // added for LOC testing
    auto hostBuffer = vkt::Vector<glm::vec4>(std::make_shared<vkt::VmaBufferAllocation>(fw->getAllocator(), vkh::VkBufferCreateInfo{
        .size = 16u * 3u,
        .usage = { .eTransferSrc = 1, .eStorageBuffer = 1, .eVertexBuffer = 1 },
    }, VMA_MEMORY_USAGE_CPU_TO_GPU));

    // triangle data
    hostBuffer[0] = glm::vec4( 1.f, -1.f, 0.f, 1.f);
    hostBuffer[1] = glm::vec4(-1.f, -1.f, 0.f, 1.f);
    hostBuffer[2] = glm::vec4( 0.f,  1.f, 0.f, 1.f);

    // 
    auto gpuBuffer = vkt::Vector<glm::vec4>(std::make_shared<vkt::VmaBufferAllocation>(fw->getAllocator(), vkh::VkBufferCreateInfo{
        .size = 16u * 3u,
        .usage = { .eTransferDst = 1, .eStorageBuffer = 1, .eVertexBuffer = 1 },
    }, VMA_MEMORY_USAGE_GPU_ONLY));

    // 
    vkt::submitOnce(device, queue, commandPool, [=](vk::CommandBuffer& cmd) {
        cmd.copyBuffer(hostBuffer, gpuBuffer, { vkh::VkBufferCopy{.size = 16u * 3u} });
    });  // Buffer LOC test end


    // 
    auto context = std::make_shared<lancer::Context>(fw);
    auto mesh = std::make_shared<lancer::Mesh>(context);
    auto node = std::make_shared<lancer::Node>(context);
    auto material = std::make_shared<lancer::Material>(context);
    auto renderer = std::make_shared<lancer::Renderer>(context);

    // initialize renderer
    context->initialize(canvasWidth, canvasHeight);
    renderer->linkMaterial(material)->linkNode(node);

    // geometry data
    mesh->addBinding(gpuBuffer, { .stride = 16u });
    mesh->addAttribute({ .format = VK_FORMAT_R32G32B32A32_SFLOAT }, true);
    node->pushInstance(vkh::VsGeometryInstance{}, node->pushMesh(mesh));

    // initialize program
    renderer->setupCommands();

    // 
    vkh::VsGraphicsPipelineCreateInfoConstruction pipelineInfo = {};
    pipelineInfo.stages = vkt::vector_cast<vkh::VkPipelineShaderStageCreateInfo, vk::PipelineShaderStageCreateInfo>({
        vkt::makePipelineStageInfo(device, vkt::readBinary("./shaders/rtrace/render.vert.spv"), vk::ShaderStageFlagBits::eVertex),
        vkt::makePipelineStageInfo(device, vkt::readBinary("./shaders/rtrace/render.frag.spv"), vk::ShaderStageFlagBits::eFragment)
    });
    pipelineInfo.graphicsPipelineCreateInfo.layout = context->getPipelineLayout();
    pipelineInfo.graphicsPipelineCreateInfo.renderPass = fw->renderPass;//context->refRenderPass();
    pipelineInfo.inputAssemblyState.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;
    pipelineInfo.viewportState.pViewports = &reinterpret_cast<vkh::VkViewport&>(viewport);
    pipelineInfo.viewportState.pScissors = &reinterpret_cast<vkh::VkRect2D&>(renderArea);
    pipelineInfo.colorBlendAttachmentStates = { {} }; // Default Blend State
    pipelineInfo.dynamicStates = vkt::vector_cast<VkDynamicState,vk::DynamicState>({vk::DynamicState::eScissor, vk::DynamicState::eViewport});
    auto finalPipeline = device.createGraphicsPipeline(fw->getPipelineCache(), pipelineInfo);

	// 
	int currSemaphore = -1;
	uint32_t currentBuffer = 0u;

	// 
	while (!glfwWindowShouldClose(manager.window)) {
        glfwPollEvents();

        // 
        auto n_semaphore = currSemaphore;
        auto c_semaphore = int32_t((size_t(currSemaphore) + 1ull) % framebuffers.size());
        currSemaphore = c_semaphore;

        // acquire next image where will rendered (and get semaphore when will presented finally)
        n_semaphore = (n_semaphore >= 0 ? n_semaphore : (framebuffers.size() - 1));
        device.acquireNextImageKHR(swapchain, std::numeric_limits<uint64_t>::max(), framebuffers[n_semaphore].semaphore, nullptr, &currentBuffer);

        { // submit rendering (and wait presentation in device)
            std::vector<vk::ClearValue> clearValues = { vk::ClearColorValue(std::array<float,4>{0.f, 0.f, 0.f, 0.0f}), vk::ClearDepthStencilValue(1.0f, 0) };

            // create command buffer (with rewrite)
            vk::CommandBuffer& commandBuffer = framebuffers[n_semaphore].commandBuffer;
            if (!commandBuffer) {
                commandBuffer = vkt::createCommandBuffer(device, commandPool, false, false); // do reference of cmd buffer
                commandBuffer.beginRenderPass(vk::RenderPassBeginInfo(fw->renderPass, framebuffers[currentBuffer].frameBuffer, renderArea, clearValues.size(), clearValues.data()), vk::SubpassContents::eInline);
                commandBuffer.setViewport(0, { viewport });
                commandBuffer.setScissor(0, { renderArea });
                commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, finalPipeline);
                commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, context->getPipelineLayout(), 0, context->getDescriptorSets(), nullptr);
                commandBuffer.draw(4, 1, 0, 0);
                commandBuffer.endRenderPass();
                commandBuffer.end();
            };

            // Create render submission 
            std::vector<vk::Semaphore> waitSemaphores = { framebuffers[n_semaphore].semaphore }, signalSemaphores = { framebuffers[c_semaphore].semaphore };
            std::vector<vk::PipelineStageFlags> waitStages = { vk::PipelineStageFlagBits::eColorAttachmentOutput };
            std::array<vk::CommandBuffer, 1> XPEHb = { commandBuffer };

            // Submit command once
            vkt::submitCmd(device, queue, { commandBuffer }, vk::SubmitInfo()
                .setPCommandBuffers(XPEHb.data()).setCommandBufferCount(XPEHb.size())
                .setPWaitDstStageMask(waitStages.data()).setPWaitSemaphores(waitSemaphores.data()).setWaitSemaphoreCount(waitSemaphores.size())
                .setPSignalSemaphores(signalSemaphores.data()).setSignalSemaphoreCount(signalSemaphores.size()));
        };

        // present for displaying of this image
        vk::Queue(queue).presentKHR(vk::PresentInfoKHR(
            1, &framebuffers[c_semaphore].semaphore,
            1, &swapchain,
            &currentBuffer, nullptr
        ));

	};

    // 
	glfwDestroyWindow(manager.window);
	glfwTerminate();
    return 0;
};
