#pragma once // #

#include "./config.hpp"

namespace jvi {

    //class Driver : public std::enable_shared_from_this<Driver> { public: 
    //    Driver() {};

    //protected: // Driver Pack For Renderer
    //    vk::PhysicalDevice gpu = {};
    //    vk::Device device = {};
    //};

    using Driver = vkt::GPUFramework;

};
