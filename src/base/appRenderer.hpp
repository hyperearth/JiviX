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

        // Rasterization Pass
        // 1. Unify Into One Rendering Pass
        // 2. Render Texture, Unmod-Normals, Tex-Normals, Depth, Samples, PBR Buffers

        // Compute Pass (like in https://bitsquid.blogspot.com/2017/08/notes-on-screen-space-hiz-tracing.html)
        // 1. Create SSLR Acceleration Buffer (Hi-Z)

        // Reproject Diffuse Pass (like in http://remi-genin.fr/blog/screen-space-plane-indexed-reflection-in-ghost-recon-wildlands/)
        // 0. Clear Write-Only Diffuse Buffer
        // 1. Diffuse Buffers Can Only Accumulate Samples
        // 2. Diffuse Samples Reproject Into Current Frame (using Compute or Rasterization POINT Shaders)
        // 3. Switch Writed Diffuse Buffer Into Read (i.e. SWAP)

        // Ray-Tracing Pass (RTX and SSLR methods)
        // 0. Clear Write-Only Diffuse Buffer
        // 1. Ray-Tracing Current Diffuse And Accumulate Into (Writable) Diffuse (And Accumulate With Reprojected)
        // 2. Switch Writed Diffuse Buffer Into Read (i.e. SWAP)
        // 3. Do Denoise Into Denoised Buffer
        // 4. Non-Diffuse Make Reflections Only On Reflect Buffers

        // How to WE Will RTX Ray Tracing? 
        // 1. CheckerBoard defaulty... (CFR-Style)
        // 2. Will try to get some data by SSLR for faster (and yeah, from Ray-Tracing shader directly)

        // Other Usable Tech?
        // - Normal-Based SSLR tracing correction
        // - Selective Bilateral Renoise (By Depth And Normals Bias)
        // - Ray-Traced Transparency (for RTX) 

        // Final Pass
        // 1. Multiply Color Textures with Diffuses, Blend Transparency and Reflections...
        // 2. Add Some Effects... 


        // FrameBuffers and RenderPass
        api::RenderPass firstPassRenderPass = {}, 
                        reprojectRenderPass = {};
        api::Framebuffer firstPassFramebuffer = {}, 
                         reprojectFramebuffer = {};
        lancer::RenderPassMaker mFirstPassRenderPass = {},
                                mReprojectRenderPass = {};


        // Ping-Pong Buffers 
        std::array<api::Image, 2u> samplesBuffer = {};
        std::array<api::Image, 2u> diffuseBuffer = {};
        std::array<api::Image, 2u> reflectBuffer = {};

        // Mesh Rasterization 
        api::Image coloredBuffer = {};
        api::Image normalsBuffer = {};
        api::Image normmodBuffer = {};
        api::Image depthStBuffer = {}; // Depth Buffer
        api::Image parametBuffer = {};

        // Output and Denoised Buffers
        api::Image denoiseBuffer = {};
        api::Image outputsBuffer = {};

        // Native Samplers 
        api::Sampler sslrSampler = {};
        api::Sampler linearSampler = {};
        api::Sampler nearestSampler = {};

        // 
        std::array<lancer::ImageMaker, 2u> mSamplesBuffer = {};
        std::array<lancer::ImageMaker, 2u> mDiffuseBuffer = {};
        std::array<lancer::ImageMaker, 2u> mReflectBuffer = {};
        
        lancer::ImageMaker mDenoiseBuffer = {};
        lancer::ImageMaker mColoredBuffer = {};
        lancer::ImageMaker mNormalsBuffer = {};
        lancer::ImageMaker mNormmodBuffer = {};
        lancer::ImageMaker mParametBuffer = {};
        lancer::ImageMaker mDepthStBuffer = {};
        lancer::ImageMaker mOutputsBuffer = {};

        // 
        api::SwapchainKHR swapchain = {};
        std::vector<vkt::Framebuffer> framebuffers = {};

        // Pipelines and Layout
        api::Pipeline finalDrawPipeline = {};
        api::Pipeline reprojectPipeline = {};
        api::Pipeline raytracedPipeline = {};
        api::Pipeline rasterizePipeline = {};


        // Pipeline Makers
        api::Buffer rtSBT = {};
        lancer::GraphicsPipelineMaker mFinalDrawPipeline = {};
        lancer::GraphicsPipelineMaker mReprojectPipeline = {};
        lancer::GraphicsPipelineMaker mRasterizePipeline = {};
        lancer::SBTHelper mRaytracedPipeline = {};

        // Descriptor Set
        //api::DescriptorSet reprojectDescriptorSet = {};
        //api::DescriptorSet rayTracedDescriptorSet = {};
        //api::DescriptorSet rasterizeDescriptorSet = {};
        //api::DescriptorSet finalizedDescriptorSet = {};
        std::array<api::DescriptorSet, 2u> descriptorSetSwap = {};
        std::array<lancer::DescriptorSetMaker, 2u> mDescriptorSetSwap = {};

        // 
        api::PipelineLayout unifiedPipelineLayout = {};
        api::DescriptorSetLayout unifiedDescriptorLayout = {};
        lancer::PipelineLayoutMaker mUnifiedPipelineLayout = {};
        lancer::DescriptorSetLayoutMaker mUnifiedDescriptorLayout = {};

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
