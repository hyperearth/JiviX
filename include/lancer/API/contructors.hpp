
#pragma once

#include "../lib/core.hpp"
#include "../API/memory.hpp"
#include <algorithm>
#include <fstream>
#include <iostream>
#include <functional>

namespace lancer {
    
    BufferMaker CreateBufferMaker(const DeviceMaker& device, const api::BufferCreateInfo& bfc = api::BufferCreateInfo().setSharingMode(api::SharingMode::eExclusive), api::Buffer* lastbuf = nullptr){
        return std::make_shared<Buffer_T>(device,bfc,lastbuf);
    };

    ImageMaker CreateBufferMaker(const DeviceMaker& device, const api::ImageCreateInfo& bfc = api::ImageCreateInfo().setSharingMode(api::SharingMode::eExclusive), api::Image* lastbuf = nullptr){
        return std::make_shared<Image_T>(device,bfc,lastbuf);
    };
    
};
