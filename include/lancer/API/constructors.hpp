
#pragma once

#include "../lib/core.hpp"
#include "../API/device.hpp"
#include "../API/buffer.hpp"
#include "../API/image.hpp"
#include "../API/descriptor.hpp"
#include "../API/gpipeline.hpp"
#include "../API/piplayout.hpp"
#include "../API/renderpass.hpp"

#ifdef EXTENSION_VMA
#include "../EXT/VMA.hpp"
#endif

#ifdef EXTENSION_RTX
#include "../EXT/RTX.hpp"
#endif

#include <algorithm>
#include <fstream>
#include <iostream>
#include <functional>

// 
namespace lancer {
    
    inline BufferMaker Device_T::createBufferMaker(const api::BufferCreateInfo& info, api::Buffer* lastbuf){
        return std::make_shared<Buffer_T>(shared_from_this(),info,lastbuf);
    };

    inline ImageMaker Device_T::createImageMaker(const api::ImageCreateInfo& info, api::Image* lastimg){
        return std::make_shared<Image_T>(shared_from_this(),info,lastimg);
    };

    inline SamplerMaker Device_T::createSamplerMaker(const api::SamplerCreateInfo& sfc, api::Sampler* lastsamp){
        return std::make_shared<Sampler_T>(shared_from_this(),sfc,lastsamp);
    };

    inline GraphicsPipelineMaker Device_T::createGraphicsPipelineMaker(const api::GraphicsPipelineCreateInfo& info, api::Pipeline* pipeline, const uint32_t& width, const uint32_t& height){
        return std::make_shared<GraphicsPipeline_T>(shared_from_this(),info,pipeline,width,height);
    };

    inline DescriptorSetLayoutMaker Device_T::createDescriptorSetLayoutMaker(const api::DescriptorSetLayoutCreateInfo& info, api::DescriptorSetLayout* desclay){
        return std::make_shared<DescriptorSetLayout_T>(shared_from_this(),info,desclay);
    };

    inline DescriptorSetMaker Device_T::createDescriptorSet(const api::DescriptorSetAllocateInfo& info, api::DescriptorSet* descset){
        return std::make_shared<DescriptorSet_T>(shared_from_this(),info,descset);
    };

    inline RenderPassMaker Device_T::createRenderPassMaker(const api::RenderPassCreateInfo& info, api::RenderPass* renderpass){
        return std::make_shared<RenderPass_T>(shared_from_this(),info,renderpass);
    };

    inline PipelineLayoutMaker Device_T::createPipelineLayoutMaker(const api::PipelineLayoutCreateInfo& info, api::PipelineLayout* playout) {
        return std::make_shared<PipelineLayout_T>(shared_from_this(), info, playout);
    };

    inline DeviceMaker PhysicalDevice_T::createDeviceMaker(const api::DeviceCreateInfo& info, api::Device* device) {
        return std::make_shared<Device_T>(shared_from_this(), info, device);
    };

    // SHOULD BE IN HEADER
    template<class T> inline MemoryAllocator& Device_T::createAllocator(const uintptr_t& info) {
        this->allocator = std::dynamic_pointer_cast<MemoryAllocator_T>(std::make_shared<T>(shared_from_this()));
        this->allocator->linkDevice(shared_from_this())->initialize(info);
        return this->allocator;
    };

#ifdef EXTENSION_RTX_IMPLEMENT
    inline SBTHelper Device_T::createSBTHelper(api::Pipeline* rtPipeline) {
        return std::make_shared<SBTHelper_T>(shared_from_this(),rtPipeline);
    };
#endif

#ifdef EXTENSION_VMA_IMPLEMENT
    
#endif
    
};
