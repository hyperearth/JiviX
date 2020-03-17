// #

#if defined(_WIN32) && !defined(WIN32)
#define WIN32
#endif

#ifdef WIN32
#define VK_USE_PLATFORM_WIN32_KHR
#endif

#define VMA_IMPLEMENTATION
#define TINYOBJLOADER_IMPLEMENTATION
#define VKT_FORCE_VMA_IMPLEMENTATION
#define VKT_ENABLE_GLFW_SUPPORT

#include "vkt2/fw.hpp"
#include "JiviX/JiviX.hpp"

#define TINYGLTF_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "misc/tiny_gltf.h"


struct Active {
    std::vector<uint8_t> keys = {};
    std::vector<uint8_t> mouse = {};

    double mX = 1e-5, mY = 1e-5, dX = 0.0, dY = 0.0;
    double tDiff = 0.0, tCurrent = 1e-5;
};

class Shared : public std::enable_shared_from_this<Shared> {
public:
    static Active active; // shared properties
    static GLFWwindow* window; // in-bound GLFW window
    //friend Renderer;

    static void TimeCallback(double milliseconds = 1e-5) {
        Shared::active.tDiff = milliseconds - Shared::active.tCurrent, Shared::active.tCurrent = milliseconds;
        Shared::active.dX = 0.0, Shared::active.dY = 0.0;
    };

    static void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
        if (action == GLFW_PRESS) Shared::active.keys[key] = uint8_t(1u);
        if (action == GLFW_RELEASE) Shared::active.keys[key] = uint8_t(0u);
        if (key == GLFW_KEY_ESCAPE && action == GLFW_RELEASE) { glfwTerminate(); exit(0); };
    };

    static void MouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
        if (action == GLFW_PRESS) Shared::active.mouse[button] = uint8_t(1u);
        if (action == GLFW_RELEASE) Shared::active.mouse[button] = uint8_t(0u);
    };

    static void MouseMoveCallback(GLFWwindow* window, double xpos = 1e-5, double ypos = 1e-5) {
        Shared::active.dX = xpos - Shared::active.mX, Shared::active.dY = ypos - Shared::active.mY; // get diff with previous position
        Shared::active.mX = xpos, Shared::active.mY = ypos; // set current mouse position
    };
};

// 
Active Shared::active; // shared properties
GLFWwindow* Shared::window; // in-bound GLFW window


// camera binding control 
class CameraController : public std::enable_shared_from_this<CameraController> {
public:
    // use pointers
    glm::dvec3* viewVector = nullptr;
    glm::dvec3* eyePos = nullptr;
    glm::dvec3* upVector = nullptr;
    glm::uvec2* canvasSize = nullptr;

    // create relative control matrice
    auto project() { return glm::lookAt(*eyePos, (*eyePos + *viewVector), *upVector); };

    // event handler
    CameraController& handle() {
        //auto mPtr = (glm::dvec2*)&Shared::active.mX, mDiff = *mPtr - mousePosition;
        auto mDiff = glm::dvec2(Shared::active.dX, Shared::active.dY);
        auto tDiff = Shared::active.tDiff;

        glm::dmat4 viewm = this->project();
        glm::dmat4 unviewm = glm::inverse(viewm);
        glm::dvec3 ca = (viewm * glm::dvec4(*eyePos, 1.0)).xyz(), vi = glm::normalize((glm::dvec4(*viewVector, 0.0) * unviewm)).xyz();
        bool isFocus = true;

        if (Shared::active.mouse.size() > 0 && Shared::active.mouse[GLFW_MOUSE_BUTTON_LEFT] && isFocus)
        {
            if (glm::abs(mDiff.x) > 0.0) this->rotateX(vi, mDiff.x);
            if (glm::abs(mDiff.y) > 0.0) this->rotateY(vi, mDiff.y);
        }

        if (Shared::active.keys.size() > 0 && Shared::active.keys[GLFW_KEY_W] && isFocus)
        {
            this->forwardBackward(ca, -tDiff);
        }

        if (Shared::active.keys.size() > 0 && Shared::active.keys[GLFW_KEY_S] && isFocus)
        {
            this->forwardBackward(ca, tDiff);
        }

        if (Shared::active.keys.size() > 0 && Shared::active.keys[GLFW_KEY_A] && isFocus)
        {
            this->leftRight(ca, -tDiff);
        }

        if (Shared::active.keys.size() > 0 && Shared::active.keys[GLFW_KEY_D] && isFocus)
        {
            this->leftRight(ca, tDiff);
        }

        if (Shared::active.keys.size() > 0 && (Shared::active.keys[GLFW_KEY_SPACE] || Shared::active.keys[GLFW_KEY_E]) && isFocus)
        {
            this->topBottom(ca, tDiff);
        }

        if (Shared::active.keys.size() > 0 && (Shared::active.keys[GLFW_KEY_LEFT_SHIFT] || Shared::active.keys[GLFW_KEY_C] || Shared::active.keys[GLFW_KEY_Q]) && isFocus)
        {
            this->topBottom(ca, -tDiff);
        }

        *viewVector = glm::normalize((glm::dvec4(vi, 0.0) * viewm).xyz());
        *eyePos = (unviewm * glm::dvec4(ca, 1.0)).xyz();

        return *this;
    }


    // sub-contollers
    CameraController& leftRight(glm::dvec3& ca, const double& diff) {
        ca.x += diff / 100.0;
        return *this;
    }

    CameraController& topBottom(glm::dvec3& ca, const double& diff) {
        ca.y += diff / 100.0;
        return *this;
    }

    CameraController& forwardBackward(glm::dvec3& ca, const double& diff) {
        ca.z += diff / 100.0;
        return *this;
    }

    CameraController& rotateY(glm::dvec3& vi, const double& diff) {
        glm::dmat4 rot = glm::rotate(diff / double(canvasSize->y) * 2., glm::dvec3(-1.0, 0.0, 0.0));
        vi = (rot * glm::vec4(vi, 1.0)).xyz();
        return *this;
    }

    CameraController& rotateX(glm::dvec3& vi, const double& diff) {
        glm::dmat4 rot = glm::rotate(diff / double(canvasSize->x) * 2., glm::dvec3(0.0, -1.0, 0.0));
        vi = (rot * glm::vec4(vi, 1.0)).xyz();
        return *this;
    }
};


void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    // make sure the viewport matches the new window dimensions; note that width and 
    // height will be significantly larger than specified on retina displays.
#ifdef ENABLE_OPENGL_INTEROP
    glViewport(0, 0, width, height);
#endif
}

//std::shared_ptr<vkt::GPUFramework> fw = {};
jvx::Driver fw = {};

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
    fw = jvx::Driver();
	auto instance = fw->createInstance();
	auto manager = fw->createWindowSurface(canvasWidth, canvasHeight);
	auto physicalDevice = fw->getPhysicalDevice(0u);
	auto device = fw->createDevice(true,"./",false);
	auto swapchain = fw->createSwapchain();
	auto renderPass = fw->createRenderPass();
	auto framebuffers = fw->createSwapchainFramebuffer(swapchain, renderPass);
	auto queue = fw->getQueue();
	auto commandPool = fw->getCommandPool();

    // OpenGL Context
    glfwMakeContextCurrent(manager.window);
    glfwSetFramebufferSizeCallback(manager.window, framebuffer_size_callback);

    // 
    auto renderArea = vk::Rect2D{ vk::Offset2D(0, 0), vk::Extent2D(canvasWidth, canvasHeight) };
    auto viewport = vk::Viewport{ 0.0f, 0.0f, static_cast<float>(renderArea.extent.width), static_cast<float>(renderArea.extent.height), 0.f, 1.f };

    // 
    auto context = jvx::Context(fw);
    auto mesh = jvx::Mesh(context);
    auto node = jvx::Node(context);
    auto material = jvx::Material(context);
    auto renderer = jvx::Renderer(context);

    // initialize renderer
    context->initialize(canvasWidth, canvasHeight);
    renderer->linkMaterial(material)->linkNode(node);

    // 
    tinygltf::Model model = {};
    tinygltf::TinyGLTF loader = {};
    std::string err = "";
    std::string wrn = "";

    // 
    const float unitScale = 100.f;
    const float unitHeight = -1.f;
    //const bool ret = loader.LoadASCIIFromFile(&model, &err, &wrn, "DamagedHelmet.gltf");
    const bool ret = loader.LoadASCIIFromFile(&model, &err, &wrn, "BoomBoxWithAxes.gltf");
    //const bool ret = loader.LoadASCIIFromFile(&model, &err, &warn, "Chess_Set/Chess_Set.gltf");
    //const bool ret = loader.LoadASCIIFromFile(&model, &err, &warn, "lost_empire.gltf"); 
    //const bool ret = loader.LoadBinaryFromFile(&model, &err, &warn, argv[1]); // for binary glTF(.glb)

    // 
    if (!wrn.empty()) {  printf("Warn : %s\n", wrn.c_str()); }
    if (!err.empty()) {  printf("Error: %s\n", err.c_str()); }
    if (!ret) { printf("Failed to parse glTF\n"); return -1; }

    // 
    using mat4_t = glm::mat3x4;

    // Every mesh will have transform buffer per internal instances
    std::vector<jvx::Mesh> meshes = {};
    std::vector<std::vector<mat4_t>> instancedTransformPerMesh = {}; // Run Out, Run Over

    // Transform Data Buffer
    //std::vector<vkt::Vector<mat4_t>> gpuInstancedTransformPerMesh = {};
    std::vector<vkt::Vector<mat4_t>> cpuInstancedTransformPerMesh = {};

    // GLTF Data Buffer
    std::vector<vkt::Vector<uint8_t>> cpuBuffers = {};
    //std::vector<vkt::Vector<uint8_t>> gpuBuffers = {};

    // 
    for (uint32_t i = 0; i < model.buffers.size(); i++) {
        cpuBuffers.push_back(vkt::Vector<>(fw->getAllocator(), vkh::VkBufferCreateInfo{
            .size = vkt::tiled(model.buffers[i].data.size(), 4ull) * 4ull,
            .usage = {.eTransferSrc = 1, .eStorageTexelBuffer = 1, .eStorageBuffer = 1, .eIndexBuffer = 1, .eVertexBuffer = 1 },
        }, VMA_MEMORY_USAGE_CPU_TO_GPU));

        // 
        memcpy(cpuBuffers.back().data(), model.buffers[i].data.data(), model.buffers[i].data.size());
    };

    // buffer views
    std::vector<vkt::Vector<uint8_t>> buffersViews = {};
    for (uint32_t i = 0; i < model.bufferViews.size(); i++) {
        const auto& BV = model.bufferViews[i];
        const auto range = vkt::tiled(BV.byteLength, 4ull) * 4ull;
        buffersViews.push_back(vkt::Vector<uint8_t>(cpuBuffers[BV.buffer], BV.byteOffset, vkt::tiled(BV.byteLength, 4ull) * 4ull));
    };

    // 
    std::vector<vk::Sampler> samplers = {};
    std::vector<vkt::ImageRegion> images = {};
    for (uint32_t i = 0; i < model.images.size(); i++) {
        const auto& img = model.images[i];

        // 
        images.push_back(vkt::ImageRegion(fw->getAllocator(), vkh::VkImageCreateInfo{
            .format = VK_FORMAT_R8G8B8A8_UNORM,
            .extent = {uint32_t(img.width),uint32_t(img.height),1u},
            .usage = {.eTransferDst = 1, .eSampled = 1, .eStorage = 1, .eColorAttachment = 1 },
        }, VMA_MEMORY_USAGE_GPU_ONLY, vkh::VkImageViewCreateInfo{
            .format = VK_FORMAT_R8G8B8A8_UNORM,
        }).setSampler(device.createSampler(vkh::VkSamplerCreateInfo{
            .magFilter = VK_FILTER_LINEAR,
            .minFilter = VK_FILTER_LINEAR,
            .addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT,
            .addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT,
        })));

        // 
        auto image = images.back();

        // 
        vkt::Vector<> imageBuf = {};
        if (img.image.size() > 0u) {
            imageBuf = vkt::Vector<>(fw->getAllocator(), vkh::VkBufferCreateInfo{
                .size = img.image.size(),
                .usage = {.eTransferSrc = 1, .eStorageTexelBuffer = 1, .eStorageBuffer = 1, .eIndexBuffer = 1, .eVertexBuffer = 1 },
            }, VMA_MEMORY_USAGE_CPU_TO_GPU);
            memcpy(imageBuf.data(), &img.image[0u], img.image.size());
        };

        // 
        context->getThread()->submitOnce([=](vk::CommandBuffer& cmd) {
            vkt::imageBarrier(cmd, vkt::ImageBarrierInfo{ .image = image->getImage(), .targetLayout = vk::ImageLayout::eGeneral, .originLayout = vk::ImageLayout::eUndefined, .subresourceRange = reinterpret_cast<const vkh::VkImageSubresourceRange&>(image.getImageSubresourceRange()) });

            auto buffer = imageBuf.has() ? imageBuf : buffersViews[img.bufferView];
            cmd.copyBufferToImage(buffer.buffer(), image.getImage(), image.getImageLayout(), { vkh::VkBufferImageCopy{
                .bufferOffset = buffer.offset(),
                .bufferRowLength = uint32_t(img.width),
                .bufferImageHeight = uint32_t(img.height),
                .imageSubresource = image.subresourceLayers(),
                .imageOffset = {0u,0u,0u},
                .imageExtent = {uint32_t(img.width),uint32_t(img.height),1u},
            } });
        });

        material->pushSampledImage(image);
    };

    // GLTF Samplers Support
    for (uint32_t i = 0; i < model.samplers.size(); i++) {
        const auto& smp = model.samplers[i];
        samplers.push_back(device.createSampler(vkh::VkSamplerCreateInfo{
            .magFilter = VK_FILTER_LINEAR,
            .minFilter = VK_FILTER_LINEAR,
            .addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT,
            .addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT
        }));
    };

    // Material 
    for (uint32_t i = 0; i < model.materials.size(); i++) {
        const auto& mat = model.materials[i];
        jvi::MaterialUnit mdk = {};
        mdk.diffuseTexture = mat.pbrMetallicRoughness.baseColorTexture.index;
        mdk.normalsTexture = mat.normalTexture.index;
        mdk.specularTexture = mat.pbrMetallicRoughness.metallicRoughnessTexture.index;
        mdk.emissionTexture = mat.emissiveTexture.index;
        mdk.specular = glm::vec4(1.f, mat.pbrMetallicRoughness.roughnessFactor, mat.pbrMetallicRoughness.metallicFactor, 0.f);
        mdk.normals = glm::vec4(0.5f,0.5f,1.0f,1.f);

        if (mat.emissiveFactor.size() > 0) {
            mdk.emission = glm::vec4(mat.emissiveFactor[0], mat.emissiveFactor[1], mat.emissiveFactor[2], 0.f);
        };
        if (mat.pbrMetallicRoughness.baseColorFactor.size() > 0) {
            mdk.diffuse = glm::vec4(mat.pbrMetallicRoughness.baseColorFactor[0], mat.pbrMetallicRoughness.baseColorFactor[1], mat.pbrMetallicRoughness.baseColorFactor[2], 1.f);
        };

        material->pushMaterial(mdk);
    };

    // BRICK GAME BANK

    // Gonki  //
    //   []   //
    // [][][] //
    //   []   //
    // [][][] //

    // Tanki  //        //        //        //
    //   []   //   []   //   []   //   [][] //
    // []{}[] // [][][] // [][][] // [][]   //
    // []  [] // [][][] // []  [] //   [][] //

    // Meshes (only one primitive supported)
    for (uint32_t i = 0; i < model.meshes.size(); i++) {
        const auto& meshData = model.meshes[i];

        // Make Instanced Primitives
        //for (uint32_t v = 0; v < meshData.primitives.size(); v++) {
        for (uint32_t v = 0; v < std::min(meshData.primitives.size(),1ull); v++) {
            const auto& primitive = meshData.primitives[v];

            // 
            uintptr_t vertexCount = 0u; bool ctype = true;//false;
            if (primitive.indices >= 0) {
                vertexCount = model.accessors[primitive.indices].count;
                if (model.accessors[primitive.indices].componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT) { ctype = true; };
            } else 
            if (primitive.attributes.find("POSITION") != primitive.attributes.end()) { // Vertices
                vertexCount = model.accessors[primitive.attributes.find("POSITION")->second].count;
            };

            // 
            meshes.push_back(jvx::Mesh(context, vertexCount<<uintptr_t(ctype)*2u));
            auto& mesh = meshes.back(); instancedTransformPerMesh.push_back({});

            // 
            std::array<std::string, 4u> NM = { "POSITION" , "TEXCOORD_0" , "NORMAL" , "TANGENT" };
            for (uint32_t i = 0u; i < NM.size(); i++) {
                if (primitive.attributes.find(NM[i]) != primitive.attributes.end()) { // Vertices
                    auto& attribute = model.accessors[primitive.attributes.find(NM[i])->second];
                    const auto& BV = model.bufferViews[attribute.bufferView];
                    const auto range = vkt::tiled(BV.byteLength, 4ull) * 4ull;

                    // 
                    auto stride = std::max(vk::DeviceSize(attribute.ByteStride(model.bufferViews[attribute.bufferView])), buffersViews[attribute.bufferView].stride());
                    auto vector = vkt::Vector<uint8_t>(cpuBuffers[BV.buffer], BV.byteOffset + attribute.byteOffset, vkt::tiled(BV.byteLength, 4ull) * 4ull);
                    vector.rangeInfo() = stride * attribute.count;

                    // 
                    uint32_t location = 0u;
                    if (NM[i] == "POSITION") { location = 0u; };
                    if (NM[i] == "TEXCOORD_0") { location = 1u; };
                    if (NM[i] == "NORMAL") { location = 2u; };
                    if (NM[i] == "TANGENT") { location = 3u; };

                    // 
                    auto type = VK_FORMAT_R32G32B32_SFLOAT;
                    if (attribute.type == TINYGLTF_TYPE_VEC4) type = VK_FORMAT_R32G32B32A32_SFLOAT;
                    if (attribute.type == TINYGLTF_TYPE_VEC3) type = VK_FORMAT_R32G32B32_SFLOAT;
                    if (attribute.type == TINYGLTF_TYPE_VEC2) type = VK_FORMAT_R32G32_SFLOAT;
                    if (attribute.type == TINYGLTF_TYPE_SCALAR) type = VK_FORMAT_R32_SFLOAT;

                    // 
                    mesh->addBinding(vector, vkh::VkVertexInputBindingDescription{ .stride = uint32_t(stride) });
                    mesh->addAttribute(vkh::VkVertexInputAttributeDescription{ .location = location, .format = type, .offset = 0u });
                }
                else if (NM[i] == "TANGENT") { // STUB for Tangents
                    auto& attribute = primitive.attributes.find("NORMAL") != primitive.attributes.end() ? model.accessors[primitive.attributes.find("NORMAL")->second] : model.accessors[primitive.attributes.find("POSITION")->second];
                    const auto& BV = model.bufferViews[attribute.bufferView];
                    const auto range = vkt::tiled(BV.byteLength, 4ull) * 4ull;

                    // 
                    auto stride = std::max(vk::DeviceSize(attribute.ByteStride(model.bufferViews[attribute.bufferView])), buffersViews[attribute.bufferView].stride());
                    auto vector = vkt::Vector<uint8_t>(cpuBuffers[BV.buffer], BV.byteOffset + attribute.byteOffset, vkt::tiled(BV.byteLength, 4ull) * 4ull);
                    vector.rangeInfo() = stride * attribute.count;

                    // 
                    auto type = VK_FORMAT_R32G32B32_SFLOAT;
                    if (attribute.type == TINYGLTF_TYPE_VEC4) type = VK_FORMAT_R32G32B32A32_SFLOAT;
                    if (attribute.type == TINYGLTF_TYPE_VEC3) type = VK_FORMAT_R32G32B32_SFLOAT;
                    if (attribute.type == TINYGLTF_TYPE_VEC2) type = VK_FORMAT_R32G32_SFLOAT;
                    if (attribute.type == TINYGLTF_TYPE_SCALAR) type = VK_FORMAT_R32_SFLOAT;

                    // 
                    mesh->addBinding(vector, vkh::VkVertexInputBindingDescription{ .stride = uint32_t(stride) });
                    mesh->addAttribute(vkh::VkVertexInputAttributeDescription{ .location = 3u, .format = type, .offset = 0u }, false);
                };
            };

            if (primitive.indices >= 0) {
                auto& attribute = model.accessors[primitive.indices];
                //auto& bufferView = buffersViews[attribute.bufferView];
                const auto& BV = model.bufferViews[attribute.bufferView];
                const auto range = vkt::tiled(BV.byteLength, 4ull) * 4ull;

                // 
                auto stride = std::max(vk::DeviceSize(attribute.ByteStride(model.bufferViews[attribute.bufferView])), buffersViews[attribute.bufferView].stride());
                auto vector = vkt::Vector<uint8_t>(cpuBuffers[BV.buffer], BV.byteOffset + attribute.byteOffset, vkt::tiled(BV.byteLength, 4ull) * 4ull);
                vector.rangeInfo() = stride * attribute.count;

                // determine index type
                mesh->setIndexData(vector.getDescriptor(), attribute.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT ? vk::IndexType::eUint16 : (attribute.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE ? vk::IndexType::eUint8EXT : vk::IndexType::eUint32));
            };

            node->pushMesh(mesh->setMaterialID(primitive.material)->increaseInstanceCount()->sharedPtr());
        };
    };

    // 
    std::shared_ptr<std::function<void(const tinygltf::Node&, glm::dmat4, int)>> vertexLoader = {};
    vertexLoader = std::make_shared<std::function<void(const tinygltf::Node&, glm::dmat4, int)>>([&](const tinygltf::Node& gnode, glm::dmat4 inTransform, int recursive)->void {
        auto localTransform = glm::dmat4(1.0);
        localTransform *= glm::dmat4(gnode.matrix.size() >= 16 ? glm::make_mat4(gnode.matrix.data()) : glm::dmat4(1.0));
        localTransform *= glm::dmat4(gnode.translation.size() >= 3 ? glm::translate(glm::make_vec3(gnode.translation.data())) : glm::dmat4(1.0));
        localTransform *= glm::dmat4(gnode.scale.size() >= 3 ? glm::scale(glm::make_vec3(gnode.scale.data())) : glm::dmat4(1.0));
        localTransform *= glm::dmat4((gnode.rotation.size() >= 4 ? glm::mat4_cast(glm::make_quat(gnode.rotation.data())) : glm::dmat4(1.0)));

        if (gnode.mesh >= 0) {
            auto& mesh = meshes[gnode.mesh]; // load mesh object (it just vector of primitives)
            node->pushInstance(vkh::VsGeometryInstance{
                .transform = mat4_t(glm::transpose(glm::dmat4(inTransform) * glm::dmat4(localTransform))),
                .instanceId = uint32_t(gnode.mesh), // MeshID
                .mask = 0xff,
                .instanceOffset = 0u,
                .flags = VK_GEOMETRY_INSTANCE_TRIANGLE_CULL_DISABLE_BIT_NV,
            });
        };

        if (gnode.children.size() > 0 && gnode.mesh < 0) {
            for (int n = 0; n < gnode.children.size(); n++) {
                if (recursive >= 0) (*vertexLoader)(model.nodes[gnode.children[n]], glm::dmat4(inTransform) * glm::dmat4(localTransform), recursive - 1);
            };
        };
    });

    // load scene
    uint32_t sceneID = 0;
    if (model.scenes.size() > 0) {
        for (int n = 0; n < model.scenes[sceneID].nodes.size(); n++) {
            auto& gnode = model.nodes[model.scenes[sceneID].nodes[n]];
            (*vertexLoader)(gnode, glm::dmat4(glm::translate(glm::dvec3(0., unitHeight, 0.)) * glm::scale(glm::dvec3(unitScale))), 16);
        };

        for (int n = 0; n < model.scenes[sceneID].nodes.size(); n++) {
            auto& gnode = model.nodes[model.scenes[sceneID].nodes[n]];
            (*vertexLoader)(gnode, glm::dmat4(glm::translate(glm::dvec3(-0., unitHeight-2.f, -2.)) * glm::scale(glm::dvec3(unitScale))), 16);
        };
    };

    // 
    glm::dvec3 eye = glm::dvec3(5.f, 2.f, 2.f);
    glm::dvec3 foc = glm::dvec3(0.f, 0.f, 0.f);
    glm::dvec3 evc = foc - eye;
    glm::dvec3 upv = glm::dvec3(0.f, 1.f, 0.f);
    glm::uvec2 canvasSize = { canvasWidth, canvasHeight };

    // 
    auto cameraController = std::make_shared<CameraController>();
    cameraController->canvasSize = &canvasSize;
    cameraController->eyePos = &eye;
    cameraController->upVector = &upv;
    cameraController->viewVector = &evc;

    // 
    //eye.z += float(context->timeDiff()) / 1000.f * 1.f;
    context->setModelView(glm::lookAt(eye, foc, glm::dvec3(0.f, 1.f, 0.f)));
    context->setPerspective(glm::perspective(80.f / 180.f * glm::pi<double>(), double(canvasWidth) / double(canvasHeight), 0.001, 10000.));

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

    // set GLFW callbacks
    glfwSetMouseButtonCallback(fw->window(), &Shared::MouseButtonCallback);
    glfwSetCursorPosCallback(fw->window(), &Shared::MouseMoveCallback);
    glfwSetKeyCallback(fw->window(), &Shared::KeyCallback);

    // 
    Shared::active = Active{};
    Shared::window = fw->window(); // set GLFW window
    Shared::active.tDiff = 0.0; // reset diff to near-zero (avoid critical math errors)
    Shared::active.keys.resize(1024, uint8_t(0u));
    Shared::active.mouse.resize(128, uint8_t(0u));
    Shared::TimeCallback(double(context->registerTime()->setDrawCount(frameCount++)->drawTime()));

    // 
    //auto sps = vkh::VkVertexInputBindingDescription{};
    //auto spc = sizeof(sps);

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
            Shared::TimeCallback(double(context->registerTime()->setModelView(cameraController->handle().project())->drawTime()));

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
            std::array<vk::CommandBuffer, 2> XPEHb = { renderer->refCommandBuffer(), commandBuffer };

            // Submit command once
            context->getThread()->submitCmd({ renderer->refCommandBuffer(), commandBuffer }, vk::SubmitInfo()
                .setPCommandBuffers(XPEHb.data()).setCommandBufferCount(XPEHb.size())
                .setPWaitDstStageMask(waitStages.data()).setPWaitSemaphores(waitSemaphores.data()).setWaitSemaphoreCount(waitSemaphores.size())
                .setPSignalSemaphores(signalSemaphores.data()).setSignalSemaphoreCount(signalSemaphores.size()));

            // 
            context->setDrawCount(frameCount++);
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
