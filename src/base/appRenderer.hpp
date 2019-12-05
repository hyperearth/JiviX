#pragma once

#ifndef NOMINMAX
#define NOMINMAX
#endif

#include "../../include/vkt/fw.hpp"
//#include <tinygltf/tiny_gltf.h>

namespace rnd {
#define api vk

    // renderer biggest class 
    class Renderer : public std::enable_shared_from_this<Renderer> { 
        protected:
            

        public:
            std::shared_ptr<vkt::GPUFramework> appBase = {};
            lancer::DeviceMaker device = {};
            lancer::PhysicalDeviceHelper physicalHelper = {};
            GLFWwindow* window = nullptr; // bound GLFW window
            const uint32_t SGHZ = 16u;

            // 
            api::SwapchainKHR swapchain = {};
            std::vector<vkt::Framebuffer> framebuffers = {};

            // Pipeline Layout
            api::Pipeline trianglePipeline = {};
            api::PipelineLayout trianglePipelineLayout = {};

            // Descriptor Set
            api::DescriptorSet inputDescriptorSet_ = {};
            api::DescriptorSetLayout inputDescriptorLayout = {};
            lancer::DescriptorSetMaker inputDescriptorSet = {};

            // Ray Tracing Pipeline
            api::Buffer rtSBT = {};
            api::Pipeline rtPipeline = {};
            api::PipelineLayout rtPipelineLayout = {};
            lancer::SBTHelper rtSBThelper = {};

            // Output Image 
            api::Image outputImage_ = {};
            lancer::ImageMaker outputImage = {};

            // Acceleration Geometry
            std::vector<glm::mat3x4> transform = {};
            std::vector<glm::vec4> vertices = {};
            std::vector<uint32_t> indices = {};
            vkt::BufferUploader<glm::mat3x4> uTransform = {};
            vkt::BufferUploader<glm::vec4> uVertices = {};
            vkt::BufferUploader<uint32_t> uIndices = {};
            vkt::GeometryBuffer<uint32_t, glm::vec4> mGeometry = {};
            vkt::AccelerationGeometry rtAccelLow = {};

            // Acceleration Instanced
            vkt::AccelerationInstanced rtAccelTop = {};


            double tPastFrameTime = 0.f;
            float guiScale = 1.0f;
            uint32_t canvasWidth = 1, canvasHeight = 1; // canvas size with SSAA accounting
            uint32_t windowWidth = 1, windowHeight = 1; // virtual window size (without DPI recalculation)
            uint32_t realWidth = 1, realHeight = 1; // real window size (with DPI accounting)
            uint32_t gpuID = 0;//1;

            // current rendering state
            int32_t currSemaphore = -1; uint32_t currentBuffer = 0;

            // names and directories
            std::string title = "vkt";
            std::string shaderPrefix = "./", shaderPack = "./shaders/";

            // methods for rendering
            void Arguments(int argc = 0, char** argv = nullptr);
            void Init(uint32_t canvasWidth, uint32_t canvasHeight, bool enableSuperSampling = false);
            void InitPipeline();
            void Draw();
            void HandleData();
            void UpdateFramebuffers(uint32_t width, uint32_t height);

    };

};
