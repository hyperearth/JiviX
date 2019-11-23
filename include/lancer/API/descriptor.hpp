#pragma once

#include "../lib/core.hpp"
#include "../API/device.hpp"

namespace lancer {


    class DescriptorSetLayout_T : public std::enable_shared_from_this<DescriptorSetLayout_T> {

        protected: 
            DeviceMaker device = {};
            api::DescriptorSetLayoutCreateInfo info = {};
            api::DescriptorSetLayout *dlayout = nullptr;
            std::vector<api::DescriptorSetLayoutBinding> _bindings = {};
            std::vector<api::DescriptorBindingFlagsEXT> _flags = {};

        public: 
            DescriptorSetLayout_T(const DeviceMaker& device, const api::DescriptorSetLayoutCreateInfo& info = {}, api::DescriptorSetLayout* dlayout = nullptr) : device(device), dlayout(dlayout), info(info) {
            };

            inline DescriptorSetLayoutMaker&& link(api::DescriptorSetLayout& lays) { dlayout = &lays; return shared_from_this(); };
            inline DescriptorSetLayoutMaker&& pushBinding(const api::DescriptorSetLayoutBinding& binding = {}, const api::DescriptorBindingFlagsEXT& flags = {}){
                _bindings.push_back(binding); _flags.push_back(flags);
                return shared_from_this(); };

            // Editable Current State 
            inline api::DescriptorSetLayoutBinding& getBinding() { return _bindings.back(); };
            inline api::DescriptorBindingFlagsEXT& getBindingFlags() { return _flags.back(); };

            // Viewable Current State 
            inline const api::DescriptorSetLayoutBinding& getBinding() const { return _bindings.back(); };
            inline const api::DescriptorBindingFlagsEXT& getBindingFlags() const { return _flags.back(); };
//api::DescriptorSetLayout* dlayout = nullptr
            // 
            inline DescriptorSetLayoutMaker&& create(const api::DescriptorSetLayoutCreateFlagBits& flags = {}) {
                const auto vkfl = api::DescriptorSetLayoutBindingFlagsCreateInfoEXT().setPBindingFlags(_flags.data()).setBindingCount(_flags.size());
                *dlayout = device->least().createDescriptorSetLayout(api::DescriptorSetLayoutCreateInfo().setFlags(flags).setPNext(&vkfl).setPBindings(_bindings.data()).setBindingCount(_bindings.size()));
                return shared_from_this(); };

            // TODO: allocate descriptor sets
            
    };

    class DescriptorSet_T : public std::enable_shared_from_this<DescriptorSet_T> {
        
        protected: 
            DeviceMaker                device = {};
            api::DescriptorSet       *lastdst = nullptr;
            api::DescriptorSetLayout *dlayout = nullptr;
            api::DescriptorSetAllocateInfo info = {};
            api::DescriptorUpdateTemplate descriptorTemplate = nullptr;
            std::vector<uint8_t> descriptorHeap = {}; // sparse descriptor array 
            std::vector<api::DescriptorUpdateTemplateEntry> descriptorEntries = {};

        public:
            DescriptorSet_T(const DeviceMaker& device = {}, api::DescriptorSetAllocateInfo info = {}, api::DescriptorSet* lastdst = nullptr) : lastdst(lastdst),info(info),device(device) {
            };

            // 
            inline api::DescriptorSet& least() { return *lastdst; };
            operator api::DescriptorSet&() { return *lastdst; };
            inline const api::DescriptorSet& Least() const { return *lastdst; };
            operator const api::DescriptorSet&() const { return *lastdst; };

            // 
            inline api::DescriptorImageInfo* addImageDesc(const uint32_t& dstBinding=0u, const uint32_t& dstArrayElement=0u, const uint32_t& descriptorCount=1u, const bool& uniform = true, const std::vector<api::Sampler>& samplers = {}) {
                const uintptr_t pt0 = descriptorHeap.size();
                descriptorHeap.resize(pt0,pt0+sizeof(api::DescriptorImageInfo)*descriptorCount);
                descriptorEntries.push_back(api::DescriptorUpdateTemplateEntry{dstBinding,dstArrayElement,descriptorCount,uniform?(api::DescriptorType::eSampledImage):(api::DescriptorType::eStorageImage),pt0,sizeof(api::DescriptorImageInfo)});
                return (api::DescriptorImageInfo*)(&descriptorHeap[pt0]);
            };

            // 
            inline api::DescriptorBufferInfo* addBufferDesc(const uint32_t& dstBinding=0u, const uint32_t& dstArrayElement=0u, const uint32_t& descriptorCount=1u, const bool& uniform = false) {
                const uintptr_t pt0 = descriptorHeap.size();
                descriptorHeap.resize(pt0,pt0+sizeof(api::DescriptorBufferInfo)*descriptorCount);
                descriptorEntries.push_back(api::DescriptorUpdateTemplateEntry{dstBinding,dstArrayElement,descriptorCount,uniform?(api::DescriptorType::eUniformBuffer):(api::DescriptorType::eStorageBuffer),pt0,sizeof(api::DescriptorBufferInfo)});
                return (api::DescriptorBufferInfo*)(&descriptorHeap[pt0]);
            };

            // 
            inline api::BufferView* addBufferViewDesc(const uint32_t& dstBinding=0u, const uint32_t& dstArrayElement=0u, const uint32_t& descriptorCount=1u, const bool& uniform = false) {
                const uintptr_t pt0 = descriptorHeap.size();
                descriptorHeap.resize(pt0,pt0+sizeof(api::BufferView)*descriptorCount);
                descriptorEntries.push_back(api::DescriptorUpdateTemplateEntry{dstBinding,dstArrayElement,descriptorCount,uniform?api::DescriptorType::eUniformTexelBuffer:api::DescriptorType::eStorageTexelBuffer,pt0,sizeof(api::BufferView)});
                return (api::BufferView*)(&descriptorHeap[pt0]);
            };

            inline DescriptorSetMaker&& create() { // TODO: create descriptor set and layout
                return shared_from_this(); };

            inline DescriptorSetMaker&& link(api::DescriptorSet* desc) { lastdst = desc; return shared_from_this(); };
            inline DescriptorSetMaker&& linkLayout(api::DescriptorSetLayout* lays) { dlayout = lays; return shared_from_this(); };

            // 
            inline DescriptorSetMaker&& apply(){
                api::DescriptorUpdateTemplateCreateInfo createInfo{};
                createInfo.templateType = api::DescriptorUpdateTemplateType::eDescriptorSet;
                createInfo.flags = {};
                createInfo.descriptorUpdateEntryCount = descriptorEntries.size();
                createInfo.pDescriptorUpdateEntries = descriptorEntries.data();
                createInfo.descriptorSetLayout = *dlayout;

                // IGNORE due isn't push descriptor 
                //createInfo.pipelineBindPoint = 0u;
                createInfo.pipelineLayout = nullptr;
                createInfo.set = {};

                // 
                device->least().createDescriptorUpdateTemplate(&createInfo,nullptr,&descriptorTemplate); // TODO: destroy previous template 
                device->least().updateDescriptorSetWithTemplate(*lastdst,descriptorTemplate,descriptorHeap.data()); // 

                // 
                return shared_from_this(); };
    };
};
