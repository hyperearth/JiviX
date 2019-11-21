#pragma once

#include "../lib/core.hpp"
#include "../API/memory.hpp"
#include <algorithm>
#include <fstream>
#include <iostream>
#include <functional>

namespace lancer {

#ifndef VKR
#define VKR
#endif

#ifndef VKR_IMPLEMENTATION
#define VKR_IMPLEMENTATION
#endif


#ifdef VKR

    // TODO: unify and decapitalize methods 

    class SBTHelper {
    public:
        SBTHelper();
        ~SBTHelper() = default;

        void        Initialize(const uint32_t numHitGroups, const uint32_t numMissGroups, const uint32_t shaderHeaderSize);
        void        Destroy();
        void        SetRaygenStage(const VkPipelineShaderStageCreateInfo& stage);
        void        AddStageToHitGroup(const Array<VkPipelineShaderStageCreateInfo>& stages, const uint32_t groupIndex);
        void        AddStageToMissGroup(const VkPipelineShaderStageCreateInfo& stage, const uint32_t groupIndex);

        uint32_t    GetGroupsStride() const;
        uint32_t    GetNumGroups() const;
        uint32_t    GetRaygenOffset() const;
        uint32_t    GetHitGroupsOffset() const;
        uint32_t    GetMissGroupsOffset() const;

        uint32_t                                   GetNumStages() const;
        const VkPipelineShaderStageCreateInfo*     GetStages() const;
        const VkRayTracingShaderGroupCreateInfoNV* GetGroups() const;

        uint32_t    GetSBTSize() const;
        bool        CreateSBT(VkDevice device, VkPipeline rtPipeline);
        VkBuffer    GetSBTBuffer() const;

    private:
        uint32_t                                   mShaderHeaderSize;
        uint32_t                                   mNumHitGroups;
        uint32_t                                   mNumMissGroups;
        Array<uint32_t>                            mNumHitShaders;
        Array<uint32_t>                            mNumMissShaders;
        Array<VkPipelineShaderStageCreateInfo>     mStages;
        Array<VkRayTracingShaderGroupCreateInfoNV> mGroups;
        Buffer                                     mSBT;
    };
#endif

/* // TODO: Re-Implement SBT Helper 
#ifdef VKR_IMPLEMENTATION
    // SBT Helper class
    SBTHelper::SBTHelper()
        : mShaderHeaderSize(0u)
        , mNumHitGroups(0u)
        , mNumMissGroups(0u) {
    }

    void SBTHelper::Initialize(const uint32_t numHitGroups, const uint32_t numMissGroups, const uint32_t shaderHeaderSize) {
        mShaderHeaderSize = shaderHeaderSize;
        mNumHitGroups = numHitGroups;
        mNumMissGroups = numMissGroups;

        mNumHitShaders.resize(numHitGroups, 0u);
        mNumMissShaders.resize(numMissGroups, 0u);

        mStages.clear();
        mGroups.clear();
    }

    void SBTHelper::Destroy() {
        mNumHitShaders.clear();
        mNumMissShaders.clear();
        mStages.clear();
        mGroups.clear();

        mSBT.Destroy();
    }

    void SBTHelper::SetRaygenStage(const VkPipelineShaderStageCreateInfo& stage) {
        // this shader stage should go first!
        assert(mStages.empty());
        mStages.push_back(stage);

        VkRayTracingShaderGroupCreateInfoNV groupInfo;
        groupInfo.sType = VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_NV;
        groupInfo.pNext = nullptr;
        groupInfo.type = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_NV;
        groupInfo.generalShader = 0;
        groupInfo.closestHitShader = VK_SHADER_UNUSED_NV;
        groupInfo.anyHitShader = VK_SHADER_UNUSED_NV;
        groupInfo.intersectionShader = VK_SHADER_UNUSED_NV;
        mGroups.push_back(groupInfo); // group 0 is always for raygen
    }

    void SBTHelper::AddStageToHitGroup(const Array<VkPipelineShaderStageCreateInfo>& stages, const uint32_t groupIndex) {
        // raygen stage should go first!
        assert(!mStages.empty());

        assert(groupIndex < mNumHitShaders.size());
        assert(!stages.empty() && stages.size() <= 3);// only 3 hit shaders per group (intersection, any-hit and closest-hit)
        assert(mNumHitShaders[groupIndex] == 0);

        uint32_t offset = 1; // there's always raygen shader

        for (uint32_t i = 0; i <= groupIndex; ++i) {
            offset += mNumHitShaders[i];
        }

        auto itStage = mStages.begin() + offset;
        mStages.insert(itStage, stages.begin(), stages.end());

        VkRayTracingShaderGroupCreateInfoNV groupInfo;
        groupInfo.sType = VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_NV;
        groupInfo.pNext = nullptr;
        groupInfo.type = VK_RAY_TRACING_SHADER_GROUP_TYPE_TRIANGLES_HIT_GROUP_NV;
        groupInfo.generalShader = VK_SHADER_UNUSED_NV;
        groupInfo.closestHitShader = VK_SHADER_UNUSED_NV;
        groupInfo.anyHitShader = VK_SHADER_UNUSED_NV;
        groupInfo.intersectionShader = VK_SHADER_UNUSED_NV;

        for (size_t i = 0; i < stages.size(); i++) {
            const VkPipelineShaderStageCreateInfo& stageInfo = stages[i];
            const uint32_t shaderIdx = static_cast<uint32_t>(offset + i);

            if (stageInfo.stage == VK_SHADER_STAGE_CLOSEST_HIT_BIT_NV) {
                groupInfo.closestHitShader = shaderIdx;
            } else if (stageInfo.stage == VK_SHADER_STAGE_ANY_HIT_BIT_NV) {
                groupInfo.anyHitShader = shaderIdx;
            }
        };

        mGroups.insert((mGroups.begin() + 1 + groupIndex), groupInfo);

        mNumHitShaders[groupIndex] += static_cast<uint32_t>(stages.size());
    }

    void SBTHelper::AddStageToMissGroup(const VkPipelineShaderStageCreateInfo& stage, const uint32_t groupIndex) {
        // raygen stage should go first!
        assert(!mStages.empty());

        assert(groupIndex < mNumMissShaders.size());
        assert(mNumMissShaders[groupIndex] == 0); // only 1 miss shader per group    

        uint32_t offset = 1; // there's always raygen shader

        // now skip all hit shaders
        for (const uint32_t numHitShader : mNumHitShaders) {
            offset += numHitShader;
        }

        for (uint32_t i = 0; i <= groupIndex; ++i) {
            offset += mNumMissShaders[i];
        }

        mStages.insert(mStages.begin() + offset, stage);

        // group create info 
        VkRayTracingShaderGroupCreateInfoNV groupInfo = {};
        groupInfo.sType = VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_NV;
        groupInfo.pNext = nullptr;
        groupInfo.type = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_NV;
        groupInfo.generalShader = offset;
        groupInfo.closestHitShader = VK_SHADER_UNUSED_NV;
        groupInfo.anyHitShader = VK_SHADER_UNUSED_NV;
        groupInfo.intersectionShader = VK_SHADER_UNUSED_NV;

        // group 0 is always for raygen, then go hit shaders
        mGroups.insert((mGroups.begin() + (groupIndex + 1 + mNumHitGroups)), groupInfo);

        mNumMissShaders[groupIndex]++;
    }

    uint32_t SBTHelper::GetGroupsStride() const {
        return mShaderHeaderSize;
    }

    uint32_t SBTHelper::GetNumGroups() const {
        return 1 + mNumHitGroups + mNumMissGroups;
    }

    uint32_t SBTHelper::GetRaygenOffset() const {
        return 0;
    }

    uint32_t SBTHelper::GetHitGroupsOffset() const {
        return 1 * mShaderHeaderSize;
    }

    uint32_t SBTHelper::GetMissGroupsOffset() const {
        return (1 + mNumHitGroups) * mShaderHeaderSize;
    }

    uint32_t SBTHelper::GetNumStages() const {
        return static_cast<uint32_t>(mStages.size());
    }

    const VkPipelineShaderStageCreateInfo* SBTHelper::GetStages() const {
        return mStages.data();
    }

    const VkRayTracingShaderGroupCreateInfoNV* SBTHelper::GetGroups() const {
        return mGroups.data();
    }

    uint32_t SBTHelper::GetSBTSize() const {
        return this->GetNumGroups() * mShaderHeaderSize;
    }

    bool SBTHelper::CreateSBT(VkDevice device, VkPipeline rtPipeline) {
        const size_t sbtSize = this->GetSBTSize();

        VkResult error = mSBT.Create(sbtSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_RAY_TRACING_BIT_NV, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
        CHECK_VK_ERROR(error, "mSBT.Create");

        if (VK_SUCCESS != error) {
            return false;
        }

        void* mem = mSBT.Map();
        error = vkGetRayTracingShaderGroupHandlesNV(device, rtPipeline, 0, this->GetNumGroups(), sbtSize, mem);
        CHECK_VK_ERROR(error, L"vkGetRaytracingShaderHandleNV");
        mSBT.Unmap();

        return (VK_SUCCESS == error);
    }

    VkBuffer SBTHelper::GetSBTBuffer() const {
        return mSBT.GetBuffer();
    }

#endif
*/

};
