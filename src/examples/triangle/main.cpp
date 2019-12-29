#define VMA_IMPLEMENTATION
#include "vkt2/fw.hpp"

vkt::GPUFramework fw = {};

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
	auto instance = fw.createInstance();
	auto manager = fw.createWindowSurface(canvasWidth, canvasHeight);
	auto physicalDevice = fw.getPhysicalDevice(0u);
	auto device = fw.createDevice(true,"./",false);
	auto swapchain = fw.createSwapchain();
	auto renderPass = fw.createRenderPass();
	auto framebuffers = fw.createSwapchainFramebuffer(swapchain, renderPass);
	auto queue = fw.getQueue();
	auto commandPool = fw.getCommandPool();

    // Vookoo-styled Create Graphics
    vkh::VsDescriptorSetLayoutCreateInfoHelper descriptorSetLayoutInfo = {};
    descriptorSetLayoutInfo.pushBinding(vkh::VkDescriptorBindingFlagsEXT{  },vkh::VkDescriptorSetLayoutBinding{ .binding = 0u, .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, .stageFlags = { .eVertex = 1, .eFragment = 1, .eCompute = 1 } });
    vk::DescriptorSetLayout descriptorSetLayout = device.createDescriptorSetLayout(VkDescriptorSetLayoutCreateInfo(descriptorSetLayoutInfo));

    // 
    vkh::VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
    std::vector<VkDescriptorSetLayout> layouts{ descriptorSetLayout };
    vk::PipelineLayout finalPipelineLayout = device.createPipelineLayout(VkPipelineLayoutCreateInfo(pipelineLayoutInfo.setSetLayouts(layouts)));

    // 
    auto renderArea = vk::Rect2D(vk::Offset2D(0, 0), vk::Extent2D(canvasWidth, canvasHeight));
    auto viewport = vk::Viewport(0.0f, 0.0f, renderArea.extent.width, renderArea.extent.height, 0, 1.0f);

    // 
    vkh::VsGraphicsPipelineCreateInfoConstruction pipelineInfo = {};
    pipelineInfo.stages = vkt::vector_cast<vkh::VkPipelineShaderStageCreateInfo>(std::vector<VkPipelineShaderStageCreateInfo>{
        vkt::makePipelineStageInfo(device, vkt::readBinary("./shaders/render.vert.spv"), vk::ShaderStageFlagBits::eVertex),
        vkt::makePipelineStageInfo(device, vkt::readBinary("./shaders/render.frag.spv"), vk::ShaderStageFlagBits::eFragment)
    });
    pipelineInfo.graphicsPipelineCreateInfo.layout = finalPipelineLayout;
    pipelineInfo.graphicsPipelineCreateInfo.renderPass = renderPass;
    pipelineInfo.inputAssemblyState.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;
    pipelineInfo.viewportState.pViewports = (VkViewport*)&viewport;
    pipelineInfo.viewportState.pScissors = (vkh::VkRect2D*)&renderArea;

    // Blending
    std::vector<vkh::VkPipelineColorBlendAttachmentState> blendState = {
        vkh::VkPipelineColorBlendAttachmentState{}
    };

    // 
    auto dynamicStates = vkt::vector_cast<VkDynamicState>(std::vector<vk::DynamicState>{vk::DynamicState::eScissor, vk::DynamicState::eViewport});
    pipelineInfo.dynamicState.setDynamicStates(dynamicStates);
    pipelineInfo.colorBlendState.setAttachments(blendState);
    auto finalPipeline = device.createGraphicsPipeline(fw.getPipelineCache(),VkGraphicsPipelineCreateInfo(pipelineInfo));

    //
    auto descriptorSet = device.allocateDescriptorSets(static_cast<vk::DescriptorSetAllocateInfo>(vkh::VkDescriptorSetAllocateInfo{
        .descriptorPool = fw.getDescriptorPool(),
        .pSetLayouts = reinterpret_cast<const VkDescriptorSetLayout*>(&descriptorSetLayout)
    }));

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
            //auto renderArea = vk::Rect2D(vk::Offset2D(0, 0), vk::Extent2D(canvasWidth, canvasHeight));
            //auto viewport = vk::Viewport(0.0f, 0.0f, renderArea.extent.width, renderArea.extent.height, 0, 1.0f);

            // create command buffer (with rewrite)
            vk::CommandBuffer& commandBuffer = framebuffers[n_semaphore].commandBuffer;
            if (!commandBuffer) {
                commandBuffer = vkt::createCommandBuffer(device, commandPool, false, false); // do reference of cmd buffer
                commandBuffer.beginRenderPass(vk::RenderPassBeginInfo(renderPass, framebuffers[currentBuffer].frameBuffer, renderArea, clearValues.size(), clearValues.data()), vk::SubpassContents::eInline);
                commandBuffer.setViewport(0, std::vector<vk::Viewport> { viewport });
                commandBuffer.setScissor(0, std::vector<vk::Rect2D> { renderArea });
                commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, finalPipeline);
                commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, finalPipelineLayout, 0, descriptorSet, nullptr);
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
