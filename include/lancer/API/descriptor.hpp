#pragma once

#include "../lib/core.hpp"

namespace lancer {
    
    class DescriptorSet {
        
        protected: 
            //api::DescriptorLayout layout = {};
            //api::DescriptorSet descriptorSet = {};
            // TODO: Device or DescriptorSetAllocator objects 
            api::DescriptorSetAllocateInfo info = {};
            api::DescriptorUpdateTemplate descriptorTemplate = nullptr;
            api::DescriptorSetLayout layout = nullptr;
            std::vector<uint8_t> descriptorHeap = {}; // sparse descriptor array 
            std::vector<api::DescriptorUpdateTemplateEntry> descriptorEntries = {};
            api::Device device = {}; // TODO: Device Stub Anti-Pattern


        public:
            DescriptorSet(){
                
            };

            api::DescriptorImageInfo*  AddImageDesc(const uint32_t& dstBinding=0u, const uint32_t& dstArrayElement=0u, const uint32_t& descriptorCount=1u, const bool& uniform = true, const std::vector<api::Sampler>& samplers = {}) {
                const uintptr_t pt0 = descriptorHeap.size();
                descriptorHeap.resize(pt0,pt0+sizeof(api::DescriptorImageInfo)*descriptorCount);
                descriptorEntries.push_back(api::DescriptorUpdateTemplateEntry{dstBinding,dstArrayElement,descriptorCount,uniform?(api::DescriptorType::eSampledImage):(api::DescriptorType::eStorageImage),pt0,sizeof(api::DescriptorImageInfo)}); // TODO: SSBO or UBO support
                return (api::DescriptorImageInfo*)(&descriptorHeap[pt0]);
            };
            
            api::DescriptorBufferInfo* AddBufferDesc(const uint32_t& dstBinding=0u, const uint32_t& dstArrayElement=0u, const uint32_t& descriptorCount=1u, const bool& uniform = false) {
                const uintptr_t pt0 = descriptorHeap.size();
                descriptorHeap.resize(pt0,pt0+sizeof(api::DescriptorBufferInfo)*descriptorCount);
                descriptorEntries.push_back(api::DescriptorUpdateTemplateEntry{dstBinding,dstArrayElement,descriptorCount,uniform?(api::DescriptorType::eUniformBuffer):(api::DescriptorType::eStorageBuffer),pt0,sizeof(api::DescriptorBufferInfo)}); // TODO: SSBO or UBO support
                return (api::DescriptorBufferInfo*)(&descriptorHeap[pt0]);
            };

            api::BufferView* AddBufferViewDesc(const uint32_t& dstBinding=0u, const uint32_t& dstArrayElement=0u, const uint32_t& descriptorCount=1u, const bool& uniform = false) {
                const uintptr_t pt0 = descriptorHeap.size();
                descriptorHeap.resize(pt0,pt0+sizeof(api::BufferView)*descriptorCount);
                descriptorEntries.push_back(api::DescriptorUpdateTemplateEntry{dstBinding,dstArrayElement,descriptorCount,uniform?api::DescriptorType::eUniformTexelBuffer:api::DescriptorType::eStorageTexelBuffer,pt0,sizeof(api::BufferView)}); // TODO: SSBO or UBO support
                return (api::BufferView*)(descriptorHeap[pt0]);
            };

            vk::DescriptorSet& Apply(api::DescriptorSet& descriptorSet, const api::DescriptorSetLayout& decl = {}){
                api::DescriptorUpdateTemplateCreateInfo createInfo{};
                createInfo.templateType = api::DescriptorUpdateTemplateType::eDescriptorSet;
                createInfo.flags = {};
                createInfo.descriptorUpdateEntryCount = descriptorEntries.size();
                createInfo.pDescriptorUpdateEntries = descriptorEntries.data();
                createInfo.descriptorSetLayout = decl?decl:layout;

                // IGNORE due isn't push descriptor 
                //createInfo.pipelineBindPoint = 0u;
                createInfo.pipelineLayout = nullptr;
                createInfo.set = {};

                // TODO: Vulkan-HPP format 
                device.createDescriptorUpdateTemplate(&createInfo,nullptr,&descriptorTemplate); // TODO: destroy previous template 
                device.updateDescriptorSetWithTemplate(descriptorSet,descriptorTemplate,descriptorHeap.data()); // 
                
                // 
                return descriptorSet;
            };
        
    }; 

};
