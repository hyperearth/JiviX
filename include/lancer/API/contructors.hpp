
#pragma once

#include "../lib/core.hpp"
#include "../API/device.hpp"
#include <algorithm>
#include <fstream>
#include <iostream>
#include <functional>

namespace lancer {
    
    inline BufferMaker Device_T::createBufferMaker(const api::BufferCreateInfo& bfc, api::Buffer* lastbuf){
        return std::make_shared<Buffer_T>(shared_from_this(),bfc,lastbuf);
    };

    inline ImageMaker Device_T::createImageMaker(const api::ImageCreateInfo& bfc, api::Image* lastbuf){
        return std::make_shared<Image_T>(shared_from_this(),bfc,lastbuf);
    };
    
    inline GraphicsPipelineMaker Device_T::createGraphicsPipeline(const api::GraphicsPipelineCreateInfo& bfc, api::Pipeline* pipeline, const uint32_t& width = 1u, const uint32_t& height = 1u){
        return std::make_shared<GraphicsPipeline_T>(shared_from_this(),bfc,pipeline,width,height);
    };

    inline DescriptorSetLayoutMaker Device_T::createDescriptorSetLayoutMaker(const api::DescriptorSetLayoutCreateInfo& bfc, api::DescriptorSetLayout* pipeline){
        return std::make_shared<DescriptorSetLayout_T>(shared_from_this(),bfc,pipeline);
    };

    inline DescriptorSetMaker Device_T::createDescriptorSet(const api::DescriptorSetAllocateInfo& bfc, api::DescriptorSet* pipeline){
        return std::make_shared<DescriptorSet_T>(shared_from_this(),bfc,pipeline);
    };

    // SHOULD BE IN HEADER
    template<class T = MemoryAllocator_T> inline std::shared_ptr<T> Device_T::createAllocator(const uintptr_t& info) {
        return (this->allocator = std::make_shared<T>(shared_from_this(),info));
    };

#ifdef VMA_ALLOCATOR
    
#endif
    
};
