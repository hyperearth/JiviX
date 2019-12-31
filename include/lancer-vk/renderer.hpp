#pragma once
#include "./config.hpp"

namespace lancer {

    class Renderer : public std::enable_shared_from_this<Renderer> { public: 
        

    protected: // 
        vk::Pipeline rasterizationStage = {};
        vk::Pipeline rayTracingStage = {};
        vk::Pipeline resamplingStage = {};
        vk::Pipeline denoiseStage = {};
        vk::Pipeline compileStage = {};
    };

};
