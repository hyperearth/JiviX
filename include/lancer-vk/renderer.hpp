#pragma once
#include "./config.hpp"
#include "./driver.hpp"
#include "./thread.hpp"
#include "./context.hpp"

namespace lancer {
    class Renderer : public std::enable_shared_from_this<Renderer> { public: 
        Renderer(const std::shared_ptr<Driver>& driver) {
            this->driver = driver;
            this->thread = std::make_shared<Thread>(this->driver);
            this->context = std::make_shared<Context>();
        };

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
        std::shared_ptr<Thread> thread = {};
    };

};
