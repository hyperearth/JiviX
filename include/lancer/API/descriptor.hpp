#pragma once

#include "../lib/core.hpp"
#include "../lib/device.hpp"

namespace lancer {
    
    class DescriptorSet {
        
        protected: 
            std::shared_ptr<Device> device = {};
            api::DescriptorSet* lastdst = {};
            api::DescriptorSetAllocateInfo info = {};
            api::DescriptorSetLayout layout = nullptr;
            api::DescriptorUpdateTemplate descriptorTemplate = nullptr;
            std::vector<uint8_t> descriptorHeap = {}; // sparse descriptor array 
            std::vector<api::DescriptorUpdateTemplateEntry> descriptorEntries = {};

        public:
            DescriptorSet(){
            };

            vk::DescriptorSet& Least() { return *lastdst; };
            operator vk::DescriptorSet&() { return *lastdst; };
            const vk::DescriptorSet& Least() const { return *lastdst; };
            operator const vk::DescriptorSet&() const { return *lastdst; };

            api::DescriptorImageInfo& AddImageDesc(const uint32_t& dstBinding=0u, const uint32_t& dstArrayElement=0u, const uint32_t& descriptorCount=1u, const bool& uniform = true, const std::vector<api::Sampler>& samplers = {}) {
                const uintptr_t pt0 = descriptorHeap.size();
                descriptorHeap.resize(pt0,pt0+sizeof(api::DescriptorImageInfo)*descriptorCount);
                descriptorEntries.push_back(api::DescriptorUpdateTemplateEntry{dstBinding,dstArrayElement,descriptorCount,uniform?(api::DescriptorType::eSampledImage):(api::DescriptorType::eStorageImage),pt0,sizeof(api::DescriptorImageInfo)}); // TODO: SSBO or UBO support
                return *(api::DescriptorImageInfo*)(&descriptorHeap[pt0]);
            };

            api::DescriptorBufferInfo& AddBufferDesc(const uint32_t& dstBinding=0u, const uint32_t& dstArrayElement=0u, const uint32_t& descriptorCount=1u, const bool& uniform = false) {
                const uintptr_t pt0 = descriptorHeap.size();
                descriptorHeap.resize(pt0,pt0+sizeof(api::DescriptorBufferInfo)*descriptorCount);
                descriptorEntries.push_back(api::DescriptorUpdateTemplateEntry{dstBinding,dstArrayElement,descriptorCount,uniform?(api::DescriptorType::eUniformBuffer):(api::DescriptorType::eStorageBuffer),pt0,sizeof(api::DescriptorBufferInfo)}); // TODO: SSBO or UBO support
                return *(api::DescriptorBufferInfo*)(&descriptorHeap[pt0]);
            };

            api::BufferView& AddBufferViewDesc(const uint32_t& dstBinding=0u, const uint32_t& dstArrayElement=0u, const uint32_t& descriptorCount=1u, const bool& uniform = false) {
                const uintptr_t pt0 = descriptorHeap.size();
                descriptorHeap.resize(pt0,pt0+sizeof(api::BufferView)*descriptorCount);
                descriptorEntries.push_back(api::DescriptorUpdateTemplateEntry{dstBinding,dstArrayElement,descriptorCount,uniform?api::DescriptorType::eUniformTexelBuffer:api::DescriptorType::eStorageTexelBuffer,pt0,sizeof(api::BufferView)}); // TODO: SSBO or UBO support
                return *(api::BufferView*)(&descriptorHeap[pt0]);
            };

            std::shared_ptr<DescriptorSet>& LinkDescriptorSet(api::DescriptorSet& desc) { 
                lastdst = &desc; return shared_from_this();
            };
            
            std::shared_ptr<DescriptorSet>& LinkDescriptorSetLayout(api::DescriptorSetLayout& lays) { 
                layout = &lays; return shared_from_this(); 
            };

            std::shared_ptr<DescriptorSet>& Apply(){
                api::DescriptorUpdateTemplateCreateInfo createInfo{};
                createInfo.templateType = api::DescriptorUpdateTemplateType::eDescriptorSet;
                createInfo.flags = {};
                createInfo.descriptorUpdateEntryCount = descriptorEntries.size();
                createInfo.pDescriptorUpdateEntries = descriptorEntries.data();
                createInfo.descriptorSetLayout = layout;

                // IGNORE due isn't push descriptor 
                //createInfo.pipelineBindPoint = 0u;
                createInfo.pipelineLayout = nullptr;
                createInfo.set = {};

                // 
                device->Least().createDescriptorUpdateTemplate(&createInfo,nullptr,&descriptorTemplate); // TODO: destroy previous template 
                device->Least().updateDescriptorSetWithTemplate(dsmc?dsmc:lastdst,descriptorTemplate,descriptorHeap.data()); // 
                if (dsmc) dsmc = lastdst; // Apply already got descriptorSet

                return shared_from_this(); 
            };
        
    }; 

};
