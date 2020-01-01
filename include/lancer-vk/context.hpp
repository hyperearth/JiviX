#pragma once
#include "./config.hpp"
#include "./driver.hpp"

namespace lancer {

    // TODO: Full Context Support
    class Context : public std::enable_shared_from_this<Context> { public: friend Mesh; friend Instance; friend Driver;
        Context(){};

        // 
        vk::Rect2D& refScissor() { return scissor; };
        vk::Viewport& refViewport() { return viewport; };
        vk::RenderPass& refRenderPass() { return renderPass; };
        vk::Framebuffer& refFramebuffer() { return framebuffer; };

        // 
        const vk::Rect2D& refScissor() const { return scissor; };
        const vk::Viewport& refViewport() const { return viewport; };
        const vk::RenderPass& refRenderPass() const { return renderPass; };
        const vk::Framebuffer& refFramebuffer() const { return framebuffer; };


    protected: // 
        vk::Rect2D scissor = {};
        vk::Viewport viewport = {};
        vk::RenderPass renderPass = {};
        vk::Framebuffer framebuffer = {};

        // 
        vk::PipelineLayout unifiedPipelineLayout = {};
        vk::DescriptorSetLayout materialDescriptorSetLayout = {};
        vk::DescriptorSetLayout rayTraceDescriptorSetLayout = {};
        vk::DescriptorSetLayout meshDataDescriptorSetLayout = {};
        vk::DescriptorSetLayout samplingDescriptorSetLayout = {};

        // 
        std::shared_ptr<Driver> driver = {};
        std::shared_ptr<Thread> thread = {};
    };

};
