// #
#define VMA_IMPLEMENTATION
#define TINYOBJLOADER_IMPLEMENTATION
#define VKT_FORCE_VMA_IMPLEMENTATION
#define VK_ENABLE_BETA_EXTENSIONS
//#define VKT_ENABLE_GLFW_SUPPORT

#ifdef WIN32
#define VK_USE_PLATFORM_WIN32_KHR
#endif

#include <vkt2/fw.hpp>
#include <JiviX/JiviX.hpp>

#define TINYGLTF_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "misc/tiny_gltf.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>


class RayTracer : public std::enable_shared_from_this<RayTracer> { public: 
	 ~RayTracer() {};
	  RayTracer() {
		  this->fw = std::make_shared<vkt::GPUFramework>();
	};

	
	std::shared_ptr<vkt::GPUFramework> fw = {};
};

GLuint handleError() {
    GLuint errorCode = 0;
    //std::cout << (errorCode = glGetError()) << std::endl;
    assert(errorCode == 0);
    return errorCode;
}


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

int main()
{
    // glfw: initialize and configure
    // ------------------------------
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_COMPAT_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // uncomment this statement to fix compilation on OS X
#endif

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

    // glad: load all OpenGL function pointers
    // ---------------------------------------
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }


    // build and compile our shader program
    // ------------------------------------
    // vertex shader
    int vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);
    // check for shader compile errors
    int success;
    char infoLog[512];
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
    }
    // fragment shader
    int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);
    // check for shader compile errors
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
    }
    // link shaders
    int shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);
    // check for linking errors
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
    }
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    // set up vertex data (and buffer(s)) and configure vertex attributes
    // ------------------------------------------------------------------
    float vertices[] = {
        -1.f, -1.f, 0.f, // left  
         1.f, -1.f, 0.f, // right 
         1.f,  1.f, 0.f,  // top   
		-1.f,  1.f, 0.f
    };

    unsigned int VBO, VAO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    // bind the Vertex Array Object first, then bind and set vertex buffer(s), and then configure vertex attributes(s).
    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // note that this is allowed, the call to glVertexAttribPointer registered VBO as the vertex attribute's bound vertex buffer object so afterwards we can safely unbind
    glBindBuffer(GL_ARRAY_BUFFER, 0); 

    // You can unbind the VAO afterwards so other VAO calls won't accidentally modify this VAO, but this rarely happens. Modifying other
    // VAOs requires a call to glBindVertexArray anyways so we generally don't unbind VAOs (nor VBOs) when it's not directly necessary.
    glBindVertexArray(0); 




	// initialize Vulkan
    auto fw = jvx::Driver();
	auto instance = fw->createInstance();
	//auto manager = fw->createWindowSurface(SCR_WIDTH, SCR_HEIGHT);
	auto physicalDevice = fw->getPhysicalDevice(0u);
	auto device = fw->createDevice(true,"./",false);
	//auto swapchain = fw->createSwapchain();
	//auto renderPass = fw->createRenderPass();
	//auto framebuffers = fw->createSwapchainFramebuffer(swapchain, renderPass);
	auto queue = fw->getQueue();
	auto commandPool = fw->getCommandPool();

    // 
    auto context = jvx::Context(fw);
    auto node = jvx::Node(context);
    auto material = jvx::Material(context);
    auto renderer = jvx::Renderer(context);
    auto meshTest0 = jvx::MeshInput(context);
    auto meshTest1 = jvx::MeshInput(context);
    auto meshBinding = jvx::MeshBinding(context, 1024u, std::vector<vk::DeviceSize>{ 1024u, 1024u });
    auto bufferViewS = jvx::BufferViewSet(context);
    //auto meshPtr = meshTest.setThread(context->getThread());

	// initialize renderer
    context->initialize(SCR_WIDTH, SCR_HEIGHT);

    // 
    auto DMI = vkt::Vector<uint32_t>(std::make_shared<vkt::VmaBufferAllocation>(fw->getAllocator(), vkh::VkBufferCreateInfo{
        .size = 24u, .usage = {.eTransferSrc = 1, .eStorageTexelBuffer = 1, .eStorageBuffer = 1, .eIndexBuffer = 1, .eVertexBuffer = 1 },
    }, VMA_MEMORY_USAGE_GPU_ONLY));

	// 
    auto DMA = vkt::Vector<glm::vec4>(std::make_shared<vkt::VmaBufferAllocation>(fw->getAllocator(), vkh::VkBufferCreateInfo{
        .size = 128u, .usage = {.eTransferSrc = 1, .eStorageTexelBuffer = 1, .eStorageBuffer = 1, .eIndexBuffer = 1, .eVertexBuffer = 1 },
    }, VMA_MEMORY_USAGE_CPU_TO_GPU));

    // 
    auto DMB = vkt::Vector<glm::vec4>(std::make_shared<vkt::VmaBufferAllocation>(fw->getAllocator(), vkh::VkBufferCreateInfo{
        .size = 128u, .usage = {.eTransferSrc = 1, .eStorageTexelBuffer = 1, .eStorageBuffer = 1, .eIndexBuffer = 1, .eVertexBuffer = 1 },
    }, VMA_MEMORY_USAGE_CPU_TO_GPU));


    // 
    auto TRS = vkt::Vector<glm::mat3x4>(std::make_shared<vkt::VmaBufferAllocation>(fw->getAllocator(), vkh::VkBufferCreateInfo{
        .size = sizeof(glm::mat3x4) * 2ull, .usage = {.eTransferSrc = 1, .eStorageTexelBuffer = 1, .eStorageBuffer = 1, .eIndexBuffer = 1, .eVertexBuffer = 1, .eSharedDeviceAddress = 1 },
    }, VMA_MEMORY_USAGE_CPU_TO_GPU));

    // 
    TRS[0] = glm::transpose(glm::translate(glm::mat4x4(1.f), glm::vec3(0.5f, -0.5f, 0.f)));
    TRS[1] = glm::mat3x4(1.f);

    // 
    DMA[0u] = glm::vec4(-1.f, -1.f, -1.f, 1.f);
    DMA[1u] = glm::vec4( 1.f, -1.f, -1.f, 1.f);
    DMA[2u] = glm::vec4( 0.f,  1.f, -1.f, 1.f);
    DMA[3u] = glm::vec4( 2.f,  1.f, -1.f, 1.f);

    // 
    DMB[0u] = glm::vec4(-1.f,  1.f, -0.999f, 1.f);
    DMB[1u] = glm::vec4( 0.f, -1.f, -0.999f, 1.f);
    DMB[2u] = glm::vec4( 1.f,  1.f, -0.999f, 1.f);
    DMB[3u] = glm::vec4( 2.f, -1.f, -0.999f, 1.f);


    // 
    meshTest0->makeQuad()->linkBViewSet(bufferViewS)->addBinding(bufferViewS->pushBufferView(DMA), vkh::VkVertexInputBindingDescription{ .stride = sizeof(glm::vec4) });
    meshTest0->addAttribute(vkh::VkVertexInputAttributeDescription{ .location = 0, .format = VK_FORMAT_R32G32B32A32_SFLOAT, .offset = 0u });
    meshTest0->setIndexData(bufferViewS->pushBufferView(DMI), vk::IndexType::eUint32)->setIndexCount(4ull); // For Scratch (������ �����)

    // 
    meshTest1->makeQuad()->linkBViewSet(bufferViewS)->addBinding(bufferViewS->pushBufferView(DMB), vkh::VkVertexInputBindingDescription{ .stride = sizeof(glm::vec4) });
    meshTest1->addAttribute(vkh::VkVertexInputAttributeDescription{ .location = 0, .format = VK_FORMAT_R32G32B32A32_SFLOAT, .offset = 0u });
    meshTest1->setIndexData(bufferViewS->pushBufferView(DMI), vk::IndexType::eUint32)->setIndexCount(4ull); // For Scratch (������ �����)

    // 
    meshBinding->addMeshInput(meshTest0->sharedPtr(), 0u);
    meshBinding->addMeshInput(meshTest1->sharedPtr(), 1u);
    meshBinding->setTransformData(TRS);

    // 
    node->pushInstance(vkh::VsGeometryInstance{
        .instanceId = uint32_t(node->pushMesh(meshBinding->sharedPtr())),
        .mask = 0xFF,
        .instanceOffset = 0,
        .flags = {}
    });

	// Orange
    jvi::MaterialUnit mdk = {};
    mdk.diffuse = glm::vec4(1.f,0.75f,0.f,1.f);
    mdk.diffuseTexture = -1;
	material->pushMaterial(mdk);

    // Blue
    mdk.diffuse = glm::vec4(0.f, 0.75f, 1.f, 1.f);
    mdk.diffuseTexture = -1;
    material->pushMaterial(mdk);

    // 
    renderer->linkMaterial(material)->linkNode(node)->setupCommands();

	// 
	struct ShareHandles {
		//HANDLE memory{ INVALID_HANDLE_VALUE };
		HANDLE glReady{ INVALID_HANDLE_VALUE };
		HANDLE glComplete{ INVALID_HANDLE_VALUE };
	} handles;

	// 
	struct Semaphores {
        vk::Semaphore glReady = {}, glComplete = {};
	} semaphores;

	{
		auto handleType = vk::ExternalSemaphoreHandleTypeFlagBits::eOpaqueWin32;
		{
			vk::SemaphoreCreateInfo sci;
			vk::ExportSemaphoreCreateInfo esci;
			sci.pNext = &esci;
			esci.handleTypes = handleType;
			semaphores.glReady = device.createSemaphore(sci);
			semaphores.glComplete = device.createSemaphore(sci);
		}
		handles.glReady = device.getSemaphoreWin32HandleKHR({ semaphores.glReady, handleType }, fw->getDispatch());
		handles.glComplete = device.getSemaphoreWin32HandleKHR({ semaphores.glComplete, handleType }, fw->getDispatch());
	}

	// Platform specific import.  On non-Win32 systems use glImportSemaphoreFdEXT instead
    GLuint glReady = 0;
    glGenSemaphoresEXT(1, &glReady);
	glImportSemaphoreWin32HandleEXT(glReady, GL_HANDLE_TYPE_OPAQUE_WIN32_EXT, handles.glReady);
    handleError();

    // 
    GLuint glComplete = 0;
    glGenSemaphoresEXT(1, &glComplete);
	glImportSemaphoreWin32HandleEXT(glComplete, GL_HANDLE_TYPE_OPAQUE_WIN32_EXT, handles.glComplete);
    handleError();

    // 
    GLuint color = context->getFrameBuffers()[8].getGL();
    glTextureParameteri(color, GL_TEXTURE_WRAP_S, GL_REPEAT);	// set texture wrapping to GL_REPEAT (default wrapping method)
    glTextureParameteri(color, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTextureParameteri(color, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTextureParameteri(color, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // 
    GLenum layoutSignal = GL_LAYOUT_GENERAL_EXT;
    GLenum layoutWait = GL_LAYOUT_GENERAL_EXT;

    // 
    GLuint64 colorHandle = 0;

    /*
    vk::ImageViewHandleInfoNVX handleInfo = {};
    handleInfo.imageView = context->getFrameBuffers()[0].getImageView();
    handleInfo.sampler = fw->device.createSampler(vkh::VkSamplerCreateInfo{
        .magFilter = VK_FILTER_LINEAR,
        .minFilter = VK_FILTER_LINEAR,
        .addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
        .addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
        .unnormalizedCoordinates = false,
    });
    handleInfo.descriptorType = vk::DescriptorType::eCombinedImageSampler;
    colorHandle = fw->getDevice().getImageViewHandleNVX(&handleInfo, fw->getDispatch());
    */

    // 
    //auto addressProp = fw->getDevice().getImageViewAddressNVX(context->getFrameBuffers()[0].getImageView(), fw->getDispatch());
    //colorHandle = addressProp.deviceAddress;

    // 
    while (!glfwWindowShouldClose(window))
    {
        processInput(window);


        // 
        glSignalSemaphoreEXT(glComplete, 0, nullptr, 1, &color, &layoutSignal);

		// 
		std::vector<vk::PipelineStageFlags> waitStages = { vk::PipelineStageFlagBits::eFragmentShader | vk::PipelineStageFlagBits::eComputeShader | vk::PipelineStageFlagBits::eRayTracingShaderKHR };
		context->getThread()->submitCmd(std::vector<vk::CommandBuffer>{ renderer->refCommandBuffer() }, vk::SubmitInfo()
            .setPWaitDstStageMask(waitStages.data())
			.setPWaitSemaphores  (&semaphores.glComplete)  .setWaitSemaphoreCount(1)
            .setPSignalSemaphores(&semaphores.glReady   ).setSignalSemaphoreCount(1));

        // 
        glWaitSemaphoreEXT(glReady, 0, nullptr, 1, &color, &layoutWait);
        

		// 
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

		// 
        glBindTextureUnit(0, color);
        //glUniformHandleui64ARB(0, GLuint64(colorHandle));

        // draw our first triangle
        glUseProgram(shaderProgram);
        glBindVertexArray(VAO); // seeing as we only have a single VAO there's no need to bind it every time, but we'll do so to keep things a bit more organized
        glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
        // glBindVertexArray(0); // no need to unbind it every time 

		// 
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

	// 
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);

    glfwTerminate();
    return 0;
}

void processInput(GLFWwindow *window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
}

void framebuffer_size_callback(GLFWwindow*window, int width, int height)
{
    // make sure the viewport matches the new window dimensions; note that width and 
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
}
