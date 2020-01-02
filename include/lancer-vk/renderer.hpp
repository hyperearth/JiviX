#pragma once
#include "./config.hpp"
#include "./driver.hpp"

namespace lancer {
    class Renderer : public std::enable_shared_from_this<Renderer> { public: 
        Renderer() {};

    protected: // 
        vk::CommandBuffer deferredCommand = {};
        vk::Pipeline rayTracingStage = {};
        vk::Pipeline resamplingStage = {};
        vk::Pipeline denoiseStage = {};
        vk::Pipeline compileStage = {};

        // binding data
        std::shared_ptr<Material> materials = {}; // materials
        std::shared_ptr<Instance> instances = {}; // instances

        // 
        std::shared_ptr<Context> context = {};
        std::shared_ptr<Driver> driver = {};
    };

};
