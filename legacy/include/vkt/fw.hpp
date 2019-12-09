#pragma once

//#ifdef OS_WIN
#if (defined(_WIN32) || defined(__MINGW32__) || defined(_MSC_VER_) || defined(__MINGW64__)) 
#define GLFW_EXPOSE_NATIVE_WIN32
#define GLFW_EXPOSE_NATIVE_WGL
#endif

//#ifdef OS_LNX
#ifdef __linux__
#define GLFW_EXPOSE_NATIVE_X11
#define GLFW_EXPOSE_NATIVE_GLX
#endif

#define ENABLE_EXTENSION_GLM
#define ENABLE_EXTENSION_VMA
#define ENABLE_EXTENSION_RTX
//#define GLFW_INCLUDE_VULKAN
#include "utils.hpp"
#include "structs.hpp"
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

namespace vkt
{

    class GPUFramework {
    protected:


        // instance extensions
        std::vector<const char*> wantedExtensions = {
            "VK_KHR_get_physical_device_properties2",
            "VK_KHR_get_surface_capabilities2",
            "VK_KHR_display",
            "VK_EXT_direct_mode_display",
            "VK_EXT_swapchain_colorspace"
        };

        // default device extensions
        std::vector<const char*> wantedDeviceExtensions = {
            "VK_EXT_swapchain_colorspace",
            "VK_EXT_external_memory_host",
            "VK_EXT_sample_locations",
            "VK_EXT_conservative_rasterization",
            "VK_EXT_hdr_metadata",
            "VK_EXT_queue_family_foreign",
            "VK_EXT_sampler_filter_minmax",
            "VK_EXT_descriptor_indexing",
            "VK_EXT_scalar_block_layout",

            "VK_AMD_gpu_shader_int16",
            "VK_AMD_gpu_shader_half_float",
            "VK_AMD_gcn_shader",
            "VK_AMD_buffer_marker",
            "VK_AMD_shader_info",
            "VK_AMD_texture_gather_bias_lod",
            "VK_AMD_shader_image_load_store_lod",
            "VK_AMD_shader_trinary_minmax",
            "VK_AMD_draw_indirect_count",

            "VK_KHR_16bit_storage",
            "VK_KHR_8bit_storage",
            "VK_KHR_incremental_present",
            "VK_KHR_push_descriptor",
            "VK_KHR_swapchain",
            "VK_KHR_sampler_ycbcr_conversion",
            "VK_KHR_image_format_list",
            "VK_KHR_shader_draw_parameters",
            "VK_KHR_variable_pointers",
            "VK_KHR_dedicated_allocation",
            "VK_KHR_relaxed_block_layout",
            "VK_KHR_descriptor_update_template",
            "VK_KHR_sampler_mirror_clamp_to_edge",
            "VK_KHR_storage_buffer_storage_class",
            "VK_KHR_vulkan_memory_model",
            "VK_KHR_dedicated_allocation",
            "VK_KHR_driver_properties",
            "VK_KHR_get_memory_requirements2",
            "VK_KHR_bind_memory2",
            "VK_KHR_maintenance1",
            "VK_KHR_maintenance2",
            "VK_KHR_maintenance3",
            "VK_KHX_shader_explicit_arithmetic_types",
            "VK_KHR_shader_atomic_int64",
            "VK_KHR_shader_float16_int8",
            "VK_KHR_shader_float_controls",
            "VK_KHR_shader_clock",
            //"VK_KHR_surface",

            "VK_NV_compute_shader_derivatives",
            "VK_NV_corner_sampled_image",
            "VK_NV_shader_image_footprint",
            "VK_NV_shader_subgroup_partitioned",

            "VK_NV_ray_tracing",
        };

        // instance layers
        std::vector<const char*> wantedLayers = {
            "VK_LAYER_LUNARG_assistant_layer",
            "VK_LAYER_LUNARG_standard_validation",
            "VK_LAYER_LUNARG_parameter_validation",
            "VK_LAYER_LUNARG_core_validation",

            //"VK_LAYER_LUNARG_api_dump",
            //"VK_LAYER_LUNARG_object_tracker",
            //"VK_LAYER_LUNARG_device_simulation",
            //"VK_LAYER_GOOGLE_threading",
            //"VK_LAYER_GOOGLE_unique_objects"
            //"VK_LAYER_RENDERDOC_Capture"
        };

        // default device layers
        std::vector<const char*> wantedDeviceValidationLayers = {
            "VK_LAYER_AMD_switchable_graphics"
        };


    public:
        GPUFramework() {};

        InstanceMaker instance = {};
        DeviceMaker device = {};
        MemoryAllocator allocator = {};
        PhysicalDeviceHelper physicalHelper = {};
        ImageMaker depthImageMaker = {};


        api::Fence fence = {};
        api::Queue queue = {};
        api::Device _device = {};
        api::Instance _instance = {};
        api::DescriptorPool _descriptorPool = {};
        api::PhysicalDevice _physicalDevice = {};
        api::CommandPool commandPool = {};
        api::RenderPass renderPass = {};
        api::Image depthImage = {};
        api::ImageView depthImageView = {};
        uint32_t queueFamilyIndex = 0;
        uint32_t instanceVersion = 0;

        std::vector<api::PhysicalDevice> physicalDevices = {};
        std::vector<uint32_t> queueFamilyIndices = {};

        //api::Device createDevice(bool isComputePrior = true, std::string shaderPath = "./", bool enableAdvancedAcceleration = true);
        const api::PhysicalDevice& getPhysicalDevice(const uint32_t& gpuID) { _physicalDevice = physicalDevices[gpuID]; return _physicalDevice; };
        const api::PhysicalDevice& getPhysicalDevice() const { return _physicalDevice; };
        const api::Device& getDevice() const { return _device; };
        const api::Queue& getQueue() const { return queue; };
        const api::Fence& getFence() const { return fence; };
        const api::Instance& getInstance() const { return _instance; };
        const api::CommandPool& getCommandPool() const { return commandPool; };

        void submitCommandWithSync(const api::CommandBuffer & cmdBuf) {
            // submit command
            api::SubmitInfo sbmi = {};
            sbmi.commandBufferCount = 1;//cmdBuffers.size();
            sbmi.pCommandBuffers = &cmdBuf;

            // submit commands
            auto fence = getFence(); {
                getQueue().submit(sbmi, fence);
                _device.waitForFences({ fence }, true, INT32_MAX);
            };
            _device.resetFences({ 1, &fence });
        }

        struct SurfaceWindow {
            SurfaceFormat surfaceFormat = {};
            api::Extent2D surfaceSize = api::Extent2D{ 0u, 0u };
            api::SurfaceKHR surface = {};
            GLFWwindow* window = nullptr;
        } applicationWindow = {};

    public:
        InstanceMaker& createInstance() {


#ifdef VOLK_H_
            volkInitialize();
#endif

            // 
            assert((instanceVersion = vk::enumerateInstanceVersion()) >= VK_MAKE_VERSION(1, 1, 0));

            // get required extensions
            uint32_t glfwExtensionCount = 0;
            const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

            // add glfw extensions to list
            for (uint32_t i = 0; i < glfwExtensionCount; i++) {
                wantedExtensions.push_back(glfwExtensions[i]);
            };

            // get our needed extensions
            auto installedExtensions = api::enumerateInstanceExtensionProperties();
            auto extensions = std::vector<const char*>();
            for (auto w : wantedExtensions) {
                for (auto i : installedExtensions)
                {
                    if (std::string(i.extensionName).compare(w) == 0)
                    {
                        extensions.emplace_back(w);
                        break;
                    }
                }
            }

            // get validation layers
            auto installedLayers = api::enumerateInstanceLayerProperties();
            auto layers = std::vector<const char*>();
            for (auto w : wantedLayers) {
                for (auto i : installedLayers)
                {
                    if (std::string(i.layerName).compare(w) == 0)
                    {
                        layers.emplace_back(w);
                        break;
                    }
                }
            }

            // app info
            auto appinfo = api::ApplicationInfo{};
            appinfo.pNext = nullptr;
            appinfo.pApplicationName = "VKTest";
            appinfo.apiVersion = VK_MAKE_VERSION(1, 1, 126);

            // create instance info
            auto cinstanceinfo = api::InstanceCreateInfo{};
            cinstanceinfo.pApplicationInfo = &appinfo;
            cinstanceinfo.enabledExtensionCount = extensions.size();
            cinstanceinfo.ppEnabledExtensionNames = extensions.data();
            cinstanceinfo.enabledLayerCount = layers.size();
            cinstanceinfo.ppEnabledLayerNames = layers.data();

            // 
            instance = std::make_shared<Instance_T>(cinstanceinfo, &_instance)->create();

            // get physical device for application
            physicalDevices = _instance.enumeratePhysicalDevices();

            // 
            return instance;
        };

        DeviceMaker createDevice(bool isComputePrior, std::string shaderPath, bool enableAdvancedAcceleration) {
            // TODO: merge into Device class 

            // use extensions
            auto deviceExtensions = std::vector<const char*>();
            auto gpuExtensions = _physicalDevice.enumerateDeviceExtensionProperties();
            for (auto w : wantedDeviceExtensions) {
                for (auto i : gpuExtensions) {
                    if (std::string(i.extensionName).compare(w) == 0) {
                        deviceExtensions.emplace_back(w); break;
                    };
                };
            };

            // use layers
            auto layers = std::vector<const char*>();
            auto deviceValidationLayers = std::vector<const char*>();
            auto gpuLayers = _physicalDevice.enumerateDeviceLayerProperties();
            for (auto w : wantedLayers) {
                for (auto i : gpuLayers) {
                    if (std::string(i.layerName).compare(w) == 0) {
                        layers.emplace_back(w); break;
                    };
                };
            };

            // minimal features
            auto gStorage16 = api::PhysicalDevice16BitStorageFeatures{};
            auto gStorage8 = api::PhysicalDevice8BitStorageFeaturesKHR{};
            auto gDescIndexing = api::PhysicalDeviceDescriptorIndexingFeaturesEXT{};
            gStorage16.pNext = &gStorage8;
            gStorage8.pNext = &gDescIndexing;

            // 
            auto gFeatures = api::PhysicalDeviceFeatures2{};
            gFeatures.pNext = &gStorage16;
            gFeatures.features.shaderInt16 = true;
            gFeatures.features.shaderInt64 = true;
            gFeatures.features.shaderUniformBufferArrayDynamicIndexing = true;
            _physicalDevice.getFeatures2(&gFeatures);

            // get features and queue family properties
            //auto gpuFeatures = gpu.getFeatures();
            auto gpuQueueProps = _physicalDevice.getQueueFamilyProperties();

            // queue family initial
            float priority = 1.0f;
            uint32_t computeFamilyIndex = -1, graphicsFamilyIndex = -1;
            auto queueCreateInfos = std::vector<api::DeviceQueueCreateInfo>();

            // compute/graphics queue family
            for (auto queuefamily : gpuQueueProps) {
                graphicsFamilyIndex++;
                if (queuefamily.queueFlags & (api::QueueFlagBits::eCompute) && queuefamily.queueFlags & (api::QueueFlagBits::eGraphics) && _physicalDevice.getSurfaceSupportKHR(graphicsFamilyIndex, surface())) {
                    queueCreateInfos.push_back(api::DeviceQueueCreateInfo(api::DeviceQueueCreateFlags()).setQueueFamilyIndex(graphicsFamilyIndex).setQueueCount(1).setPQueuePriorities(&priority));
                    queueFamilyIndices.push_back(graphicsFamilyIndex);
                    break;
                };
            };

            // if have supported queue family, then use this device
            if (queueCreateInfos.size() > 0) {
                this->physicalHelper = std::make_shared<PhysicalDevice_T>(instance, this->_physicalDevice);
                this->device = std::make_shared<Device_T>(this->physicalHelper, api::DeviceCreateInfo().setFlags(api::DeviceCreateFlags())
                    .setPNext(&gFeatures) //.setPEnabledFeatures(&gpuFeatures)
                    .setPQueueCreateInfos(queueCreateInfos.data()).setQueueCreateInfoCount(queueCreateInfos.size())
                    .setPpEnabledExtensionNames(deviceExtensions.data()).setEnabledExtensionCount(deviceExtensions.size())
                    .setPpEnabledLayerNames(deviceValidationLayers.data()).setEnabledLayerCount(deviceValidationLayers.size()), &_device); // already created device now!
            };


            //api::PipelineCacheCreateInfo cacheInfo = {};
            //cacheInfo.initialDataSize = 32768u;
            //auto cache = _device.createPipelineCache(cacheInfo);

            // return device with queue pointer
            const uint32_t qptr = 0;
            this->queueFamilyIndex = queueFamilyIndices[qptr];
            this->device->linkPhysicalHelper(this->physicalHelper)->create()->cache(std::vector<uint8_t>{ 0u,0u,0u,0u });
            this->queue = this->_device.getQueue(queueFamilyIndex, 0); // 
            this->fence = this->_device.createFence(api::FenceCreateInfo().setFlags({}));
            this->commandPool = this->_device.createCommandPool(api::CommandPoolCreateInfo(api::CommandPoolCreateFlags(api::CommandPoolCreateFlagBits::eResetCommandBuffer), queueFamilyIndex));
            this->allocator = this->device->createAllocator<VMAllocator_T>()->initialize();

            // Manually Create Descriptor Pool
            auto dps = std::vector<api::DescriptorPoolSize>{
                api::DescriptorPoolSize().setType(vk::DescriptorType::eCombinedImageSampler).setDescriptorCount(256u),
                api::DescriptorPoolSize().setType(vk::DescriptorType::eSampledImage).setDescriptorCount(1024u),
                api::DescriptorPoolSize().setType(vk::DescriptorType::eSampler).setDescriptorCount(1024u),
                api::DescriptorPoolSize().setType(vk::DescriptorType::eAccelerationStructureNV).setDescriptorCount(256u),
                api::DescriptorPoolSize().setType(vk::DescriptorType::eStorageBuffer).setDescriptorCount(1024u),
                api::DescriptorPoolSize().setType(vk::DescriptorType::eUniformBuffer).setDescriptorCount(256u),
                api::DescriptorPoolSize().setType(vk::DescriptorType::eStorageTexelBuffer).setDescriptorCount(256u),
                api::DescriptorPoolSize().setType(vk::DescriptorType::eUniformTexelBuffer).setDescriptorCount(256u),
            };

            // Un-Deviced
            return this->device->linkDescriptorPool(&(this->_descriptorPool = this->device->least().createDescriptorPool(api::DescriptorPoolCreateInfo().setMaxSets(256u).setPPoolSizes(dps.data()).setPoolSizeCount(dps.size()))));
        };

        // create window and surface for this application (multi-window not supported)
        inline SurfaceWindow& createWindowSurface(GLFWwindow * window, uint32_t WIDTH, uint32_t HEIGHT, std::string title = "TestApp") {
            applicationWindow.window = window;
            applicationWindow.surfaceSize = api::Extent2D{ WIDTH, HEIGHT };
            auto result = glfwCreateWindowSurface((VkInstance&)(_instance), applicationWindow.window, nullptr, (VkSurfaceKHR*)& applicationWindow.surface);
            if (result != VK_SUCCESS) { glfwTerminate(); exit(result); };
            return applicationWindow;
        }

        // create window and surface for this application (multi-window not supported)
        inline SurfaceWindow& createWindowSurface(uint32_t WIDTH, uint32_t HEIGHT, std::string title = "TestApp") {
            applicationWindow.window = glfwCreateWindow(WIDTH, HEIGHT, title.c_str(), nullptr, nullptr);
            applicationWindow.surfaceSize = api::Extent2D{ WIDTH, HEIGHT };
            auto result = glfwCreateWindowSurface((VkInstance&)(_instance), applicationWindow.window, nullptr, (VkSurfaceKHR*)& applicationWindow.surface);
            if (result != VK_SUCCESS) { glfwTerminate(); exit(result); };
            return applicationWindow;
        }

        // getters
        api::SurfaceKHR surface() const { return applicationWindow.surface; }
        GLFWwindow* window() const { return applicationWindow.window; }
        const SurfaceFormat& format() const { return applicationWindow.surfaceFormat; }
        const api::Extent2D& size() const { return applicationWindow.surfaceSize; }

        // setters
        void format(SurfaceFormat format) { applicationWindow.surfaceFormat = format; }
        void size(const api::Extent2D & size) { applicationWindow.surfaceSize = size; }

        // 
        inline SurfaceFormat getSurfaceFormat(api::PhysicalDevice gpu)
        {
            auto surfaceFormats = gpu.getSurfaceFormatsKHR(applicationWindow.surface);

            const std::vector<api::Format> preferredFormats = { api::Format::eR8G8B8A8Unorm, api::Format::eB8G8R8A8Unorm };

            api::Format surfaceColorFormat =
                surfaceFormats.size() == 1 &&
                surfaceFormats[0].format == api::Format::eUndefined
                ? api::Format::eR8G8B8A8Unorm
                : surfaceFormats[0].format;

            // search preferred surface format support
            bool surfaceFormatFound = false;
            uint32_t surfaceFormatID = 0;
            for (int i = 0; i < preferredFormats.size(); i++)
            {
                if (surfaceFormatFound) break;
                for (int f = 0; f < surfaceFormats.size(); f++)
                {
                    if (surfaceFormats[f].format == preferredFormats[i])
                    {
                        surfaceFormatFound = true;
                        surfaceFormatID = f;
                        break;
                    }
                }
            }

            // get supported color format
            surfaceColorFormat = surfaceFormats[surfaceFormatID].format;
            api::ColorSpaceKHR surfaceColorSpace = surfaceFormats[surfaceFormatID].colorSpace;

            // get format properties?
            auto formatProperties = gpu.getFormatProperties(surfaceColorFormat);

            // only if these depth formats
            std::vector<api::Format> depthFormats = {
                api::Format::eD32SfloatS8Uint, api::Format::eD32Sfloat,
                api::Format::eD24UnormS8Uint, api::Format::eD16UnormS8Uint,
                api::Format::eD16Unorm };

            // choice supported depth format
            api::Format surfaceDepthFormat = depthFormats[0];
            for (auto format : depthFormats) {
                auto depthFormatProperties = gpu.getFormatProperties(format);
                if (depthFormatProperties.optimalTilingFeatures & api::FormatFeatureFlagBits::eDepthStencilAttachment) {
                    surfaceDepthFormat = format; break;
                }
            }

            // return format result
            SurfaceFormat sfd = {};
            sfd.colorSpace = surfaceColorSpace;
            sfd.colorFormat = surfaceColorFormat;
            sfd.depthFormat = surfaceDepthFormat;
            sfd.colorFormatProperties = formatProperties; // get properties about format
            return sfd;
        }

        inline api::RenderPass& createRenderPass()
        {
            auto formats = applicationWindow.surfaceFormat;
            auto rps = device->createRenderPassMaker({}, &renderPass);

            // 
            rps->addAttachment(formats.colorFormat)->getAttachmentDescription()
                .setSamples(api::SampleCountFlagBits::e1)
                .setLoadOp(api::AttachmentLoadOp::eLoad)
                .setStoreOp(api::AttachmentStoreOp::eStore)
                .setStencilLoadOp(api::AttachmentLoadOp::eDontCare)
                .setStencilStoreOp(api::AttachmentStoreOp::eDontCare)
                .setInitialLayout(api::ImageLayout::eUndefined)
                .setFinalLayout(api::ImageLayout::ePresentSrcKHR);

            // 
            rps->addAttachment(formats.depthFormat)->getAttachmentDescription()
                .setSamples(api::SampleCountFlagBits::e1)
                .setLoadOp(api::AttachmentLoadOp::eClear)
                .setStoreOp(api::AttachmentStoreOp::eDontCare)
                .setStencilLoadOp(api::AttachmentLoadOp::eDontCare)
                .setStencilStoreOp(api::AttachmentStoreOp::eDontCare)
                .setInitialLayout(api::ImageLayout::eUndefined)
                .setFinalLayout(api::ImageLayout::eDepthStencilAttachmentOptimal);

            // 
            rps->addSubpass(api::PipelineBindPoint::eGraphics)->getSubpassDescription();
            rps->subpassColorAttachment(0u, api::ImageLayout::eColorAttachmentOptimal);
            rps->subpassDepthStencilAttachment(1u, api::ImageLayout::eDepthStencilAttachmentOptimal);

            // 
            rps->addDependency(VK_SUBPASS_EXTERNAL, 0u)->getSubpassDependency()
                .setDependencyFlags(api::DependencyFlagBits::eByRegion)
                .setSrcStageMask(api::PipelineStageFlagBits::eColorAttachmentOutput | api::PipelineStageFlagBits::eBottomOfPipe | api::PipelineStageFlagBits::eTransfer)
                .setSrcAccessMask(api::AccessFlagBits::eColorAttachmentWrite)
                .setDstStageMask(api::PipelineStageFlagBits::eColorAttachmentOutput)
                .setDstAccessMask(api::AccessFlagBits::eColorAttachmentRead | api::AccessFlagBits::eColorAttachmentWrite);

            // 
            rps->addDependency(0u, VK_SUBPASS_EXTERNAL)->getSubpassDependency()
                .setDependencyFlags(api::DependencyFlagBits::eByRegion)
                .setSrcStageMask(api::PipelineStageFlagBits::eColorAttachmentOutput)
                .setSrcAccessMask(api::AccessFlagBits::eColorAttachmentRead | api::AccessFlagBits::eColorAttachmentWrite)
                .setDstStageMask(api::PipelineStageFlagBits::eColorAttachmentOutput | api::PipelineStageFlagBits::eTopOfPipe | api::PipelineStageFlagBits::eTransfer)
                .setDstAccessMask(api::AccessFlagBits::eColorAttachmentRead | api::AccessFlagBits::eColorAttachmentWrite);

            // create renderpass finally
            return rps->create()->getRenderPass();
        }

        // update swapchain framebuffer
        inline void updateSwapchainFramebuffer(std::vector<Framebuffer> & swapchainBuffers, api::SwapchainKHR & swapchain, api::RenderPass & renderpass)
        {
            // The swapchain handles allocating frame images.
            auto formats = applicationWindow.surfaceFormat;
            auto gpuMemoryProps = _physicalDevice.getMemoryProperties();

            // 
            auto imageInfoVK = api::ImageCreateInfo{};
            imageInfoVK.initialLayout = api::ImageLayout::eUndefined;
            imageInfoVK.sharingMode = api::SharingMode::eExclusive;
            imageInfoVK.flags = {};
            imageInfoVK.pNext = nullptr;
            imageInfoVK.arrayLayers = 1;
            imageInfoVK.extent = vk::Extent3D{ applicationWindow.surfaceSize.width, applicationWindow.surfaceSize.height, 1u };
            imageInfoVK.format = { formats.depthFormat };
            imageInfoVK.imageType = api::ImageType::e2D;
            imageInfoVK.mipLevels = 1;
            imageInfoVK.samples = api::SampleCountFlagBits::e1;
            imageInfoVK.tiling = api::ImageTiling::eOptimal;
            imageInfoVK.usage = api::ImageUsageFlagBits::eDepthStencilAttachment|api::ImageUsageFlagBits::eTransferSrc;

            // 
            VmaAllocationCreateInfo allocCreateInfo = {};
            allocCreateInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;

            // next-gen create image
            if (depthImageMaker) { depthImageMaker->free(); }; // use smart free 
            depthImageMaker = device->createImageMaker(imageInfoVK, &depthImage)
                ->setImageSubresourceRange(api::ImageSubresourceRange{ api::ImageAspectFlagBits::eDepth | api::ImageAspectFlagBits::eStencil, 0, 1, 0, 1 })
                ->create2D(formats.depthFormat, applicationWindow.surfaceSize.width, applicationWindow.surfaceSize.height)->allocate(allocator, (uintptr_t)&allocCreateInfo)
                ->createImageView(&depthImageView,api::ImageViewType::e2D,formats.depthFormat )
                ->allocate((uintptr_t)(&allocCreateInfo));

            // 
            auto swapchainImages = _device.getSwapchainImagesKHR(swapchain);
            swapchainBuffers.resize(swapchainImages.size());
            for (int i = 0; i < swapchainImages.size(); i++)
            { // create framebuffers
                std::array<api::ImageView, 2> views = {}; // predeclare views
                views[0] = _device.createImageView(api::ImageViewCreateInfo{ {}, swapchainImages[i], api::ImageViewType::e2D, formats.colorFormat, api::ComponentMapping(), api::ImageSubresourceRange{api::ImageAspectFlagBits::eColor, 0, 1, 0, 1} }); // color view
                views[1] = depthImageView; // depth view
                swapchainBuffers[i].frameBuffer = _device.createFramebuffer(api::FramebufferCreateInfo{ {}, renderpass, uint32_t(views.size()), views.data(), applicationWindow.surfaceSize.width, applicationWindow.surfaceSize.height, 1u });
            };
        }

        inline std::vector<Framebuffer> createSwapchainFramebuffer(api::SwapchainKHR swapchain, api::RenderPass renderpass) {
            // framebuffers vector
            std::vector<Framebuffer> swapchainBuffers = {};
            updateSwapchainFramebuffer(swapchainBuffers, swapchain, renderpass);
            for (int i = 0; i < swapchainBuffers.size(); i++)
            { // create semaphore
                swapchainBuffers[i].semaphore = device->least().createSemaphore(api::SemaphoreCreateInfo());
                swapchainBuffers[i].waitFence = device->least().createFence(api::FenceCreateInfo().setFlags(api::FenceCreateFlagBits::eSignaled));
            };
            return swapchainBuffers;
        }

        // create swapchain template
        inline api::SwapchainKHR createSwapchain()
        {
            api::SurfaceKHR surface = applicationWindow.surface;
            SurfaceFormat& formats = applicationWindow.surfaceFormat;

            auto surfaceCapabilities = _physicalDevice.getSurfaceCapabilitiesKHR(surface);
            auto surfacePresentModes = _physicalDevice.getSurfacePresentModesKHR(surface);

            // check the surface width/height.
            if (!(surfaceCapabilities.currentExtent.width == -1 ||
                surfaceCapabilities.currentExtent.height == -1))
            {
                applicationWindow.surfaceSize = surfaceCapabilities.currentExtent;
            }

            // get supported present mode, but prefer mailBox
            auto presentMode = api::PresentModeKHR::eImmediate;
            std::vector<api::PresentModeKHR> priorityModes = { api::PresentModeKHR::eImmediate, api::PresentModeKHR::eMailbox, api::PresentModeKHR::eFifoRelaxed, api::PresentModeKHR::eFifo };

            bool found = false;
            for (auto pm : priorityModes) {
                if (found) break;
                for (auto sfm : surfacePresentModes) { if (pm == sfm) { presentMode = pm; found = true; break; } }
            }

            // swapchain info
            auto swapchainCreateInfo = api::SwapchainCreateInfoKHR();
            swapchainCreateInfo.surface = surface;
            swapchainCreateInfo.minImageCount = std::min(surfaceCapabilities.maxImageCount, 3u);
            swapchainCreateInfo.imageFormat = formats.colorFormat;
            swapchainCreateInfo.imageColorSpace = formats.colorSpace;
            swapchainCreateInfo.imageExtent = applicationWindow.surfaceSize;
            swapchainCreateInfo.imageArrayLayers = 1;
            swapchainCreateInfo.imageUsage = api::ImageUsageFlagBits::eColorAttachment;
            swapchainCreateInfo.imageSharingMode = api::SharingMode::eExclusive;
            swapchainCreateInfo.preTransform = api::SurfaceTransformFlagBitsKHR::eIdentity;
            swapchainCreateInfo.compositeAlpha = api::CompositeAlphaFlagBitsKHR::eOpaque;
            swapchainCreateInfo.presentMode = presentMode;
            swapchainCreateInfo.clipped = true;

            // create swapchain
            return device->least().createSwapchainKHR(swapchainCreateInfo, nullptr);
        }
    };

}; // namespace NSM
