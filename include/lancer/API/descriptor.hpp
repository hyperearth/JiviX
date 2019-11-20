#pragma once

#include "../lib/core.hpp"
#include "../API/device.hpp"

namespace lancer {


    class DescriptorSetLayout : public std::enable_shared_from_this<DescriptorSetLayout> {

        protected: 
            std::shared_ptr<Device>    device = {};
            api::DescriptorSetLayout *dlayout = nullptr;
            std::vector<api::DescriptorSetLayoutBinding> _bindings = {};
            std::vector<api::DescriptorBindingFlagsEXT> _flags = {};

        public: 
            DescriptorSetLayout(const std::shared_ptr<Device>& device, api::DescriptorSetLayout* dlayout = nullptr) : device(device), dlayout(dlayout) {
            };

            std::shared_ptr<DescriptorSetLayout>&& Link(api::DescriptorSetLayout& lays) { 
                dlayout = &lays; 
                return shared_from_this(); };

            std::shared_ptr<DescriptorSetLayout>&& PushBinding(const api::DescriptorSetLayoutBinding& binding = {}, const api::DescriptorBindingFlagsEXT& flags = {}){
                _bindings.push_back(binding); _flags.push_back(flags);
                return shared_from_this(); };

            // Editable Current State 
                  api::DescriptorSetLayoutBinding& GetBinding() { return _bindings.back(); };
                  api::DescriptorBindingFlagsEXT& GetFlags() { return _flags.back(); };

            // Viewable Current State 
            const api::DescriptorSetLayoutBinding& GetBinding() const { return _bindings.back(); };
            const api::DescriptorBindingFlagsEXT& GetFlags() const { return _flags.back(); };

            // 
            std::shared_ptr<DescriptorSetLayout>&& Create(const api::DescriptorSetLayoutCreateFlagBits& flags = {}) {
                const auto vkfl = api::DescriptorSetLayoutBindingFlagsCreateInfoEXT().setPBindingFlags(_flags.data()).setBindingCount(_flags.size());
                *dlayout = device->Least().createDescriptorSetLayout(api::DescriptorSetLayoutCreateInfo().setFlags(flags).setPNext(&vkfl).setPBindings(_bindings.data()).setBindingCount(_bindings.size()));
                return shared_from_this(); };
            
    };

    class DescriptorSet : public std::enable_shared_from_this<DescriptorSet> {
        
        protected: 
            std::shared_ptr<Device>    device = {};
            api::DescriptorSet       *lastdst = nullptr;
            api::DescriptorSetLayout *dlayout = nullptr;
            api::DescriptorSetAllocateInfo info = {};
            api::DescriptorUpdateTemplate descriptorTemplate = nullptr;
            std::vector<uint8_t> descriptorHeap = {}; // sparse descriptor array 
            std::vector<api::DescriptorUpdateTemplateEntry> descriptorEntries = {};

        public:
            DescriptorSet(const std::shared_ptr<Device>& device, api::DescriptorSet* lastdst = nullptr, api::DescriptorSetAllocateInfo info = {}) : lastdst(lastdst),info(info),device(device) {
            };

            // 
            api::DescriptorSet& Least() { return *lastdst; };
            operator api::DescriptorSet&() { return *lastdst; };
            const api::DescriptorSet& Least() const { return *lastdst; };
            operator const api::DescriptorSet&() const { return *lastdst; };

            // 
            api::DescriptorImageInfo* AddImageDesc(const uint32_t& dstBinding=0u, const uint32_t& dstArrayElement=0u, const uint32_t& descriptorCount=1u, const bool& uniform = true, const std::vector<api::Sampler>& samplers = {}) {
                const uintptr_t pt0 = descriptorHeap.size();
                descriptorHeap.resize(pt0,pt0+sizeof(api::DescriptorImageInfo)*descriptorCount);
                descriptorEntries.push_back(api::DescriptorUpdateTemplateEntry{dstBinding,dstArrayElement,descriptorCount,uniform?(api::DescriptorType::eSampledImage):(api::DescriptorType::eStorageImage),pt0,sizeof(api::DescriptorImageInfo)}); // TODO: SSBO or UBO support
                return (api::DescriptorImageInfo*)(&descriptorHeap[pt0]);
            };

            // 
            api::DescriptorBufferInfo* AddBufferDesc(const uint32_t& dstBinding=0u, const uint32_t& dstArrayElement=0u, const uint32_t& descriptorCount=1u, const bool& uniform = false) {
                const uintptr_t pt0 = descriptorHeap.size();
                descriptorHeap.resize(pt0,pt0+sizeof(api::DescriptorBufferInfo)*descriptorCount);
                descriptorEntries.push_back(api::DescriptorUpdateTemplateEntry{dstBinding,dstArrayElement,descriptorCount,uniform?(api::DescriptorType::eUniformBuffer):(api::DescriptorType::eStorageBuffer),pt0,sizeof(api::DescriptorBufferInfo)}); // TODO: SSBO or UBO support
                return (api::DescriptorBufferInfo*)(&descriptorHeap[pt0]);
            };

            // 
            api::BufferView* AddBufferViewDesc(const uint32_t& dstBinding=0u, const uint32_t& dstArrayElement=0u, const uint32_t& descriptorCount=1u, const bool& uniform = false) {
                const uintptr_t pt0 = descriptorHeap.size();
                descriptorHeap.resize(pt0,pt0+sizeof(api::BufferView)*descriptorCount);
                descriptorEntries.push_back(api::DescriptorUpdateTemplateEntry{dstBinding,dstArrayElement,descriptorCount,uniform?api::DescriptorType::eUniformTexelBuffer:api::DescriptorType::eStorageTexelBuffer,pt0,sizeof(api::BufferView)}); // TODO: SSBO or UBO support
                return (api::BufferView*)(&descriptorHeap[pt0]);
            };

            std::shared_ptr<DescriptorSet>&& Create() { // TODO: create descriptor set and layout
                return shared_from_this(); };

            std::shared_ptr<DescriptorSet>&& Link(api::DescriptorSet& desc) { 
                lastdst = &desc; 
                return shared_from_this(); };

            std::shared_ptr<DescriptorSet>&& LinkLayout(api::DescriptorSetLayout& lays) { 
                dlayout = &lays; 
                return shared_from_this(); };

            // 
            std::shared_ptr<DescriptorSet>&& Apply(){
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
                device->Least().createDescriptorUpdateTemplate(&createInfo,nullptr,&descriptorTemplate); // TODO: destroy previous template 
                device->Least().updateDescriptorSetWithTemplate(*lastdst,descriptorTemplate,descriptorHeap.data()); // 

                // 
                return shared_from_this(); };
    };
};
