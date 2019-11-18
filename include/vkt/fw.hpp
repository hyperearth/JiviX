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

//#define VRT_IMPLEMENTATION
#include "utils.hpp"
#include "structs.hpp"
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>
#include <lancer/lib/core.hpp>
#include <lancer/API/memory.hpp>
#include <lancer/API/device.hpp>
#include <lancer/API/VMA.hpp>

namespace vkt
{

    class ComputeFramework {
    protected:


        // instance extensions
        std::vector<const char*> wantedExtensions = {
            "VK_KHR_get_physical_device_properties2",
            "VK_KHR_get_surface_capabilities2",
            "VK_KHR_display", "VK_KHR_surface",
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
        ComputeFramework() {};

        std::shared_ptr<Instance> instance = {};
        std::shared_ptr<Device> device = {};
        std::shared_ptr<Allocator> allocator = {};
        api::Fence fence = {};
        api::Queue queue = {};
        api::Device _device = {};
        api::Instance _instance = {};
        api::DescriptorPool descriptorPool = {};
        api::PhysicalDevice physicalDevice = {};
        api::CommandPool commandPool = {};
        api::RenderPass renderpass = {};
        api::Image depthImage = {};
        api::ImageView depthImageView = {};
        uint32_t queueFamilyIndex = 0;

        std::vector<api::PhysicalDevice> physicalDevices = {};
        std::vector<uint32_t> queueFamilyIndices = {};

        //api::Device createDevice(bool isComputePrior = true, std::string shaderPath = "./", bool enableAdvancedAcceleration = true);

        const api::PhysicalDevice& getPhysicalDevice(const uint32_t& gpuID) { return (physicalDevice = physicalDevices[gpuID]); };
        const api::PhysicalDevice& getPhysicalDevice() const { return this->physicalDevice; };
        const api::Device& getDevice() const { return this->device; };
        const api::Queue& getQueue() const { return this->queue; };
        const api::Fence& getFence() const { return this->fence; };
        const api::Instance& getInstance() const { return this->instance; };
        const api::CommandPool& getCommandPool() const { return this->commandPool; };

        void submitCommandWithSync(const api::CommandBuffer & cmdBuf) {
            // submit command
            api::SubmitInfo sbmi = {};
            sbmi.commandBufferCount = 1;//cmdBuffers.size();
            sbmi.pCommandBuffers = &cmdBuf;

            // submit commands
            auto fence = getFence(); {
                getQueue().submit(sbmi, fence);
                device.waitForFences({ fence }, true, INT32_MAX);
            };
            device.resetFences({ 1, &fence });
        }

        struct SurfaceWindow {
            SurfaceFormat surfaceFormat = {};
            api::Extent2D surfaceSize = api::Extent2D{ 0u, 0u };
            api::SurfaceKHR surface = {};
            GLFWwindow* window = nullptr;
        } applicationWindow = {};

    public:
        std::shared_ptr<Instance> createInstance() {

#ifdef VOLK_H_
            volkInitialize();
#endif

            auto supportedVkApiVersion = 0u;
            auto apiResult = vkEnumerateInstanceVersion(&supportedVkApiVersion);
            if (supportedVkApiVersion < VK_MAKE_VERSION(1, 1, 0)) return instance;

            // get required extensions
            unsigned int glfwExtensionCount = 0;
            const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

            // add glfw extensions to list
            for (uint32_t i = 0; i < glfwExtensionCount; i++) {
                wantedExtensions.push_back(glfwExtensions[i]);
            }

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
            auto appinfo = VkApplicationInfo(api::ApplicationInfo{});
            appinfo.pNext = nullptr;
            appinfo.pApplicationName = "VKTest";
            appinfo.apiVersion = VK_MAKE_VERSION(1, 1, 126);

            // create instance info
            auto cinstanceinfo = VkInstanceCreateInfo(api::InstanceCreateInfo{});
            cinstanceinfo.pApplicationInfo = &appinfo;
            cinstanceinfo.enabledExtensionCount = extensions.size();
            cinstanceinfo.ppEnabledExtensionNames = extensions.data();
            cinstanceinfo.enabledLayerCount = layers.size();
            cinstanceinfo.ppEnabledLayerNames = layers.data();

            // 
            instance = std::make_shared<Instance>(&_instance, cinstanceinfo);//->Create();

            // get physical device for application
            physicalDevices = _instance.enumeratePhysicalDevices();

            // 
            return instance;
        };

        std::shared_ptr<Device>& createDevice(bool isComputePrior, std::string shaderPath, bool enableAdvancedAcceleration) {
            // TODO: merge into Device class 

            // use extensions
            auto deviceExtensions = std::vector<const char*>();
            auto gpuExtensions = physicalDevice.enumerateDeviceExtensionProperties();
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
            auto gpuLayers = physicalDevice.enumerateDeviceLayerProperties();
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

            auto gFeatures = api::PhysicalDeviceFeatures2{};
            gFeatures.pNext = &gStorage16;
            gFeatures.features.shaderInt16 = true;
            gFeatures.features.shaderInt64 = true;
            gFeatures.features.shaderUniformBufferArrayDynamicIndexing = true;
            physicalDevice.getFeatures2(&gFeatures);

            // get features and queue family properties
            //auto gpuFeatures = gpu.getFeatures();
            auto gpuQueueProps = physicalDevice.getQueueFamilyProperties();

            // queue family initial
            float priority = 1.0f;
            uint32_t computeFamilyIndex = -1, graphicsFamilyIndex = -1;
            auto queueCreateInfos = std::vector<api::DeviceQueueCreateInfo>();

            // compute/graphics queue family
            for (auto queuefamily : gpuQueueProps) {
                graphicsFamilyIndex++;
                if (queuefamily.queueFlags & (api::QueueFlagBits::eCompute) && queuefamily.queueFlags & (api::QueueFlagBits::eGraphics) && physicalDevice.getSurfaceSupportKHR(graphicsFamilyIndex, surface())) {
                    queueCreateInfos.push_back(api::DeviceQueueCreateInfo(api::DeviceQueueCreateFlags()).setQueueFamilyIndex(graphicsFamilyIndex).setQueueCount(1).setPQueuePriorities(&priority));
                    queueFamilyIndices.push_back(graphicsFamilyIndex);
                    break;
                };
            };

            // if have supported queue family, then use this device
            if (queueCreateInfos.size() > 0) {
                this->physicalHelper = std::make_shared<PhysicalDeviceHelper>(this->physicalDevice = physicalDevice);
                this->device = std::make_shared<Device>(this->physicalHelper, &_device, api::DeviceCreateInfo().setFlags(api::DeviceCreateFlags())
                    .setPNext(&gFeatures) //.setPEnabledFeatures(&gpuFeatures)
                    .setPQueueCreateInfos(queueCreateInfos.data()).setQueueCreateInfoCount(queueCreateInfos.size())
                    .setPpEnabledExtensionNames(deviceExtensions.data()).setEnabledExtensionCount(deviceExtensions.size())
                    .setPpEnabledLayerNames(deviceValidationLayers.data()).setEnabledLayerCount(deviceValidationLayers.size()));
            };

            // return device with queue pointer
            const uint32_t qptr = 0;
            this->fence = this->_device.createFence(api::FenceCreateInfo().setFlags({}));
            this->queueFamilyIndex = queueFamilyIndices[qptr];
            this->commandPool = this->_device.createCommandPool(api::CommandPoolCreateInfo(api::CommandPoolCreateFlags(api::CommandPoolCreateFlagBits::eResetCommandBuffer), queueFamilyIndex));
            this->queue = this->_device.getQueue(queueFamilyIndex, 0); // 
            return device
                ->Link(_device)
                ->LinkPhysicalHelper(this->physicalHelper)
                ->LinkDescriptorPool(this->descriptorPool)
                ->LinkAllocator(this->allocator=std::make_shared<VMAllocator>(device))->Initialize(); // Finally Initiate Device 
        };

        // create window and surface for this application (multi-window not supported)
        inline SurfaceWindow& createWindowSurface(GLFWwindow * window, uint32_t WIDTH, uint32_t HEIGHT, std::string title = "TestApp") {
            applicationWindow.window = window;
            applicationWindow.surfaceSize = api::Extent2D{ WIDTH, HEIGHT };
            auto result = glfwCreateWindowSurface(instance, applicationWindow.window, nullptr, (VkSurfaceKHR*)& applicationWindow.surface);
            if (result != VK_SUCCESS) { glfwTerminate(); exit(result); };
            return applicationWindow;
        }

        // create window and surface for this application (multi-window not supported)
        inline SurfaceWindow& createWindowSurface(uint32_t WIDTH, uint32_t HEIGHT, std::string title = "TestApp") {
            applicationWindow.window = glfwCreateWindow(WIDTH, HEIGHT, title.c_str(), nullptr, nullptr);
            applicationWindow.surfaceSize = api::Extent2D{ WIDTH, HEIGHT };
            auto result = glfwCreateWindowSurface(instance, applicationWindow.window, nullptr, (VkSurfaceKHR*)& applicationWindow.surface);
            if (result != VK_SUCCESS) { glfwTerminate(); exit(result); };
            return applicationWindow;
        }

        // getters
        api::SurfaceKHR surface() const {
            return applicationWindow.surface;
        }

        GLFWwindow* window() const {
            return applicationWindow.window;
        }

        const SurfaceFormat& format() const {
            return applicationWindow.surfaceFormat;
        }

        const api::Extent2D& size() const {
            return applicationWindow.surfaceSize;
        }


        // setters
        void format(SurfaceFormat format) {
            applicationWindow.surfaceFormat = format;
        }

        void size(const api::Extent2D & size) {
            applicationWindow.surfaceSize = size;
        }


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

        inline api::RenderPass createRenderpass()
        {
            auto formats = applicationWindow.surfaceFormat;

            // attachments
            std::vector<api::AttachmentDescription> attachmentDescriptions = {

                api::AttachmentDescription()
                    .setFormat(formats.colorFormat)
                    .setSamples(api::SampleCountFlagBits::e1)
                    .setLoadOp(api::AttachmentLoadOp::eLoad)
                    .setStoreOp(api::AttachmentStoreOp::eStore)
                    .setStencilLoadOp(api::AttachmentLoadOp::eDontCare)
                    .setStencilStoreOp(api::AttachmentStoreOp::eDontCare)
                    .setInitialLayout(api::ImageLayout::eUndefined)
                    .setFinalLayout(api::ImageLayout::ePresentSrcKHR),

                api::AttachmentDescription()
                    .setFormat(formats.depthFormat)
                    .setSamples(api::SampleCountFlagBits::e1)
                    .setLoadOp(api::AttachmentLoadOp::eClear)
                    .setStoreOp(api::AttachmentStoreOp::eDontCare)
                    .setStencilLoadOp(api::AttachmentLoadOp::eDontCare)
                    .setStencilStoreOp(api::AttachmentStoreOp::eDontCare)
                    .setInitialLayout(api::ImageLayout::eUndefined)
                    .setFinalLayout(api::ImageLayout::eDepthStencilAttachmentOptimal)
            };

            // attachments references
            std::vector<api::AttachmentReference> colorReferences = { api::AttachmentReference(0, api::ImageLayout::eColorAttachmentOptimal) };
            std::vector<api::AttachmentReference> depthReferences = { api::AttachmentReference(1, api::ImageLayout::eDepthStencilAttachmentOptimal) };

            // subpasses desc
            std::vector<api::SubpassDescription> subpasses = {
                api::SubpassDescription()
                    .setPipelineBindPoint(api::PipelineBindPoint::eGraphics)
                    .setPColorAttachments(colorReferences.data())
                    .setColorAttachmentCount(colorReferences.size())
                    .setPDepthStencilAttachment(depthReferences.data()) };

            // dependency
            std::vector<api::SubpassDependency> dependencies = {
                api::SubpassDependency()
                    .setDependencyFlags(api::DependencyFlagBits::eByRegion)
                    .setSrcSubpass(VK_SUBPASS_EXTERNAL)
                    .setSrcStageMask(api::PipelineStageFlagBits::eColorAttachmentOutput |
                                     api::PipelineStageFlagBits::eBottomOfPipe |
                                     api::PipelineStageFlagBits::eTransfer)
                    .setSrcAccessMask(api::AccessFlagBits::eColorAttachmentWrite)

                    .setDstSubpass(0)
                    .setDstStageMask(api::PipelineStageFlagBits::eColorAttachmentOutput)
                    .setDstAccessMask(api::AccessFlagBits::eColorAttachmentRead |
                                      api::AccessFlagBits::eColorAttachmentWrite),

                api::SubpassDependency()
                    .setDependencyFlags(api::DependencyFlagBits::eByRegion)
                    .setSrcSubpass(0)
                    .setSrcStageMask(api::PipelineStageFlagBits::eColorAttachmentOutput)
                    .setSrcAccessMask(api::AccessFlagBits::eColorAttachmentRead |
                                      api::AccessFlagBits::eColorAttachmentWrite)

                    .setDstSubpass(VK_SUBPASS_EXTERNAL)
                    .setDstStageMask(api::PipelineStageFlagBits::eColorAttachmentOutput |
                                     api::PipelineStageFlagBits::eTopOfPipe |
                                     api::PipelineStageFlagBits::eTransfer)
                    .setDstAccessMask(api::AccessFlagBits::eColorAttachmentRead |
                                      api::AccessFlagBits::eColorAttachmentWrite)

            };

            // create renderpass
            return (renderpass = device.createRenderPass(api::RenderPassCreateInfo(
                api::RenderPassCreateFlags(), attachmentDescriptions.size(),
                attachmentDescriptions.data(), subpasses.size(), subpasses.data(),
                dependencies.size(), dependencies.data())));
        }

        // update swapchain framebuffer
        inline void updateSwapchainFramebuffer(api::SwapchainKHR & swapchain, api::RenderPass & renderpass, std::vector<Framebuffer> & swapchainBuffers)
        {
            // The swapchain handles allocating frame images.
            auto formats = applicationWindow.surfaceFormat;
            auto gpuMemoryProps = physicalDevice.getMemoryProperties();

            // 
            auto imageInfoVK = api::ImageCreateInfo{};
            imageInfoVK.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            imageInfoVK.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
            imageInfoVK.flags = 0;
            imageInfoVK.pNext = nullptr;
            imageInfoVK.arrayLayers = 1;
            imageInfoVK.extent = { applicationWindow.surfaceSize.width, applicationWindow.surfaceSize.height, 1 };
            imageInfoVK.format = { formats.depthFormat };
            imageInfoVK.imageType = VK_IMAGE_TYPE_2D;
            imageInfoVK.mipLevels = 1;
            imageInfoVK.samples = VK_SAMPLE_COUNT_1_BIT;
            imageInfoVK.tiling = VK_IMAGE_TILING_OPTIMAL;
            imageInfoVK.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;

            // 
            VmaAllocationCreateInfo allocCreateInfo = {};
            allocCreateInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;

            // next-gen create image
            auto imageMaker = std::make_shared<Image>(device, &depthImage, imageInfoVK);
            imageMaker->Create2D(formats.depthFormat, applicationWindow.surfaceSize.width, applicationWindow.surfaceSize.height)->Allocate(allocator, (uintptr_t)&allocCreateInfo); // 

            // image view for usage
            auto vinfo = api::ImageViewCreateInfo{};
            vinfo.subresourceRange = api::ImageSubresourceRange{ api::ImageAspectFlagBits::eDepth | api::ImageAspectFlagBits::eStencil, 0, 1, 0, 1 };
            vinfo.flags = 0;
            vinfo.pNext = nullptr;
            vinfo.components = VkComponentMapping{ VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G, VK_COMPONENT_SWIZZLE_B, VK_COMPONENT_SWIZZLE_A };
            vinfo.format = VkFormat(formats.depthFormat);
            vinfo.image = depthImage;
            vinfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
            depthImageView = api::Device(*device).createImageView(api::ImageViewCreateInfo(vinfo));

            // 
            auto swapchainImages = api::Device(*device).getSwapchainImagesKHR(swapchain);
            swapchainBuffers.resize(swapchainImages.size());
            for (int i = 0; i < swapchainImages.size(); i++)
            { // create framebuffers
                api::Image image = swapchainImages[i]; // prelink images
                std::array<api::ImageView, 2> views = {}; // predeclare views
                views[0] = api::Device(*device).createImageView(api::ImageViewCreateInfo{ {}, image, api::ImageViewType::e2D, formats.colorFormat, api::ComponentMapping(), api::ImageSubresourceRange{api::ImageAspectFlagBits::eColor, 0, 1, 0, 1} }); // color view
                views[1] = depthImageView; // depth view
                swapchainBuffers[i].frameBuffer = api::Device(*device).createFramebuffer(api::FramebufferCreateInfo{ {}, renderpass, uint32_t(views.size()), views.data(), applicationWindow.surfaceSize.width, applicationWindow.surfaceSize.height, 1 });
            }
        }

        inline std::vector<Framebuffer> createSwapchainFramebuffer(api::SwapchainKHR swapchain, api::RenderPass renderpass) {
            // framebuffers vector
            std::vector<Framebuffer> swapchainBuffers = {};
            updateSwapchainFramebuffer(device, swapchain, renderpass, swapchainBuffers);
            for (int i = 0; i < swapchainBuffers.size(); i++)
            { // create semaphore
                swapchainBuffers[i].semaphore = api::Device(*device).createSemaphore(api::SemaphoreCreateInfo());
                swapchainBuffers[i].waitFence = api::Device(*device).createFence(api::FenceCreateInfo().setFlags(api::FenceCreateFlagBits::eSignaled));
            }
            return swapchainBuffers;
        }

        // create swapchain template
        inline api::SwapchainKHR createSwapchain()
        {
            api::SurfaceKHR surface = applicationWindow.surface;
            SurfaceFormat& formats = applicationWindow.surfaceFormat;

            auto surfaceCapabilities = api::PhysicalDevice(*device).getSurfaceCapabilitiesKHR(surface);
            auto surfacePresentModes = api::PhysicalDevice(*device).getSurfacePresentModesKHR(surface);

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
            return api::Device(*device).createSwapchainKHR(swapchainCreateInfo, nullptr);
        }
    };

}; // namespace NSM
