#define VMA_IMPLEMENTATION
#define TINYOBJLOADER_IMPLEMENTATION
#define VKT_FORCE_VMA_IMPLEMENTATION
#define VKT_ENABLE_GLFW_SUPPORT

#include <vkt2/fw.hpp>
#include <lancer-vk/lancer.hpp>

#define TINYGLTF_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "misc/tiny_gltf.h"


std::shared_ptr<vkt::GPUFramework> fw = {};

int main() {
    glfwInit();

    // 
	if (GLFW_FALSE == glfwVulkanSupported()) {
		glfwTerminate(); return -1;
	};

    // 
	uint32_t canvasWidth = 1600, canvasHeight = 1200; // For 3840x2160 Resolutions
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


    // 
    auto context = std::make_shared<lancer::Context>(fw);
    auto mesh = std::make_shared<lancer::Mesh>(context);
    auto node = std::make_shared<lancer::Node>(context);
    auto material = std::make_shared<lancer::Material>(context);
    auto renderer = std::make_shared<lancer::Renderer>(context);

    // initialize renderer
    context->initialize(canvasWidth, canvasHeight);
    renderer->linkMaterial(material)->linkNode(node);


    tinygltf::Model model = {};
    tinygltf::TinyGLTF loader = {};
    std::string err = "";
    std::string warn = "";

    bool ret = loader.LoadASCIIFromFile(&model, &err, &warn, "BoomBoxWithAxes.gltf");
    //bool ret = loader.LoadBinaryFromFile(&model, &err, &warn, argv[1]); // for binary glTF(.glb)

    if (!warn.empty()) {
        printf("Warn: %s\n", warn.c_str());
    }

    if (!err.empty()) {
        printf("Err: %s\n", err.c_str());
    }

    if (!ret) {
        printf("Failed to parse glTF\n");
        return -1;
    }

    using mat4_t = glm::mat3x4;

    // Every mesh will have transform buffer per internal instances
    std::vector<std::shared_ptr<lancer::Mesh>> meshes = {};
    std::vector<std::vector<mat4_t>> instancedTransformPerMesh = {}; // Run Out, Run Over

    // Transform Data Buffer
    std::vector<vkt::Vector<mat4_t>> gpuInstancedTransformPerMesh = {};
    std::vector<vkt::Vector<mat4_t>> cpuInstancedTransformPerMesh = {};

    // GLTF Data Buffer
    std::vector<vkt::Vector<uint8_t>> cpuBuffers = {};
    std::vector<vkt::Vector<uint8_t>> gpuBuffers = {};

    // 
    for (uint32_t i = 0; i < model.buffers.size(); i++) {
        cpuBuffers.push_back(vkt::Vector<>(std::make_shared<vkt::VmaBufferAllocation>(fw->getAllocator(), vkh::VkBufferCreateInfo{
            .size = model.buffers[i].data.size(),
            .usage = {.eTransferSrc = 1, .eStorageTexelBuffer = 1, .eStorageBuffer = 1, .eIndexBuffer = 1, .eVertexBuffer = 1 },
        }, VMA_MEMORY_USAGE_CPU_TO_GPU)));

        // 
        memcpy(cpuBuffers.back().data(), model.buffers[i].data.data(), model.buffers[i].data.size());

        // 
        gpuBuffers.push_back(vkt::Vector<>(std::make_shared<vkt::VmaBufferAllocation>(fw->getAllocator(), vkh::VkBufferCreateInfo{
            .size = model.buffers[i].data.size(),
            .usage = {.eTransferDst = 1, .eStorageTexelBuffer = 1, .eStorageBuffer = 1, .eIndexBuffer = 1, .eVertexBuffer = 1 },
        }, VMA_MEMORY_USAGE_GPU_ONLY)));

        // 
        vkt::submitOnce(device, queue, commandPool, [=](vk::CommandBuffer& cmd) {
            cmd.copyBuffer(cpuBuffers.back(), gpuBuffers.back(), { vkh::VkBufferCopy{.size = model.buffers[i].data.size()} });
        });
    };

    // buffer views
    std::vector<vkt::Vector<uint8_t>> buffersViews = {};
    for (uint32_t i = 0; i < model.bufferViews.size(); i++) {
        const auto& BV = model.bufferViews[i];
        buffersViews.push_back(vkt::Vector<uint8_t>(gpuBuffers[BV.buffer], BV.byteOffset, BV.byteLength));
        if (BV.byteStride) { buffersViews.back().stride = BV.byteStride; };
    };

    // Gonki //
    //   #   //
    // # # # //
    //   #   //
    // # # # //

    // Tanki //       //       // 
    //   #   //   #   //   #   //
    // # * # // # # # // # # # //
    // #   # // # # # // #   # //

    // Meshes (only one primitive supported)
    for (uint32_t i = 0; i < model.meshes.size(); i++) {
        const auto& meshData = model.meshes[i];

        // Make Instanced Primitives
        //for (uint32_t v = 0; v < meshData.primitives.size(); v++) {
        for (uint32_t v = 0; v < std::min(meshData.primitives.size(),1ull); v++) {
            const auto& primitive = meshData.primitives[v];
            auto mesh = std::make_shared<lancer::Mesh>(context); meshes.push_back(mesh);
            instancedTransformPerMesh.push_back({});

            if (primitive.attributes.find("POSITION") != primitive.attributes.end()) { // Vertices
                const auto& attribute = model.accessors[primitive.attributes.find("POSITION")->second];
                const auto& bufferView = buffersViews[attribute.bufferView];

                // 
                mesh->addBinding(bufferView, vkh::VkVertexInputBindingDescription{ 0u, uint32_t(attribute.ByteStride(model.bufferViews[attribute.bufferView])) });
                mesh->addAttribute(vkh::VkVertexInputAttributeDescription{ 0u, 0u, VK_FORMAT_R32G32B32_SFLOAT, uint32_t(attribute.byteOffset) }, true);
            };

            if (primitive.attributes.find("TEXCOORD_0") != primitive.attributes.end()) { // Texcoord
                const auto& attribute = model.accessors[primitive.attributes.find("TEXCOORD_0")->second];
                const auto& bufferView = buffersViews[attribute.bufferView];

                // 
                mesh->addBinding(bufferView, vkh::VkVertexInputBindingDescription{ 1u, uint32_t(attribute.ByteStride(model.bufferViews[attribute.bufferView])) });
                mesh->addAttribute(vkh::VkVertexInputAttributeDescription{ 1u, 1u, VK_FORMAT_R32G32_SFLOAT, uint32_t(attribute.byteOffset) });
            };

            if (primitive.attributes.find("NORMAL") != primitive.attributes.end()) { // Normals
                const auto& attribute = model.accessors[primitive.attributes.find("NORMAL")->second];
                const auto& bufferView = buffersViews[attribute.bufferView];

                // 
                mesh->addBinding(bufferView, vkh::VkVertexInputBindingDescription{ 2u, uint32_t(attribute.ByteStride(model.bufferViews[attribute.bufferView])) });
                mesh->addAttribute(vkh::VkVertexInputAttributeDescription{ 2u, 2u, VK_FORMAT_R32G32B32_SFLOAT, uint32_t(attribute.byteOffset) });
            };

            if (primitive.indices >= 0) {
                const auto& attribute = model.accessors[primitive.indices];
                const auto& bufferView = buffersViews[attribute.bufferView];

                // determine index type
                mesh->setIndexData(bufferView, attribute.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT ? vk::IndexType::eUint16 : vk::IndexType::eUint32);
            };

            node->pushMesh(mesh->setMaterialID(primitive.material)->increaseInstanceCount());
        };
    };


    std::shared_ptr<std::function<void(const tinygltf::Node&, glm::dmat4, int)>> vertexLoader = {};
    vertexLoader = std::make_shared<std::function<void(const tinygltf::Node&, glm::dmat4, int)>>([&](const tinygltf::Node& gnode, glm::dmat4 inTransform, int recursive)->void {
        auto localTransform = glm::dmat4(1.0);
        localTransform *= glm::dmat4(gnode.matrix.size() >= 16 ? glm::make_mat4(gnode.matrix.data()) : glm::dmat4(1.0));
        localTransform *= glm::dmat4(gnode.translation.size() >= 3 ? glm::translate(glm::make_vec3(gnode.translation.data())) : glm::dmat4(1.0));
        localTransform *= glm::dmat4(gnode.scale.size() >= 3 ? glm::scale(glm::make_vec3(gnode.scale.data())) : glm::dmat4(1.0));
        localTransform *= glm::dmat4((gnode.rotation.size() >= 4 ? glm::mat4_cast(glm::make_quat(gnode.rotation.data())) : glm::dmat4(1.0)));

        auto transform = glm::dmat4(inTransform) * glm::dmat4(localTransform);
        if (gnode.mesh >= 0) {
            auto& mesh = meshes[gnode.mesh]; // load mesh object (it just vector of primitives)
            node->pushInstance(vkh::VsGeometryInstance{
                .transform = mat4_t(glm::transpose(transform)),
                .instanceId = uint32_t(gnode.mesh),
                .mask = 0xff,
                .instanceOffset = 0u,
                .flags = VK_GEOMETRY_INSTANCE_TRIANGLE_CULL_DISABLE_BIT_NV,
            }, gnode.mesh);
        };

        if (gnode.children.size() > 0 && gnode.mesh < 0) {
            for (int n = 0; n < gnode.children.size(); n++) {
                if (recursive >= 0) (*vertexLoader)(model.nodes[gnode.children[n]], transform, recursive - 1);
            };
        };
    });

    // load scene
    uint32_t sceneID = 0; const float unitScale = 100.f;
    if (model.scenes.size() > 0) {
        for (int n = 0; n < model.scenes[sceneID].nodes.size(); n++) {
        //uint32_t n = 0u; {
            auto& gnode = model.nodes[model.scenes[sceneID].nodes[n]];
            (*vertexLoader)(gnode, glm::dmat4(glm::scale(glm::vec3(unitScale))), 8);
        };
    };

    // 
    glm::vec3 eye = glm::vec3(5.f, 2.f, 2.f);
    glm::vec3 foc = glm::vec3(0.f, 0.f, 0.f);

    // 
    //eye.z += float(context->timeDiff()) / 1000.f * 1.f;
    context->setModelView(glm::lookAt(eye, foc, glm::vec3(0.f, 1.f, 0.f)));
    context->setPerspective(glm::perspective(80.f / 180.f * glm::pi<float>(), float(canvasWidth) / float(canvasHeight), 0.0001f, 10000.f));

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
    uint32_t frameCount = 0u;

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
            std::vector<vk::ClearValue> clearValues = { vk::ClearColorValue(std::array<float,4>{0.f, 0.f, 0.f, 0.f}), vk::ClearDepthStencilValue(1.0f, 0) };
            context->registerTime()->setDrawCount(frameCount++);

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

            // Drunk Debug Camera Animation
            eye.x -= float(context->timeDiff()) / 1000.f * 0.1f;
            foc.x -= float(context->timeDiff()) / 1000.f * 0.1f;
            context->setModelView(glm::lookAt(eye, foc, glm::vec3(0.f, 1.f, 0.f)));

            // Create render submission 
            std::vector<vk::Semaphore> waitSemaphores = { framebuffers[n_semaphore].semaphore }, signalSemaphores = { framebuffers[c_semaphore].semaphore };
            std::vector<vk::PipelineStageFlags> waitStages = { vk::PipelineStageFlagBits::eColorAttachmentOutput };
            std::array<vk::CommandBuffer, 2> XPEHb = { renderer->refCommandBuffer(), commandBuffer };

            // Submit command once
            vkt::submitCmd(device, queue, { renderer->refCommandBuffer(), commandBuffer }, vk::SubmitInfo()
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
