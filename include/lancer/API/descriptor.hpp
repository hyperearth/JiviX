#pragma once

#include "../lib/core.hpp"

namespace lancer {
    
    class DescriptorSet {
        
        protected: 
            //api::DescriptorLayout layout = {};
            //api::DescriptorSet descriptorSet = {};
            api::DescriptorSetAllocateInfo info = {api::VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,nullptr,0u};
            api::DescriptorUpdateTemplate descriptorTemplate = VK_NULL_HANDLE;
            std::vector<uint8_t> descriptorHeap = {}; // sparse descriptor array 
            std::vector<api::DescriptorUpdateTemplateEntry> descriptorEntries = {};

        public:
            DescriptorSet(){
                
            };

            api::DescriptorImageInfo*  AddImageDesc(const uint32_t& dstBinding=0u, const uint32_t& dstArrayElement=0u, const uint32_t& descriptorCount=1u, const bool& uniform = true, const std::vector<Sampler>& samplers = {}) {
                const uintptr_t pt0 = descriptorHeap.size();
                descriptorHeap.resize(pt0,pt0+sizeof(api::DescriptorImageInfo)*descriptorCount);
                descriptorEntries.push_back(api::DescriptorUpdateTemplateEntry{dstBinding,dstArrayElement,descriptorCount,uniform?api::VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER:api::VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,pt0,sizeof(api::DescriptorImageInfo)}); // TODO: SSBO or UBO support
                return (api::DescriptorImageInfo*)(&descriptorHeap[pt0]);
            };
            
            api::DescriptorBufferInfo* AddBufferDesc(const uint32_t& dstBinding=0u, const uint32_t& dstArrayElement=0u, const uint32_t& descriptorCount=1u, const bool& uniform = false) {
                const uintptr_t pt0 = descriptorHeap.size();
                descriptorHeap.resize(pt0,pt0+sizeof(api::DescriptorBufferInfo)*descriptorCount);
                descriptorEntries.push_back(api::DescriptorUpdateTemplateEntry{dstBinding,dstArrayElement,descriptorCount,uniform?api::VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER:api::VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,pt0,sizeof(api::DescriptorBufferInfo)}); // TODO: SSBO or UBO support
                return (api::DescriptorBufferInfo*)(&descriptorHeap[pt0]);
            };

            api::BufferView* AddBufferViewDesc(const uint32_t& dstBinding=0u, const uint32_t& dstArrayElement=0u, const uint32_t& descriptorCount=1u, const bool& uniform = false) {
                const uintptr_t pt0 = descriptorHeap.size();
                descriptorHeap.resize(pt0,pt0+sizeof(api::BufferView)*descriptorCount);
                descriptorEntries.push_back(api::DescriptorUpdateTemplateEntry{dstBinding,dstArrayElement,descriptorCount,uniform?api::VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER:api::VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER,pt0,sizeof(api::BufferView)}); // TODO: SSBO or UBO support
                return (api::BufferView*)(descriptorHeap[pt0]);
            };

            vk::DescriptorSet& Apply(vk::DescriptorSet& descriptorSet, const vk::DescriptorLayout& decl = 0u){
                api::DescriptorUpdateTemplateCreateInfo createInfo{};
                createInfo.templateType = cvk::VK_DESCRIPTOR_UPDATE_TEMPLATE_TYPE_DESCRIPTOR_SET;
                createInfo.flags = 0u;
                createInfo.descriptorUpdateEntryCount = descriptorEntries.size();
                createInfo.pDescriptorUpdateEntries = descriptorEntries.data();
                createInfo.descriptorSetLayout = decl?decl:layout;

                // IGNORE due isn't push descriptor 
                //createInfo.pipelineBindPoint = 0u;
                createInfo.pipelineLayout = 0u;
                createInfo.set = 0u;

                // TODO: Vulkan-HPP format 
                api::CreateDescriptorUpdateTemplate(/*device,*/&createInfo,nullptr,&descriptorTemplate); // TODO: destroy previous template 
                api::UpdateDescriptorSetWithTemplate(/*device,*/descriptorSet,descriptorTemplate,descriptorHeap.data()); // 
                
                // 
                return descriptorSet;
            };
        
    }; 

};
