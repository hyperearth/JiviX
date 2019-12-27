#pragma once

#include <memory>
#include <vector>
#include "./structures.hpp"

namespace vkh {
    
    // TODO: Some Vookoo-like aggregators and helpers
    // 1. Ray Tracing Pipeline and SBT helper
    // 2. Descriptor Set Layout aggregator
    // 3. Render Pass helper
    // 4. Device and driver aggregate
    // 5. Descriptor Set aggregate and helper
    // 6. Buffer and VMA based vectors

    class VsRayTracingPipelineCreateInfoHelper { protected: 
        VkRayTracingPipelineCreateInfoNV vk_info = {};
        VkRayTracingShaderGroupCreateInfoNV raygen_shader_group = {};
        std::vector<VkPipelineShaderStageCreateInfo> stages = {};
        std::vector<VkRayTracingShaderGroupCreateInfoNV> miss_shader_groups = {};
        std::vector<VkRayTracingShaderGroupCreateInfoNV> hit_shader_groups = {};
        std::vector<VkRayTracingShaderGroupCreateInfoNV> compiled_shader_groups = {};
        
    public: // 

        // get offsets of shader groups
        uintptr_t raygenOffsetIndex() { return 0u; };
        uintptr_t missOffsetIndex() {return 1u; };
        uintptr_t hitOffsetIndex() { return miss_shader_groups.size()+missOffsetIndex(); };

        // 
        VsRayTracingPipelineCreateInfoHelper(const VkRayTracingPipelineCreateInfoNV& info = {}){
            vk_info = info;
        };

        // result groups
        std::vector<VkRayTracingShaderGroupCreateInfoNV>& compileGroups() {
            compiled_shader_groups = { raygen_shader_group };
            for (auto& group : miss_shader_groups) { compiled_shader_groups.push_back(group); };
            for (auto& group : hit_shader_groups) { compiled_shader_groups.push_back(group); };
            return compiled_shader_groups;
        };

        // WARNING: Only One Hit Group Supported At Once
        VsRayTracingPipelineCreateInfoHelper& addShaderStages(const std::vector<VkPipelineShaderStageCreateInfo>& stages_in = {}, const VkRayTracingShaderGroupTypeNV& prior_group_type = VK_RAY_TRACING_SHADER_GROUP_TYPE_TRIANGLES_HIT_GROUP_NV) {
            for (auto& stage : stages_in) {
                if (stage.stage == VK_SHADER_STAGE_RAYGEN_BIT_NV) {
                    const uintptr_t last_idx = stages.size(); stages.push_back(stage);
                    raygen_shader_group.generalShader = last_idx;
                };
            };

            uintptr_t group_idx = -1U;
            for (auto& stage : stages_in) {
                if (stage.stage == VK_SHADER_STAGE_MISS_BIT_NV) {
                    const uintptr_t last_idx = stages.size(); stages.push_back(stage);
                    group_idx = miss_shader_groups.size(); miss_shader_groups.push_back({});
                    //if (group_idx == -1U) { group_idx = miss_shader_groups.size(); miss_shader_groups.push_back({}); };
                    miss_shader_groups[group_idx].generalShader = last_idx;//break;
                };
            };

            group_idx = -1U; // Only One Hit Group Supported At Once
            for (auto& stage : stages_in) {
                if (stage.stage == VK_SHADER_STAGE_CLOSEST_HIT_BIT_NV || stage.stage == VK_SHADER_STAGE_ANY_HIT_BIT_NV || stage.stage == VK_SHADER_STAGE_INTERSECTION_BIT_NV) {
                    const uintptr_t last_idx = stages.size(); stages.push_back(stage);
                    if (group_idx == -1U) { group_idx = hit_shader_groups.size(); hit_shader_groups.push_back({}); };
                    if (stage.stage == VK_SHADER_STAGE_CLOSEST_HIT_BIT_NV) {
                        hit_shader_groups[group_idx].type = prior_group_type;
                        hit_shader_groups[group_idx].closestHitShader = last_idx;
                    };
                    if (stage.stage == VK_SHADER_STAGE_ANY_HIT_BIT_NV) {
                        hit_shader_groups[group_idx].type = prior_group_type;
                        hit_shader_groups[group_idx].anyHitShader = last_idx;
                    };
                    if (stage.stage == VK_SHADER_STAGE_INTERSECTION_BIT_NV) {
                        hit_shader_groups[group_idx].type = VK_RAY_TRACING_SHADER_GROUP_TYPE_PROCEDURAL_HIT_GROUP_NV, 
                        hit_shader_groups[group_idx].intersectionShader = last_idx;
                    };
                };
            };

            return *this;
        };

        operator VkRayTracingPipelineCreateInfoNV&() {
            auto& groups = compileGroups();
            vk_info.pGroups = groups.data();
            vk_info.groupCount = groups.size();
            vk_info.pStages = stages.data();
            vk_info.stageCount = stages.size();
            return vk_info;
        };
    };

    struct VsDescriptorHandle { using T = uint8_t;
        VkDescriptorUpdateTemplateEntry* entry_t = nullptr;
        void* field_t = nullptr;

        // any buffers and images can `write` into types
        template<class T = uint8_t> inline operator T&() { return (*field_t); };
        template<class T = uint8_t> inline operator const T&() const { return (*field_t); };
        template<class T = uint8_t> inline T& offset(const uint32_t& idx = 0u) { return description_handle{entry_t,(T*)field_t+idx}; };
        template<class T = uint8_t> inline const T& offset(const uint32_t& idx = 0u) const { return description_handle{entry_t,(T*)field_t+idx}; };
        inline const uint32_t& size() const { return entry_t->descriptorCount; }; 

        template<class T = uint8_t>
        inline VsDescriptorHandle& operator=(const T& d) { *(T*)field_t = d; return *this; };
    };
    
    class VsDescriptorSetCreateInfoHelper { public: uint32_t flags = 0u; using T = uintptr_t;
        template<class T = T>
        inline VsDescriptorHandle& _push_description( const VkDescriptorUpdateTemplateEntry& entry_ ) { // Un-Safe API again
            const uintptr_t pt0 = heap_.size();
            heap_.resize(pt0+sizeof(T)*entry_.descriptorCount, 0u);
            entries_.push_back(entry_);
            entries_.back().offset = pt0;
            entries_.back().stride = sizeof(T);
            handles_.push_back({ &entries_.back(), &heap_.back() }); 
            return handles_.back();
        };

        // official function (not template)
        VsDescriptorHandle& pushDescription( const VkDescriptorUpdateTemplateEntry& entry = {} ) {
            if (entry.descriptorType == VK_DESCRIPTOR_TYPE_STORAGE_BUFFER || entry.descriptorType == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER) {
                return _push_description<VkDescriptorBufferInfo>(entry);
            } else 
            if (entry.descriptorType == VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER || entry.descriptorType == VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER) {
                return _push_description<VkBufferView>(entry);
            } else 
            if (entry.descriptorType == VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_NV) {
                return _push_description<VkAccelerationStructureNV>(entry);
            } else {
                return _push_description<VkDescriptorImageInfo>(entry);
            };
        };

        // 
        VsDescriptorSetCreateInfoHelper(const VkDescriptorSetLayout& layout = {}, const VkDescriptorPool& pool = {}){
            templ_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_UPDATE_TEMPLATE_CREATE_INFO;
            templ_info.pNext = nullptr;
            templ_info.flags = {};
            templ_info.descriptorUpdateEntryCount = 0u;
            templ_info.pDescriptorUpdateEntries = nullptr;
            templ_info.descriptorSetLayout = layout;
            templ_info.pipelineLayout = VK_NULL_HANDLE;
            templ_info.pipelineBindPoint = VK_PIPELINE_BIND_POINT_COMPUTE;
            templ_info.templateType = VK_DESCRIPTOR_UPDATE_TEMPLATE_TYPE_DESCRIPTOR_SET;
            alloc_info.descriptorPool = pool;
            alloc_info.pSetLayouts = &templ_info.descriptorSetLayout;
            alloc_info.descriptorSetCount = 1u;
        };

        // 
        operator const std::vector<VkDescriptorUpdateTemplateEntry>&() const { return entries; };
        operator std::vector<VkDescriptorUpdateTemplateEntry>&() { return entries; };

        // 
        operator VkDescriptorSetAllocateInfo&() { return alloc_info; };
        operator VkDescriptorUpdateTemplateCreateInfo&() {
            templ_info.pDescriptorUpdateEntries = entries.data();
            templ_info.descriptorUpdateEntryCount = entries.size();
            return templ_info;
        };

        // 
        operator const VkDescriptorSetAllocateInfo&() const { return alloc_info; };
        operator const VkDescriptorUpdateTemplateCreateInfo&() const { return templ_info; };

        // 
        operator std::vector<VkDescriptorUpdateTemplateEntry>&() { return entries; };
        operator const std::vector<VkDescriptorUpdateTemplateEntry>&() const { return entries; };

    protected: // 
        std::vector<uint8_t> heap = {};
        std::vector<VkDescriptorUpdateTemplateEntry> entries = {};
        std::vector<VsDescriptorHandle> handles = {};

        VkDescriptorSetAllocateInfo alloc_info = {};
        VkDescriptorUpdateTemplateCreateInfo templ_info = {};
    };

};
