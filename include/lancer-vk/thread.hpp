#pragma once
#include "./config.hpp"
#include "./driver.hpp"

namespace lancer {

    class Thread : public std::enable_shared_from_this<Thread> { public: 
        Thread() {};

    protected: // 
        vk::Queue queue = {};
        vk::CommandPool commandPool = {};
        vk::DescriptorPool descriptorPool = {};
        std::shared_ptr<vkt::GPUFramework> driver = {};
    };

};
