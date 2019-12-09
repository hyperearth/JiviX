//#pragma once

//#define VMA_IMPLEMENTATION
//#define RADX_IMPLEMENTATION



#undef small
#define small char
#undef small
#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include "../base/appRenderer.hpp"
#undef small

// initial renderer
const uint32_t canvasWidth = 640, canvasHeight = 360; // 
const bool enableSuperSampling = true;
std::shared_ptr<rnd::Renderer> renderer = nullptr;

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    renderer->UpdateFramebuffers(width, height);
    //glViewport(0, 0, width, height);
}

// main program with handling
int main(int argc, char** argv) {

    // initialize API
    if (!glfwInit() || !glfwVulkanSupported()) exit(EXIT_FAILURE);
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    // initialize renderer
    renderer = std::make_shared<rnd::Renderer>();
    renderer->Arguments(argc, argv);
    renderer->Init(canvasWidth, canvasHeight, enableSuperSampling); // init GLFW window there
    renderer->InitPipeline();

    glfwSetWindowSizeCallback(renderer->window, &framebuffer_size_callback);

    // looping rendering
    while (!glfwWindowShouldClose(renderer->window)) {
        glfwPollEvents();
        renderer->Draw();
        renderer->HandleData();
    };

    glfwDestroyWindow(renderer->window);
    glfwTerminate();
    return 0;
};
