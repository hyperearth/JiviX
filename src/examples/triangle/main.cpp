#define VMA_IMPLEMENTATION
#include "vkt2/fw.hpp"

vkt::GPUFramework fw = {};

int main() {

    glfwInit();

	if (GLFW_FALSE == glfwVulkanSupported()) {
		// not supported
		glfwTerminate();
		return -1;
	}

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

	// 
	int currSemaphore = -1;
	uint32_t currentBuffer = 0u;

	// TODO: create graphics pipeline, descriptor set and pipeline layout
	while (!glfwWindowShouldClose(manager.window)) {
        glfwPollEvents();


        // 
        auto n_semaphore = currSemaphore;
        auto c_semaphore = int32_t((size_t(currSemaphore) + 1ull) % framebuffers.size());
        currSemaphore = c_semaphore;

        // acquire next image where will rendered (and get semaphore when will presented finally)
        n_semaphore = (n_semaphore >= 0 ? n_semaphore : (framebuffers.size() - 1));
        device.acquireNextImageKHR(swapchain, std::numeric_limits<uint64_t>::max(), framebuffers[n_semaphore].semaphore, nullptr, &currentBuffer);

        { // Submit rendering (and wait presentation in device)
            std::vector<vk::ClearValue> clearValues = { vk::ClearColorValue(std::array<float,4>{0.f, 0.f, 0.f, 0.0f}), vk::ClearDepthStencilValue(1.0f, 0) };
            auto renderArea = vk::Rect2D(vk::Offset2D(0, 0), vk::Extent2D(canvasWidth, canvasHeight));
            auto viewport = vk::Viewport(0.0f, 0.0f, renderArea.extent.width, renderArea.extent.height, 0, 1.0f);

            // CSreate command buffer (with rewrite)
			// TODO: finalize test application
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

	glfwDestroyWindow(manager.window);
	glfwTerminate();

    return 0;
};
