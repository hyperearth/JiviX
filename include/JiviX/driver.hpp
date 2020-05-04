#pragma once

#include "./core.hpp"
#include "./JVI/config.hpp"
#include "./JVI/driver.hpp"


namespace jvx { 
    class Driver : public Wrap<jvi::Driver> { public: using T = jvi::Driver;
        Driver() { object = std::make_shared<jvi::Driver>(); };
        Driver(vkt::uni_ptr<jvi::Driver> obj) { this->object = obj; };
        Driver(std::shared_ptr<jvi::Driver> obj) { this->object = obj; };

        CALLIFY(getPhysicalDevice);
        CALLIFY(getDispatch);
        CALLIFY(getDevice);
        CALLIFY(getQueue);
        CALLIFY(getFence);
        CALLIFY(getInstance);
        CALLIFY(getCommandPool);
        CALLIFY(getPipelineCache);
        CALLIFY(getDescriptorPool);
        CALLIFY(getMemoryProperties);
        CALLIFY(getAllocator);
        //CALLIFY(submitCommandWithSync);
        CALLIFY(getDepthImageView);
        CALLIFY(getDepthImage);
        CALLIFY(createInstance);
        CALLIFY(createDevice);
        CALLIFY(getInstanceCreateInfoAddress);
        CALLIFY(getDeviceCreateInfoAddress);
        //CALLIFY(initializeGL);

#ifdef VKT_ENABLE_GLFW_SUPPORT
        CALLIFY(createWindowSurface);
        CALLIFY(surface);
        CALLIFY(window);
        CALLIFY(format);
        CALLIFY(size);
        CALLIFY(getSurfaceFormat);
        CALLIFY(createRenderPass);
        CALLIFY(updateSwapchainFramebuffer);
        CALLIFY(createSwapchainFramebuffer);
        CALLIFY(createSwapchain);
#endif
    };
};
