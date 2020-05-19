// #
#define VMA_IMPLEMENTATION
#define TINYOBJLOADER_IMPLEMENTATION
#define VKT_FORCE_VMA_IMPLEMENTATION
#define VK_ENABLE_BETA_EXTENSIONS
#define VKT_ENABLE_GLFW_SUPPORT
#define TINYEXR_IMPLEMENTATION

#ifdef WIN32
#define VK_USE_PLATFORM_WIN32_KHR
//#define VKT_ENABLE_GLFW_SUPPORT
#endif

#define FORCE_RAY_TRACING

// 
#include <iostream>
#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>
#include <vkt3/fw.hpp>
#include <JiviX/JiviX.hpp>
//#include <misc/tiny_gltf.h>
#include <misc/tinyexr.h>

// 
#include <glbinding-aux/debug.h>
#include <glbinding/getProcAddress.h>

#define TINYGLTF_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "misc/tiny_gltf.h"


#if defined(ENABLE_OPENGL_INTEROP) && !defined(VKT_USE_GLAD)
using namespace gl;
#endif

class RayTracer : public std::enable_shared_from_this<RayTracer> { public: 
	 ~RayTracer() {};
	  RayTracer() {
		  this->fw = std::make_shared<vkt::GPUFramework>();
	};

	
	std::shared_ptr<vkt::GPUFramework> fw = {};
};

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow *window);

// settings
const unsigned int SCR_WIDTH = 1600;
const unsigned int SCR_HEIGHT = 1200;


const char *vertexShaderSource = "#version 460 compatibility\n"
    "layout (location = 0) in vec4 aPos;\n" 
    "void main()\n"
    "{\n"
    "   gl_Position = vec4(aPos.xyz, 1.0f);\n"
    "}\0";


const char *fragmentShaderSource = "#version 460 compatibility\n"
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



struct FDStruct {
    glm::vec4 fPosition = glm::vec4(1.f);
    glm::vec4 fTexcoord = glm::vec4(1.f);
    glm::vec4 fNormal = glm::vec4(1.f);
    glm::vec4 fTangent = glm::vec4(1.f);
    glm::vec4 fBinormal = glm::vec4(1.f);
};



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
    glFinish();
    return errorCode;
}

#define glCheckError() glCheckError_(__FILE__, __LINE__) 


void error(int errnum, const char* errmsg)
{
    std::cerr << errnum << ": " << errmsg << std::endl;
}


void key_callback(GLFWwindow* window, int key, int /*scancode*/, int action, int /*mods*/)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
    {
        glfwSetWindowShouldClose(window, 1);
    }
}

int main() {
    // glfw: initialize and configure
    // ------------------------------
    glfwSetErrorCallback(error);
    glfwInit();

    // 
    if (GLFW_FALSE == glfwVulkanSupported()) {
        glfwTerminate(); return -1;
    };

    // glfw: window and context hints
    glfwDefaultWindowHints();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_COMPAT_PROFILE);
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, true);
    glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_API);
    glfwWindowHint(GLFW_CONTEXT_CREATION_API, GLFW_NATIVE_CONTEXT_API);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, true);

    // glfw window creation
    // --------------------
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "LearnOpenGL", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);



	// initialize Vulkan
    auto fw = jvx::Driver();
    fw->getAppObject().window = window;

    // Ininitialize by GlBinding
    //glfwMakeContextCurrent(appObj.opengl = glfwCreateWindow(canvasWidth, canvasHeight, "GLTest", nullptr, nullptr));
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

    // build and compile our shader program
    // ------------------------------------
    const int vertexTFShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexTFShader, 1, &vertexTFShaderSource, NULL);
    glCompileShader(vertexTFShader);
    // check for shader compile errors
    int success; char infoLog[512];
    glGetShaderiv(vertexTFShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(vertexTFShader, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
    }
    glCheckError();

    // 
    const int geometryTFShader = glCreateShader(GL_GEOMETRY_SHADER);
    glShaderSource(geometryTFShader, 1, &geometryTFShaderSource, NULL);
    glCompileShader(geometryTFShader);
    // check for shader compile errors
    glGetShaderiv(geometryTFShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(geometryTFShader, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
    }
    glCheckError();


    // link shaders
    const int shaderTFProgram = glCreateProgram();
    const char* varyings[] = { "fPosition", "fTexcoord", "fNormal", "fTangent", "fBinormal" };
    glTransformFeedbackVaryings(shaderTFProgram, 5, varyings, GL_INTERLEAVED_ATTRIBS);
    glAttachShader(shaderTFProgram, vertexTFShader);
    glAttachShader(shaderTFProgram, geometryTFShader);
    glLinkProgram(shaderTFProgram);
    glValidateProgram(shaderTFProgram);
    glCheckError();
    // check for linking errors
    glGetProgramiv(shaderTFProgram, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(shaderTFProgram, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
    }
    //glDeleteShader(vertexTFShader);
    glCheckError();


    // vertex shader
    const int vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);
    // check for shader compile errors
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
    }
    glCheckError();


    // fragment shader
    const int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);
    // check for shader compile errors
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
    }
    glCheckError();


    // link shaders
    const int shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);
    glValidateProgram(shaderProgram);
    glCheckError();
    // check for linking errors
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
    }
    //glDeleteShader(vertexShader);
    //glDeleteShader(fragmentShader);
    glCheckError();


    // 
    float tf_vertices[] = {
        -1.f, -1.f, -1.f,
         1.f, -1.f, -1.f,
         0.f,  1.f, -1.f,
         2.f,  1.f, -1.f
    };

    unsigned int TFVBO, TFVAO;
    glGenVertexArrays(1, &TFVAO);
    glBindVertexArray(TFVAO);
    glCheckError();

    glGenBuffers(1, &TFVBO);
    glBindBuffer(GL_ARRAY_BUFFER, TFVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(tf_vertices), tf_vertices, GL_STATIC_DRAW);
    glCheckError();

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glCheckError();


    // 
    float vertices[] = {
        -1.f, -1.f, 0.f, // left  
         1.f, -1.f, 0.f, // right 
         1.f,  1.f, 0.f, // top   
        -1.f,  1.f, 0.f
    };

    unsigned int VBO, VAO;
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);
    glCheckError();

    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glCheckError();

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glCheckError();

    // Debug View
    std::vector<FDStruct> outScript(6u);



	auto instance = fw->createInstance();
	auto physicalDevice = fw->getPhysicalDevice(0u);
	auto device = fw->createDevice(true,"./",false);
	auto queue = fw->getQueue();
	auto commandPool = fw->getCommandPool();

    // 
    auto context = jvx::Context(fw);
    auto node = jvx::Node(context);
    auto material = jvx::Material(context);
    auto renderer = jvx::Renderer(context);

	// initialize renderer
    context->initialize(SCR_WIDTH, SCR_HEIGHT);
    renderer->linkMaterial(material)->linkNode(node);

    // 
    auto meshTest0 = jvx::MeshInput(context);
    auto meshTest1 = jvx::MeshInput(context);
    auto meshBinding = jvx::MeshBinding(context, 4u, std::vector<vk::DeviceSize>{ 2u, 2u });
    auto bufferViewS = jvx::BufferViewSet(context);

    // 
    auto TRS = vkt::Vector<glm::mat3x4>(std::make_shared<vkt::VmaBufferAllocation>(fw->getAllocator(), vkh::VkBufferCreateInfo{
        .size = sizeof(glm::mat3x4) * 2ull, .usage = {.eTransferSrc = 1, .eStorageTexelBuffer = 1, .eStorageBuffer = 1, .eIndexBuffer = 1, .eVertexBuffer = 1, .eRayTracing = 1, .eSharedDeviceAddress = 1 },
    }, vkt::VmaMemoryInfo{.memUsage = VMA_MEMORY_USAGE_CPU_TO_GPU }));

    // 
    TRS[0] = glm::mat3x4(1.f);
    TRS[1] = glm::transpose(glm::translate(glm::mat4x4(1.f), glm::vec3(0.5f, -0.5f, 0.f)));

    // 
    meshBinding->setTransformData(TRS)->addRangeInput(2u)->setIndexCount(6u);
    //meshBinding->addRangeInput(2u)->setIndexCount(6u);

    // 
    node->pushInstance(vkh::VsGeometryInstance{
        .instanceId = uint32_t(node->pushMesh(meshBinding->sharedPtr())),
        .mask = 0xFF,
        .instanceOffset = 0,
        .flags = {}
    });

	// Orange
    jvi::MaterialUnit mdk = {};
    mdk.diffuse = glm::vec4(1.f, 0.75f, 0.f, 1.f);
    mdk.diffuseTexture = -1;
    //mdk.emission = glm::vec4(1.f, 0.75f, 0.f, 1.f);
    //mdk.emissionTexture = -1;
	material->pushMaterial(mdk);

    // Blue
    mdk.diffuse = glm::vec4(0.f, 0.75f, 1.f, 1.f);
    mdk.diffuseTexture = -1;
    //mdk.emission = glm::vec4(0.f, 0.75f, 1.f, 1.f);
    //mdk.emissionTexture = -1;
    material->pushMaterial(mdk);


    std::cout << "Materialized" << std::endl;

    /*
    // HDRI RenderDoc CORRUPT! (i.e. CRASH runtime with Unable to Debug through Visual Studio 2019)
    vkt::ImageRegion image = {};
    {
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

        if (width >= 0 && height >= 0 && rgba) { // 
            image = vkt::ImageRegion(std::make_shared<vkt::VmaImageAllocation>(fw.getAllocator(), vkh::VkImageCreateInfo{  // experimental: callify
                .format = VK_FORMAT_R32G32B32A32_SFLOAT,
                .extent = {uint32_t(width),uint32_t(height),1u},
                .usage = {.eTransferDst = 1, .eSampled = 1, .eStorage = 1, .eColorAttachment = 1 },
            }, vkt::VmaMemoryInfo{ .memUsage = VMA_MEMORY_USAGE_GPU_ONLY, .deviceDispatch = fw->getDeviceDispatch(), .instanceDispatch = fw->getInstanceDispatch() }), vkh::VkImageViewCreateInfo{
                .format = VK_FORMAT_R32G32B32A32_SFLOAT,
            });

            // Create Sampler By Reference
            vkh::handleVk(fw->getDeviceDispatch()->CreateSampler(vkh::VkSamplerCreateInfo{
                .magFilter = VK_FILTER_LINEAR,
                .minFilter = VK_FILTER_LINEAR,
                .addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT,
                .addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
            }, nullptr, &image.refSampler()));

            // 
            vkt::Vector<> imageBuf = {};
            if (width > 0u && height > 0u && rgba) {
                imageBuf = vkt::Vector<>(std::make_shared<vkt::VmaBufferAllocation>(fw.getAllocator(), vkh::VkBufferCreateInfo{ // experimental: callify
                    .size = size_t(width) * size_t(height) * sizeof(glm::vec4),
                    .usage = {.eTransferSrc = 1, .eStorageTexelBuffer = 1, .eStorageBuffer = 1, .eIndexBuffer = 1, .eVertexBuffer = 1 },
                    }, vkt::VmaMemoryInfo{ .memUsage = VMA_MEMORY_USAGE_CPU_TO_GPU, .deviceDispatch = fw->getDeviceDispatch(), .instanceDispatch = fw->getInstanceDispatch() }));
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

            // ONLY FOR TEST!
            material->setBackgroundImage(image);
        };
    };
    std::cout << "HDRI Initialized" << std::endl;
    */

    // 
    //renderer->setupCommands();

	// 
	struct ShareHandles {
		//HANDLE memory{ INVALID_HANDLE_VALUE };
		HANDLE glReady{ INVALID_HANDLE_VALUE };
		HANDLE glComplete{ INVALID_HANDLE_VALUE };
	} handles;

	// 
	struct Semaphores {
        VkSemaphore glReady = {}, glComplete = {};
	} semaphores;

    { // Vulkan Semaphores
        const auto exportable = vkh::VkExportSemaphoreCreateInfo{ .handleTypes = {.eOpaqueWin32 = 1} };
        vkh::handleVk(fw->getDeviceDispatch()->CreateSemaphoreA(vkh::VkSemaphoreCreateInfo{ .pNext = &exportable }, nullptr, &semaphores.glReady));
        vkh::handleVk(fw->getDeviceDispatch()->CreateSemaphoreA(vkh::VkSemaphoreCreateInfo{ .pNext = &exportable }, nullptr, &semaphores.glComplete));
    };
    { // On non-Win32 systems needs use glImportSemaphoreFdEXT instead
        vkh::handleVk(fw->getDeviceDispatch()->GetSemaphoreWin32HandleKHR(vkh::VkSemaphoreGetWin32HandleInfoKHR{ .semaphore = semaphores.glReady, .handleType = VK_EXTERNAL_SEMAPHORE_HANDLE_TYPE_OPAQUE_WIN32_BIT }, &handles.glReady));
        vkh::handleVk(fw->getDeviceDispatch()->GetSemaphoreWin32HandleKHR(vkh::VkSemaphoreGetWin32HandleInfoKHR{ .semaphore = semaphores.glComplete, .handleType = VK_EXTERNAL_SEMAPHORE_HANDLE_TYPE_OPAQUE_WIN32_BIT }, &handles.glComplete));
    };

	// Platform specific import.
    GLuint glReady = 0;
    glGenSemaphoresEXT(1, &glReady);
	glImportSemaphoreWin32HandleEXT(glReady, GL_HANDLE_TYPE_OPAQUE_WIN32_EXT, handles.glReady);
    glCheckError();

    // 
    GLuint glComplete = 0;
    glGenSemaphoresEXT(1, &glComplete);
	glImportSemaphoreWin32HandleEXT(glComplete, GL_HANDLE_TYPE_OPAQUE_WIN32_EXT, handles.glComplete);
    glCheckError();

    // 
    GLuint color = context->getFrameBuffers()[8].getGL();
    glTextureParameteri(color, GL_TEXTURE_WRAP_S, GL_REPEAT);	// set texture wrapping to GL_REPEAT (default wrapping method)
    glTextureParameteri(color, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTextureParameteri(color, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTextureParameteri(color, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glCheckError();

    // 
    GLenum layoutSignal = GL_LAYOUT_GENERAL_EXT;
    GLenum layoutWait = GL_LAYOUT_COLOR_ATTACHMENT_EXT;//GL_LAYOUT_GENERAL_EXT;

    // 
    GLuint64 colorHandle = 0;

    // 
    auto GLID = meshBinding->getBindingBufferGL();

    // 
    VkFence waitFence = {};
    vkh::handleVk(fw->getDeviceDispatch()->CreateFence(vkh::VkFenceCreateInfo{ .flags{1} }, nullptr, &waitFence));

    //
    glfwSwapInterval(1);

    // 
    //renderer->setupCommands({}, false); // 
    while (!glfwWindowShouldClose(window))
    {
        processInput(window);

        // 
        glUseProgram(shaderTFProgram);
        glBindVertexArray(TFVAO);
        glEnable(GL_RASTERIZER_DISCARD);
        glCheckError();

        // Do transform feedback
        glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0u, GLID);
        glBeginTransformFeedback(GL_TRIANGLES);
        glDrawArrays(GL_LINES_ADJACENCY, 0, 4);
        glEndTransformFeedback();
        glCheckError();
        
        // Signal for Vulkan
        glSignalSemaphoreEXT(glComplete, 1, &GLID, 1, &color, &layoutSignal);
        glCheckError();
        
        // Fence a Vulkan call...
        vkh::handleVk(fw->getDeviceDispatch()->WaitForFences(1u, &waitFence, true, 30ull * 1000ull * 1000ull * 1000ull));
        vkh::handleVk(fw->getDeviceDispatch()->ResetFences(1u, &waitFence));
        //glFinish(); // For Debug

		// Ray-Trace!
        std::vector<vkh::VkPipelineStageFlags> waitStages = { vkh::VkPipelineStageFlags{ .eFragmentShader = 1, .eComputeShader = 1, .eTransfer = 1, .eRayTracingShader = 1, .eAccelerationStructureBuild = 1 } };
        vkh::handleVk(fw->getDeviceDispatch()->QueueSubmit(queue, 1u, vkh::VkSubmitInfo{
            .waitSemaphoreCount = 1u, .pWaitSemaphores = &semaphores.glComplete, .pWaitDstStageMask = waitStages.data(),
            .commandBufferCount = 1u, .pCommandBuffers = &renderer->setupCommands()->refCommandBuffer(),
            .signalSemaphoreCount = 1u, .pSignalSemaphores = &semaphores.glReady
        }, waitFence));

        // Wait in OpenGL side...
        glWaitSemaphoreEXT(glReady, 1, &GLID, 1, &color, &layoutWait);
        glCheckError();

        // 
        //glGetNamedBufferSubData(GLID, 0u, outScript.size() * sizeof(FDStruct), outScript.data());

		// Draw Result...
        glDisable(GL_RASTERIZER_DISCARD);
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        glBindTextureUnit(0, color);
        glUseProgram(shaderProgram);
        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
        //glFinish(); // For Debug
        glCheckError();

		// 
        //glFinish();
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

	// 
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glCheckError();

    glfwTerminate();
    return 0;
}

void processInput(GLFWwindow *window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    // make sure the viewport matches the new window dimensions; note that width and 
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
}