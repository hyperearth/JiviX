// #

#if defined(_WIN32) && !defined(WIN32)
#define WIN32
#endif

#ifdef WIN32
#define VK_USE_PLATFORM_WIN32_KHR
#endif

//
#define VMA_IMPLEMENTATION
#define VKT_FORCE_VMA_IMPLEMENTATION
#define VKT_ENABLE_GLFW_SUPPORT
#define TINYGLTF_IMPLEMENTATION
#define TINYEXR_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#define VKT_ENABLE_GLFW_LINKED
#define VKT_ENABLE_GLFW_SURFACE

//
#include <string>
#include <iostream>

//
#include "vkt3/fw.hpp"
#include "JiviX/JiviX.hpp"
#include "misc/tiny_gltf.h"
#include "misc/tinyexr.h"

//
//#include <glbinding-aux/debug.h>
//#include <glbinding/getProcAddress.h>

#include "tinyobjloader/tiny_obj_loader.h"

//
struct Active {
    std::vector<uint8_t> keys = {};
    std::vector<uint8_t> mouse = {};

    double mX = 1e-5, mY = 1e-5, dX = 0.0, dY = 0.0;
    double tDiff = 0.0, tCurrent = 1e-5;
};


//
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


#if defined(ENABLE_OPENGL_INTEROP) && !defined(VKT_USE_GLAD)
using namespace gl;
#endif

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

// FOR FUTURE USAGE!
class RayTracer : public std::enable_shared_from_this<RayTracer> {
public:
    ~RayTracer() {};
    RayTracer() {
        this->fw = std::make_shared<vkt::GPUFramework>();
    };

    //
    std::shared_ptr<vkt::GPUFramework> fw = {};
};

//
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow* window);

// settings
const unsigned int SCR_WIDTH = 1600;
const unsigned int SCR_HEIGHT = 1200;


const char* vertexShaderSource = "#version 460 compatibility\n"
                                 "layout (location = 0) in vec4 aPos;\n"
                                 "void main()\n"
                                 "{\n"
                                 "   gl_Position = vec4(aPos.xyz, 1.0f);\n"
                                 "}\0";


const char* fragmentShaderSource = "#version 460 compatibility\n"
                                   "#extension GL_ARB_bindless_texture : require\n"
                                   "out vec4 FragColor;\n"
                                   "layout (binding = 0) uniform sampler2D texture0;\n"
                                   "void main()\n"
                                   "{\n"
                                   "	vec2 tx = gl_FragCoord.xy/vec2(1600.f,1200.f);\n"
                                   "   FragColor = vec4(pow(texture(texture0,tx).xyz,1.f.xxx/2.2.xxx),1.f);\n"
                                   "   //FragColor = vec4(pow(texture(texture1,tx).xyz/texture(texture1,tx).w*vec3(0.5f,0.5f,1.f),1.f.xxx/2.2.xxx),1.f);\n"
                                   "   //FragColor = vec4(pow(texture(texture0,tx).xyz,1.f.xxx/2.2.xxx),1.f);\n"
                                   "}\n\0";


const char* vertexTFShaderSource = "#version 460 compatibility\n"
                                   "layout (location = 0) in vec3 aPos;\n"
                                   "layout (location = 0) out vec3 oPos;\n"

                                   "void main()\n"
                                   "{\n"
                                   "   gl_Position = vec4(oPos = aPos, 1.f);\n"
                                   "}\0";


const char* geometryTFShaderSource = "#version 460 compatibility\n"
                                     "layout (location = 0) in vec3 oPos[];\n"

                                     "layout (location = 0, xfb_buffer = 0, xfb_stride = 80, xfb_offset = 0) out vec4 fPosition;\n"
                                     "layout (location = 1, xfb_buffer = 0, xfb_stride = 80, xfb_offset = 16) out vec4 fTexcoord;\n"
                                     "layout (location = 2, xfb_buffer = 0, xfb_stride = 80, xfb_offset = 32) out vec4 fNormal;\n"
                                     "layout (location = 3, xfb_buffer = 0, xfb_stride = 80, xfb_offset = 48) out vec4 fTangent;\n"
                                     "layout (location = 4, xfb_buffer = 0, xfb_stride = 80, xfb_offset = 64) out vec4 fBinormal;\n"

                                     "layout (lines_adjacency) in; \n"
                                     "layout (triangle_strip, max_vertices = 4) out; \n"

                                     "void main()\n"
                                     "{\n"
                                     "for (int i=0;i<4;i++) {\n"
                                     "   gl_Position = vec4(oPos[i].xyz, 1.f);\n"
                                     "   fNormal = vec4(vec3(0.f,0.f,1.f), 1.f);\n"
                                     "   fPosition = vec4(oPos[i].xyz, 1.0f);\n"
                                     "   EmitVertex();\n"
                                     "}\n"
                                     "EndPrimitive();\n"
                                     "}\0";


const char* computeTestShaderSource = "#version 460 compatibility\n"
                                      "layout (binding = 0, rgba32f) uniform image2D source;\n"
                                      "layout (local_size_x = 16u, local_size_y = 16u) in;\n"
                                      "void main(){\n"
                                      "vec4 color = imageLoad(source, ivec2(gl_GlobalInvocationID.xy));\n"
                                      "imageStore(source, ivec2(gl_GlobalInvocationID.xy), vec4(color.xyz * vec3(1.f,0.1f,0.1f), 1.f));\n"
                                      "}\0";



struct FDStruct {
    glm::vec4 fPosition = glm::vec4(1.f);
    glm::vec4 fTexcoord = glm::vec4(1.f);
    glm::vec4 fNormal = glm::vec4(1.f);
    glm::vec4 fTangent = glm::vec4(1.f);
    glm::vec4 fBinormal = glm::vec4(1.f);
};


class attrib_t;

/*
void APIENTRY glDebugOutput(GLenum source,
    GLenum type,
    unsigned int id,
    GLenum severity,
    GLsizei length,
    const char* message,
    const void* userParam)
{
    // ignore non-significant error/warning codes
    if (id == 131169 || id == 131185 || id == 131218 || id == 131204) return;

    std::cout << "---------------" << std::endl;
    std::cout << "Debug message (" << id << "): " << message << std::endl;

    switch (source)
    {
    case GL_DEBUG_SOURCE_API:             std::cout << "Source: API"; break;
    case GL_DEBUG_SOURCE_WINDOW_SYSTEM:   std::cout << "Source: Window System"; break;
    case GL_DEBUG_SOURCE_SHADER_COMPILER: std::cout << "Source: Shader Compiler"; break;
    case GL_DEBUG_SOURCE_THIRD_PARTY:     std::cout << "Source: Third Party"; break;
    case GL_DEBUG_SOURCE_APPLICATION:     std::cout << "Source: Application"; break;
    case GL_DEBUG_SOURCE_OTHER:           std::cout << "Source: Other"; break;
    } std::cout << std::endl;

    switch (type)
    {
    case GL_DEBUG_TYPE_ERROR:               std::cout << "Type: Error"; break;
    case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: std::cout << "Type: Deprecated Behaviour"; break;
    case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:  std::cout << "Type: Undefined Behaviour"; break;
    case GL_DEBUG_TYPE_PORTABILITY:         std::cout << "Type: Portability"; break;
    case GL_DEBUG_TYPE_PERFORMANCE:         std::cout << "Type: Performance"; break;
    case GL_DEBUG_TYPE_MARKER:              std::cout << "Type: Marker"; break;
    case GL_DEBUG_TYPE_PUSH_GROUP:          std::cout << "Type: Push Group"; break;
    case GL_DEBUG_TYPE_POP_GROUP:           std::cout << "Type: Pop Group"; break;
    case GL_DEBUG_TYPE_OTHER:               std::cout << "Type: Other"; break;
    } std::cout << std::endl;

    switch (severity)
    {
    case GL_DEBUG_SEVERITY_HIGH:         std::cout << "Severity: high"; break;
    case GL_DEBUG_SEVERITY_MEDIUM:       std::cout << "Severity: medium"; break;
    case GL_DEBUG_SEVERITY_LOW:          std::cout << "Severity: low"; break;
    case GL_DEBUG_SEVERITY_NOTIFICATION: std::cout << "Severity: notification"; break;
    } std::cout << std::endl;
    std::cout << std::endl;
}


GLenum glCheckError_(const char* file, int line) {
    GLenum errorCode = GL_NO_ERROR;
    while ((errorCode = glGetError()) != GL_NO_ERROR)
    {
        std::string error = "UNKNOWN_ERROR";
        switch (errorCode)
        {
        case GL_INVALID_ENUM:                  error = "INVALID_ENUM"; break;
        case GL_INVALID_VALUE:                 error = "INVALID_VALUE"; break;
        case GL_INVALID_OPERATION:             error = "INVALID_OPERATION"; break;
        case GL_STACK_OVERFLOW:                error = "STACK_OVERFLOW"; break;
        case GL_STACK_UNDERFLOW:               error = "STACK_UNDERFLOW"; break;
        case GL_OUT_OF_MEMORY:                 error = "OUT_OF_MEMORY"; break;
        case GL_INVALID_FRAMEBUFFER_OPERATION: error = "INVALID_FRAMEBUFFER_OPERATION"; break;
        default:                               error = "UNKNOWN_ERROR"; break;
        };
        std::cerr << error << " | " << file << " (" << line << ")" << std::endl; throw (error);
        assert(errorCode == GL_NO_ERROR);
        glfwTerminate(); exit(int(errorCode));
    };
    return errorCode;
}

#define glCheckError() glCheckError_(__FILE__, __LINE__)
*/

void error(int errnum, const char * errmsg)
{
    std::cerr << errnum << ": " << errmsg << std::endl;
}

struct VertexUnit {
    glm::vec4 vertex = glm::vec4(0.f,0.f,0.f,1.f);
    glm::vec4 normal = glm::vec4(0.f,0.f,1.f,0.f);
    glm::vec4 texcoord = glm::vec4(0.f,0.f,0.f,0.f);
    glm::vec4 color = glm::vec4(1.f,1.f,1.f,1.f);
};

//
int main() {
    glfwSetErrorCallback(error);
    glfwInit();

    //
    if (GLFW_FALSE == glfwVulkanSupported()) {
        glfwTerminate(); return -1;
    };

    //
    uint32_t canvasWidth = SCR_WIDTH, canvasHeight = SCR_HEIGHT; // For 3840x2160 Resolutions
    //uint32_t canvasWidth = 1920, canvasHeight = 1080;
    glfwDefaultWindowHints();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_API);
    glfwWindowHint(GLFW_CONTEXT_CREATION_API, GLFW_NATIVE_CONTEXT_API);

    // TODO: SPECIFIC GL_API
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    // OpenGL IS NOT MAIN! IT FOR TEST ONLY!
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_COMPAT_PROFILE);
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, true);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // uncomment this statement to fix compilation on OS X
#endif

    // For OpenGL Context
    fw = jvx::Driver();
    auto& appObj = fw->getAppObject();

    // Ininitialize by GlBinding
    //glfwMakeContextCurrent(appObj.opengl = glfwCreateWindow(canvasWidth, canvasHeight, "GLTest", nullptr, nullptr));
#ifdef ENABLE_OPENGL_INTEROP
    glbinding::initialize(0, glfwGetProcAddress, true, false);
    glbinding::aux::enableGetErrorCallback();

    // Pravoslavie Smerti
    int flags = 0u; glGetIntegerv(GL_CONTEXT_FLAGS, &flags);
    if (flags & int(GL_CONTEXT_FLAG_DEBUG_BIT)) {
        // initialize debug output
        glEnable(GL_DEBUG_OUTPUT);
        glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
        glDebugMessageCallback(glDebugOutput, nullptr);
        glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE);
        glCheckError();
    }
#endif

    //
    float xscale = 1.f, yscale = 1.f;
    GLFWmonitor* primary = glfwGetPrimaryMonitor();
    glfwGetMonitorContentScale(primary, &xscale, &yscale);

    //
    auto renderArea = vkh::VkRect2D{ vkh::VkOffset2D{0, 0}, vkh::VkExtent2D{ uint32_t(canvasWidth / 2.f * xscale), uint32_t(canvasHeight / 2.f * yscale) } };
    auto viewport = vkh::VkViewport{ 0.0f, 0.0f, static_cast<float>(renderArea.extent.width), static_cast<float>(renderArea.extent.height), 0.f, 1.f };

    //
    auto instance = fw->createInstance();
    //auto manager = fw->createWindowSurface(canvasWidth, canvasHeight);
    auto manager = fw->createWindowSurface(fw->createWindowOnly(renderArea.extent.width, renderArea.extent.height, "VKTest"));
    auto physicalDevice = fw->getPhysicalDevice(0u);
    auto device = fw->createDevice(true, "./", false);
    auto swapchain = fw->createSwapchain();
    auto renderPass = fw->createRenderPass();
    auto framebuffers = fw->createSwapchainFramebuffer(swapchain, renderPass);
    auto queue = fw->getQueue();
    auto commandPool = fw->getCommandPool();
    auto allocator = fw->getAllocator();


    //
    glfwSetFramebufferSizeCallback(manager.window, framebuffer_size_callback);
    //vkt::initializeGL(); // PentaXIL

    // initialize renderer
    auto context = jvx::Context(fw);

    // Initialize Early
    context.initialize(canvasWidth, canvasHeight); // experimental: callify

    // Create Other Objects
    //auto mesh = jvx::MeshBinding(context, 2048ull * 64ull);
    auto bvse = jvx::BufferViewSet(context);
    auto node = jvx::Node(context);
    auto renderer = jvx::Renderer(context);
    auto material = jvx::Material(context);
    renderer->linkMaterial(material->sharedPtr())->linkNode(node->sharedPtr());



    //
    using mat4_t = glm::mat3x4;

    // Every mesh will have transform buffer per internal instances
    std::vector<jvx::MeshBinding> meshes = {};
    std::vector<std::vector<mat4_t>> instancedTransformPerMesh = {}; // Run Out, Run Over

    // Transform Data Buffer
    //std::vector<vkt::Vector<mat4_t>> gpuInstancedTransformPerMesh = {};
    std::vector<vkt::Vector<mat4_t>> cpuInstancedTransformPerMesh = {};

    // GLTF Data Buffer
    //std::vector<vkt::Vector<uint8_t>> cpuBuffers = {};
    //std::vector<vkt::Vector<uint8_t>> gpuBuffers = {};



    // buffer views
    auto imageUsage = vkh::VkImageUsageFlags{.eTransferDst = 1, .eSampled = 1, .eStorage = 1, .eColorAttachment = 1 };
    auto bufferUsage = vkh::VkBufferUsageFlags{.eTransferSrc = 1, .eStorageTexelBuffer = 1, .eStorageBuffer = 1, .eIndexBuffer = 1, .eVertexBuffer = 1, .eTransformFeedbackBuffer = 1 };
    auto uploadUsage = vkh::VkBufferUsageFlags{.eTransferSrc = 1, .eStorageTexelBuffer = 1, .eStorageBuffer = 1, .eIndexBuffer = 1, .eVertexBuffer = 1 };

    //
    std::vector<vkt::Vector<VertexUnit>> buffersViews = {};

    //
    auto aspect = vkh::VkImageAspectFlags{.eColor = 1};
    auto apres = vkh::VkImageSubresourceRange{.aspectMask = aspect};

    {   //
        int width = 0u, height = 0u;
        float* rgba = nullptr;
        const char* err = nullptr;

        //
        std::vector<glm::vec4> gSkyColor = {
                glm::vec4(0.9f,0.98,0.999f, 1.f),
                glm::vec4(0.9f,0.98,0.999f, 1.f),
                glm::vec4(0.9f,0.98,0.999f, 1.f),
                glm::vec4(0.9f,0.98,0.999f, 1.f)
        };

        { //
            int ret = LoadEXR(&rgba, &width, &height, "spiaggia_di_mondello_4k.exr", &err);
            if (ret != 0) {
                printf("err: %s\n", err); //

                // backdoor image
                rgba = (float*)gSkyColor.data();
                width = 2u, height = 2u;
            }
        };

        //
        std::vector<vkt::ImageRegion> images = {};

        {   //
            images.push_back(vkt::ImageRegion(std::make_shared<vkt::VmaImageAllocation>(fw.getAllocator(), vkh::VkImageCreateInfo{  // experimental: callify
                    .format = VK_FORMAT_R32G32B32A32_SFLOAT,
                    .extent = vkh::VkExtent3D{uint32_t(width),uint32_t(height),1u},
                    .usage = imageUsage,
            }, vkt::VmaMemoryInfo{ .memUsage = VMA_MEMORY_USAGE_GPU_ONLY }), vkh::VkImageViewCreateInfo{
                    .format = VK_FORMAT_R32G32B32A32_SFLOAT,
                    .subresourceRange = apres
            }));

            //
            auto image = images.back();

            //
            vkt::Vector<> imageBuf = {};
            if (width > 0u && height > 0u && rgba) {
                imageBuf = vkt::Vector<>(std::make_shared<vkt::VmaBufferAllocation>(fw.getAllocator(), vkh::VkBufferCreateInfo{ // experimental: callify
                    .size = size_t(width) * size_t(height) * sizeof(glm::vec4), .usage = uploadUsage,
                }, vkt::VmaMemoryInfo{ .memUsage = VMA_MEMORY_USAGE_CPU_TO_GPU }));
                memcpy(imageBuf.data(), rgba, size_t(width) * size_t(height) * sizeof(glm::vec4));
            };

            //
            context->getThread()->submitOnce([&](VkCommandBuffer& cmd) {
                image.transfer(cmd);

                auto buffer = imageBuf;
                fw->getDeviceDispatch()->CmdCopyBufferToImage(cmd, buffer.buffer(), image.getImage(), image.getImageLayout(), 1u, vkh::VkBufferImageCopy{
                        .bufferOffset = buffer.offset(),
                        .bufferRowLength = uint32_t(width),
                        .bufferImageHeight = uint32_t(height),
                        .imageSubresource = image.subresourceLayers(),
                        .imageOffset = {0u,0u,0u},
                        .imageExtent = {uint32_t(width),uint32_t(height),1u},
                });
            });

            //
            material->setBackgroundImage(image);
        };
    };



    //
    std::vector<jvx::MeshInput> inputs = {};

    //
    std::string inputfile = "sphere.obj";
    tinyobj::attrib_t attrib = {};
    std::vector<tinyobj::shape_t> shapes = {};
    std::vector<tinyobj::material_t> materials = {};

    //
    std::string warn = "";
    std::string err = "";

    //
    bool ret = tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, inputfile.c_str());
    if (!warn.empty()) { std::cout << warn << std::endl; }
    if (!err.empty()) { std::cerr << err << std::endl; }
    if (!ret) { exit(1); }

    // Loop over shapes
    std::vector<std::vector<VkDeviceSize>> primitiveCountPer = {};
    std::vector<VkDeviceSize> vertexCountAll = {};
    for (size_t s = 0; s < shapes.size(); s++) {
        vertexCountAll.push_back(0ull);
        primitiveCountPer.push_back({});
        for (size_t f = 0; f < shapes[s].mesh.num_face_vertices.size(); f++) { //
            primitiveCountPer.back().push_back(vertexCountAll[s]);
            vertexCountAll.back() += shapes[s].mesh.num_face_vertices[f];
        };
    };

    //
    for (size_t s = 0; s < shapes.size(); s++) {
        jvx::MeshBinding mBinding(context, vertexCountAll[s], primitiveCountPer[s]);
        meshes.push_back(mBinding->setIndexCount(vertexCountAll[s])->sharedPtr());

        size_t index_offset = 0; // Loop over faces(polygon)
        for (size_t f = 0; f < shapes[s].mesh.num_face_vertices.size(); f++) { //
            jvx::MeshInput mInput(context); inputs.push_back(mInput);

            //
            auto verticeCount = shapes[s].mesh.num_face_vertices[f];
            for (size_t v = 0; v < verticeCount; v++) { //
                tinyobj::index_t idx = shapes[s].mesh.indices[index_offset + v];
                tinyobj::real_t vx = attrib.vertices[3*idx.vertex_index+0], vy = attrib.vertices[3*idx.vertex_index+1], vz = attrib.vertices[3*idx.vertex_index+2];
                tinyobj::real_t nx = attrib.normals[3*idx.normal_index+0], ny = attrib.normals[3*idx.normal_index+1], nz = attrib.normals[3*idx.normal_index+2];
                tinyobj::real_t tx = attrib.texcoords[2*idx.texcoord_index+0], ty = attrib.texcoords[2*idx.texcoord_index+1];

                //
                buffersViews.back()[index_offset+v].vertex = glm::vec4(vx,vy,vz,1.f);
                buffersViews.back()[index_offset+v].normal = glm::vec4(nx,ny,nz,0.f);
                buffersViews.back()[index_offset+v].texcoord = glm::vec4(tx,ty,0.f,0.f);
            };
            index_offset += verticeCount;

            //
            buffersViews.push_back(vkt::Vector<VertexUnit>(std::make_shared<vkt::VmaBufferAllocation>(fw->getAllocator(), vkh::VkBufferCreateInfo{ .size = verticeCount * sizeof(VertexUnit), .usage = bufferUsage }, vkt::VmaMemoryInfo{ .memUsage = VMA_MEMORY_USAGE_CPU_TO_GPU })));

            //
            auto bufferIndex = bvse->pushBufferView(vkt::Vector<uint8_t>(buffersViews.back().getAllocation(), VkDeviceSize(0ull), VkDeviceSize(buffersViews.back().range()), VkDeviceSize(buffersViews.back().stride())));
            mInput->linkBViewSet(bvse)->addBinding(bufferIndex, vkh::VkVertexInputBindingDescription{ .binding = uint32_t(bufferIndex), .stride = uint32_t(buffersViews.back().range()) }); // TODO: USE SAME BINDING
            mInput->addAttribute(vkh::VkVertexInputAttributeDescription{ .location = 0, .format = VK_FORMAT_R32G32B32_SFLOAT, .offset = offsetof(VertexUnit, vertex) });
            mInput->addAttribute(vkh::VkVertexInputAttributeDescription{ .location = 1, .format = VK_FORMAT_R32G32B32_SFLOAT, .offset = offsetof(VertexUnit, texcoord) });
            mInput->addAttribute(vkh::VkVertexInputAttributeDescription{ .location = 2, .format = VK_FORMAT_R32G32_SFLOAT, .offset = offsetof(VertexUnit, normal) });

            // material support NOT added...
            //mBinding.addMeshInput(mInput, shapes[s].mesh.material_ids[f]);
            mBinding.addMeshInput(mInput, 0u);
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
    context->setModelView(glm::mat4x4(glm::lookAt(eye, foc, glm::dvec3(0.f, 1.f, 0.f))));
    context->setPerspective(glm::mat4x4(glm::perspective(80.f / 180.f * glm::pi<double>(), double(canvasWidth) / double(canvasHeight), 0.001, 10000.)));

    //
    vkh::VsGraphicsPipelineCreateInfoConstruction pipelineInfo = {};
    pipelineInfo.stages = {
            vkt::makePipelineStageInfo(fw->getDeviceDispatch(), vkt::readBinary(std::string("./shaders/rtrace/render.vert.spv")), VK_SHADER_STAGE_VERTEX_BIT),
            vkt::makePipelineStageInfo(fw->getDeviceDispatch(), vkt::readBinary(std::string("./shaders/rtrace/render.frag.spv")), VK_SHADER_STAGE_FRAGMENT_BIT)
    };
    pipelineInfo.graphicsPipelineCreateInfo.layout = context->getPipelineLayout();
    pipelineInfo.graphicsPipelineCreateInfo.renderPass = fw->applicationWindow.renderPass;//context->refRenderPass();
    pipelineInfo.inputAssemblyState.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;
    pipelineInfo.viewportState.pViewports = &reinterpret_cast<vkh::VkViewport&>(viewport);
    pipelineInfo.viewportState.pScissors = &reinterpret_cast<vkh::VkRect2D&>(renderArea);
    pipelineInfo.colorBlendAttachmentStates = { {} }; // Default Blend State
    pipelineInfo.dynamicStates = { VK_DYNAMIC_STATE_SCISSOR, VK_DYNAMIC_STATE_VIEWPORT };

    //
    VkPipeline finalPipeline = {};
    vkh::handleVk(fw->getDeviceDispatch()->CreateGraphicsPipelines(fw->getPipelineCache(), 1u, pipelineInfo, nullptr, &finalPipeline));

    //
    int64_t currSemaphore = -1;
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
    while (!glfwWindowShouldClose(manager.window)) { //
        glfwPollEvents();

        //
        int64_t n_semaphore = currSemaphore, c_semaphore = (currSemaphore + 1) % framebuffers.size(); // Next Semaphore
        currSemaphore = (c_semaphore = c_semaphore >= 0 ? c_semaphore : int64_t(framebuffers.size()) + c_semaphore); // Current Semaphore
        (n_semaphore = n_semaphore >= 0 ? n_semaphore : int64_t(framebuffers.size()) + n_semaphore); // Fix for Next Semaphores

        //
        vkh::handleVk(fw->getDeviceDispatch()->WaitForFences(1u, &framebuffers[c_semaphore].waitFence, true, 30ull * 1000ull * 1000ull * 1000ull));
        vkh::handleVk(fw->getDeviceDispatch()->ResetFences(1u, &framebuffers[c_semaphore].waitFence));
        vkh::handleVk(fw->getDeviceDispatch()->AcquireNextImageKHR(swapchain, std::numeric_limits<uint64_t>::max(), framebuffers[c_semaphore].presentSemaphore, nullptr, &currentBuffer));
        //fw->getDeviceDispatch()->SignalSemaphore(vkh::VkSemaphoreSignalInfo{.semaphore = framebuffers[n_semaphore].semaphore, .value = 1u});

        { // submit rendering (and wait presentation in device)
            std::vector<vkh::VkClearValue> clearValues = {vkh::VkClearValue{.color = vkh::VkClearColorValue{glm::vec4(0.f, 0.f, 0.f, 0.f)}}, vkh::VkClearValue{.depthStencil = vkh::VkClearDepthStencilValue{1.0f, 0} } };
            Shared::TimeCallback(double(context->registerTime()->setModelView(glm::mat4x4(cameraController->handle().project()))->drawTime()));

            // Create render submission
            std::vector<VkSemaphore> waitSemaphores = { framebuffers[currentBuffer].presentSemaphore }, signalSemaphores = { framebuffers[currentBuffer].computeSemaphore };
            std::vector<vkh::VkPipelineStageFlags> waitStages = {
                    vkh::VkPipelineStageFlags{.eFragmentShader = 1, .eComputeShader = 1, .eTransfer = 1, .eRayTracingShader = 1, .eAccelerationStructureBuild = 1 },
                    vkh::VkPipelineStageFlags{.eFragmentShader = 1, .eComputeShader = 1, .eTransfer = 1, .eRayTracingShader = 1, .eAccelerationStructureBuild = 1 }
            };

            // Submit command once
            //renderer->setupCommands();
            vkh::handleVk(fw->getDeviceDispatch()->QueueSubmit(queue, 1u, vkh::VkSubmitInfo{
                    .waitSemaphoreCount = static_cast<uint32_t>(waitSemaphores.size()), .pWaitSemaphores = waitSemaphores.data(), .pWaitDstStageMask = waitStages.data(),
                    .commandBufferCount = 1u, .pCommandBuffers = &renderer->setupCommands()->refCommandBuffer(),
                    .signalSemaphoreCount = static_cast<uint32_t>(signalSemaphores.size()), .pSignalSemaphores = signalSemaphores.data()
            }, {}));

            //
            waitSemaphores = { framebuffers[currentBuffer].computeSemaphore }, signalSemaphores = { framebuffers[currentBuffer].drawSemaphore };

            // create command buffer (with rewrite)
            VkCommandBuffer& commandBuffer = framebuffers[currentBuffer].commandBuffer;
            if (!commandBuffer) {
                commandBuffer = vkt::createCommandBuffer(fw->getDeviceDispatch(), commandPool, false, false); // do reference of cmd buffer

                // Already present, prepare to render
                vkt::imageBarrier(commandBuffer, vkt::ImageBarrierInfo{
                        .image = framebuffers[currentBuffer].image,
                        .targetLayout = VK_IMAGE_LAYOUT_GENERAL,
                        .originLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
                        .subresourceRange = vkh::VkImageSubresourceRange{ {}, 0u, 1u, 0u, 1u }.also([=](auto* it) {
                            auto aspect = vkh::VkImageAspectFlags{ .eColor = 1u };
                            it->aspectMask = aspect;
                            return it;
                        })
                });

                // Already present, prepare to render
                vkt::imageBarrier(commandBuffer, vkt::ImageBarrierInfo{
                        .image = fw->getDepthImage(),
                        .targetLayout = VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_STENCIL_ATTACHMENT_OPTIMAL,
                        .originLayout = VK_IMAGE_LAYOUT_GENERAL,
                        .subresourceRange = vkh::VkImageSubresourceRange{ {}, 0u, 1u, 0u, 1u }.also([=](auto* it) {
                            auto aspect = vkh::VkImageAspectFlags{.eDepth = 1u, .eStencil = 1u };
                            it->aspectMask = aspect;
                            return it;
                        })
                });

                //
                decltype(auto) descriptorSets = context->getDescriptorSets();
                fw->getDeviceDispatch()->CmdBeginRenderPass(commandBuffer, vkh::VkRenderPassBeginInfo{.renderPass = fw->applicationWindow.renderPass, .framebuffer = framebuffers[currentBuffer].frameBuffer, .renderArea = renderArea, .clearValueCount = static_cast<uint32_t>(clearValues.size()), .pClearValues = clearValues.data()}, VK_SUBPASS_CONTENTS_INLINE);
                fw->getDeviceDispatch()->CmdSetViewport(commandBuffer, 0u, 1u, viewport);
                fw->getDeviceDispatch()->CmdSetScissor(commandBuffer, 0u, 1u, renderArea);
                fw->getDeviceDispatch()->CmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, finalPipeline);
                fw->getDeviceDispatch()->CmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, context->getPipelineLayout(), 0u, descriptorSets.size(), descriptorSets.data(), 0u, nullptr);
                fw->getDeviceDispatch()->CmdDraw(commandBuffer, 4, 1, 0, 0);
                fw->getDeviceDispatch()->CmdEndRenderPass(commandBuffer);
                vkt::commandBarrier(fw->getDeviceDispatch(), commandBuffer);
                fw->getDeviceDispatch()->EndCommandBuffer(commandBuffer);
            };

            // Submit command once
            vkh::handleVk(fw->getDeviceDispatch()->QueueSubmit(queue, 1u, vkh::VkSubmitInfo{
                    .waitSemaphoreCount = static_cast<uint32_t>(waitSemaphores.size()), .pWaitSemaphores = waitSemaphores.data(), .pWaitDstStageMask = waitStages.data(),
                    .commandBufferCount = 1u, .pCommandBuffers = &commandBuffer,
                    .signalSemaphoreCount = static_cast<uint32_t>(signalSemaphores.size()), .pSignalSemaphores = signalSemaphores.data()
            }, framebuffers[currentBuffer].waitFence));

            //
            context->setDrawCount(frameCount++);
        };

        //
        std::vector<VkSemaphore> waitSemaphoes = { framebuffers[c_semaphore].drawSemaphore };
        vkh::handleVk(fw->getDeviceDispatch()->QueuePresentKHR(queue, vkh::VkPresentInfoKHR{
                .waitSemaphoreCount = static_cast<uint32_t>(waitSemaphoes.size()), .pWaitSemaphores = waitSemaphoes.data(),
                .swapchainCount = 1, .pSwapchains = &swapchain,
                .pImageIndices = &currentBuffer, .pResults = nullptr
        }));

    };

    //
    glfwDestroyWindow(manager.window);
    glfwTerminate(); exit(0);
    return 0;
};
