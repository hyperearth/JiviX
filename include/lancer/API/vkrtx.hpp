#pragma once

#include "../lib/core.hpp"
#include "../API/memory.hpp"
#include "../API/buffer.hpp"
#include "../API/VMA.hpp"
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

    class SBTHelper_T : public std::enable_shared_from_this<SBTHelper_T> {
    public:
         SBTHelper_T(const DeviceMaker& device = {}, api::Pipeline* rtPipeline = nullptr) : mDevice(device), mNumHitGroups(0u), mNumMissGroups(0u) {};
        ~SBTHelper_T() = default;

        void        destroy();
        SBTHelper   initialize(const uint32_t& numHitGroups, const uint32_t& numMissGroups, const uint32_t& shaderHeaderSize);
        SBTHelper   setRaygenStage(const api::PipelineShaderStageCreateInfo& stage);
        SBTHelper   addStageToHitGroup(const std::vector<api::PipelineShaderStageCreateInfo>& stages, const uint32_t& groupIndex = 0u);
        SBTHelper   addStageToMissGroup(const api::PipelineShaderStageCreateInfo& stage, const uint32_t& groupIndex = 0u);
        SBTHelper   linkDevice(const DeviceMaker& device = {}) { this->mDevice = device; return shared_from_this(); }; 
        SBTHelper   linkBuffer(api::Buffer* buffer) { pSBT = buffer; };
        SBTHelper   linkPipeline(api::Pipeline* rtPipeline = nullptr) { this->mPipeline = rtPipeline; return shared_from_this(); };

        uint32_t    getGroupsStride() const;
        uint32_t    getNumGroups() const;
        uint32_t    getRaygenOffset() const;
        uint32_t    getHitGroupsOffset() const;
        uint32_t    getMissGroupsOffset() const;

        uint32_t                                   getNumStages() const;
        const api::PipelineShaderStageCreateInfo*     getStages() const;
        const api::RayTracingShaderGroupCreateInfoNV* getGroups() const;

        bool               createSBT();
        uint32_t           getSBTSize() const;
        const api::Buffer& getSBTBuffer() const;
              api::Buffer& getSBTBuffer();

    private:
        uint32_t                                              mShaderHeaderSize;
        uint32_t                                              mNumHitGroups;
        uint32_t                                              mNumMissGroups;
        std::vector<uint32_t>                                 mNumHitShaders;
        std::vector<uint32_t>                                 mNumMissShaders;
        std::vector<api::PipelineShaderStageCreateInfo>       mStages;
        std::vector<api::RayTracingShaderGroupCreateInfoNV>   mGroups;
        
        DeviceMaker                                           mDevice;
        BufferMaker                                           mSBT;
        Vector<>                                              vSBT;
        api::Buffer*                                          pSBT;
        api::Pipeline*                                        mPipeline;
        api::DescriptorBufferInfo                             mBufInfo;
    };
#endif

// TODO: Re-Implement SBT Helper 
#ifdef VKR_IMPLEMENTATION
    // SBT Helper class

    SBTHelper SBTHelper_T::initialize(const uint32_t& numHitGroups, const uint32_t& numMissGroups, const uint32_t& shaderHeaderSize) {
        mShaderHeaderSize = shaderHeaderSize;
        mNumHitGroups = numHitGroups;
        mNumMissGroups = numMissGroups;

        mNumHitShaders.resize(numHitGroups, 0u);
        mNumMissShaders.resize(numMissGroups, 0u);

        mStages.clear();
        mGroups.clear();
        
        return shared_from_this();
    }

    void SBTHelper_T::destroy() {
        mNumHitShaders.clear();
        mNumMissShaders.clear();
        mStages.clear();
        mGroups.clear();
    }

    SBTHelper SBTHelper_T::setRaygenStage(const api::PipelineShaderStageCreateInfo& stage) {
        // this shader stage should go first!
        assert(mStages.empty());
        mStages.push_back(stage);

        api::RayTracingShaderGroupCreateInfoNV groupInfo = {};
        groupInfo.type = api::RayTracingShaderGroupTypeNV::eGeneral;
        groupInfo.generalShader = 0;
        groupInfo.closestHitShader = VK_SHADER_UNUSED_NV;
        groupInfo.anyHitShader = VK_SHADER_UNUSED_NV;
        groupInfo.intersectionShader = VK_SHADER_UNUSED_NV;
        mGroups.push_back(groupInfo); // group 0 is always for raygen

        return shared_from_this();
    }

    SBTHelper SBTHelper_T::addStageToHitGroup(const std::vector<api::PipelineShaderStageCreateInfo>& stages, const uint32_t& groupIndex) {
        // raygen stage should go first!
        assert(groupIndex < mNumHitShaders.size());
        assert(!mStages.empty());
        assert(!stages.empty() && stages.size() <= 3);// only 3 hit shaders per group (intersection, any-hit and closest-hit)
        assert(mNumHitShaders[groupIndex] == 0);

        uint32_t offset = 1; // there's always raygen shader
        for (uint32_t i = 0; i <= groupIndex; ++i) {
            offset += mNumHitShaders[i];
        }

        auto itStage = mStages.begin() + offset;
        mStages.insert(itStage, stages.begin(), stages.end());

        api::RayTracingShaderGroupCreateInfoNV groupInfo;
        groupInfo.type = api::RayTracingShaderGroupTypeNV::eTrianglesHitGroup;
        groupInfo.generalShader = VK_SHADER_UNUSED_NV;
        groupInfo.closestHitShader = VK_SHADER_UNUSED_NV;
        groupInfo.anyHitShader = VK_SHADER_UNUSED_NV;
        groupInfo.intersectionShader = VK_SHADER_UNUSED_NV;

        for (size_t i = 0; i < stages.size(); i++) {
            const api::PipelineShaderStageCreateInfo& stageInfo = stages[i];
            const uint32_t shaderIdx = static_cast<uint32_t>(offset + i);

            if (stageInfo.stage == api::ShaderStageFlagBits::eClosestHitNV) {
                groupInfo.closestHitShader = shaderIdx; // Pricol 
            } else if (stageInfo.stage == api::ShaderStageFlagBits::eAnyHitNV) {
                groupInfo.anyHitShader = shaderIdx; // Pricol 
            }
        };

        mGroups.insert((mGroups.begin() + 1 + groupIndex), groupInfo);
        mNumHitShaders[groupIndex] += static_cast<uint32_t>(stages.size());
    }

    SBTHelper SBTHelper_T::addStageToMissGroup(const api::PipelineShaderStageCreateInfo& stage, const uint32_t& groupIndex) {
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
        api::RayTracingShaderGroupCreateInfoNV groupInfo = {};
        groupInfo.type = api::RayTracingShaderGroupTypeNV::eGeneral;
        groupInfo.generalShader = offset;
        groupInfo.closestHitShader = VK_SHADER_UNUSED_NV;
        groupInfo.anyHitShader = VK_SHADER_UNUSED_NV;
        groupInfo.intersectionShader = VK_SHADER_UNUSED_NV;

        // group 0 is always for raygen, then go hit shaders
        mGroups.insert((mGroups.begin() + (groupIndex + 1 + mNumHitGroups)), groupInfo);

        mNumMissShaders[groupIndex]++;
    }

    uint32_t SBTHelper_T::getGroupsStride() const {
        return mShaderHeaderSize;
    }

    uint32_t SBTHelper_T::getNumGroups() const {
        return 1 + mNumHitGroups + mNumMissGroups;
    }

    uint32_t SBTHelper_T::getRaygenOffset() const {
        return 0;
    }

    uint32_t SBTHelper_T::getHitGroupsOffset() const {
        return 1 * mShaderHeaderSize;
    }

    uint32_t SBTHelper_T::getMissGroupsOffset() const {
        return (1 + mNumHitGroups) * mShaderHeaderSize;
    }

    uint32_t SBTHelper_T::getNumStages() const {
        return static_cast<uint32_t>(mStages.size());
    }

    const api::PipelineShaderStageCreateInfo* SBTHelper_T::getStages() const {
        return mStages.data();
    }

    const api::RayTracingShaderGroupCreateInfoNV* SBTHelper_T::getGroups() const {
        return mGroups.data();
    }

    uint32_t SBTHelper_T::getSBTSize() const {
        return this->getNumGroups() * mShaderHeaderSize;
    }

    bool SBTHelper_T::createSBT() {
        const size_t sbtSize = this->getSBTSize();

        VmaAllocationCreateInfo allocInfo = {};
        allocInfo.usage = VMA_MEMORY_USAGE_CPU_TO_GPU;
        allocInfo.flags = VMA_ALLOCATION_CREATE_MAPPED_BIT;

        (mSBT = mDevice->createBufferMaker(api::BufferCreateInfo().setSize(sbtSize).setUsage(
            api::BufferUsageFlagBits::eRayTracingNV|
            api::BufferUsageFlagBits::eTransferDst|
            api::BufferUsageFlagBits::eTransferSrc|
            api::BufferUsageFlagBits::eHostVisible
        ), pSBT))->allocate(mDevice->getAllocator(),(uintptr_t)(&allocInfo));
        vSBT = std::make_shared<Vector_T<>>(mSBT->createRegion(&mBufInfo,0u,sbtSize));

        api::Result result = mDevice->least().getRayTracingShaderGroupHandlesNV(*mPipeline,0,this->getNumGroups(),sbtSize,mSBT->getMapped());
        return (result == api::Result::eSuccess);
    };

    api::Buffer& SBTHelper_T::getSBTBuffer() { return *pSBT; };
    const api::Buffer& SBTHelper_T::getSBTBuffer() const { return *pSBT; };

#endif

};
