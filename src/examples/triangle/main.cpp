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

	int width = 800;
	int height = 600;
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	//auto window = glfwCreateWindow(width, height, application_info.pApplicationName, nullptr, nullptr);

	fw.createInstance();
	fw.createWindowSurface(800u, 600u);
	auto physicalDevice = fw.getPhysicalDevice(0u);
	fw.createDevice(true,"./",false);
	//auto surfaceFormat = fw.getSurfaceFormat();
	fw.createSwapchain();

    return 0;
};
