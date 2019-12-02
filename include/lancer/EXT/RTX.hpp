#pragma once

#include "../lib/core.hpp"
#include "../API/memory.hpp"
#include "../API/buffer.hpp"
#include "../EXT/VMA.hpp"
#include <algorithm>
#include <fstream>
#include <iostream>
#include <functional>


// TODO: Another File for GLM Extensions
#ifdef EXTENSION_GLM
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/vec_swizzle.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/random.hpp>
#include <glm/gtx/component_wise.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtx/transform.hpp>
#endif



namespace lancer {

#ifndef EXTENSION_RTX
#define EXTENSION_RTX
#endif

#ifndef EXTENSION_RTX_IMPLEMENT
#define EXTENSION_RTX_IMPLEMENT
#endif

#ifdef EXTENSION_RTX // TODO: Create Ray Tracing Pipeline
    class SBTHelper_T;
    using SBTHelper = std::shared_ptr<SBTHelper_T>;

    class InstancedAcceleration_T;
    using InstancedAcceleration = std::shared_ptr<InstancedAcceleration_T>;

    class GeometryAcceleration_T;
    using GeometryAcceleration = std::shared_ptr<GeometryAcceleration_T>;

    // Declare SBT Class 
    class SBTHelper_T : public std::enable_shared_from_this<SBTHelper_T> {
    public:
         SBTHelper_T(const api::RayTracingPipelineCreateInfoNV& rpt = api::RayTracingPipelineCreateInfoNV().setMaxRecursionDepth(4u), const DeviceMaker& device = {}, api::Pipeline* rtPipeline = nullptr) : mDevice(device), mPipeline(rtPipeline), mNumHitGroups(0u), mNumMissGroups(0u), mRTC(rpt) {};
        ~SBTHelper_T() = default;

        void        destroy();
        SBTHelper&& initialize(const uint32_t& numHitGroups, const uint32_t& numMissGroups, const uint32_t& shaderHeaderSize);
        SBTHelper&& setRaygenStage(const api::PipelineShaderStageCreateInfo& stage);
        SBTHelper&& addStageToHitGroup(const std::vector<api::PipelineShaderStageCreateInfo>& stages, const uint32_t& groupIndex = 0u);
        SBTHelper&& addStageToMissGroup(const api::PipelineShaderStageCreateInfo& stage, const uint32_t& groupIndex = 0u);
        SBTHelper&& linkDevice(const DeviceMaker& device = {}) { this->mDevice = device; return shared_from_this(); };
        SBTHelper&& linkBuffer(api::Buffer* buffer) { pSBT = buffer; };
        SBTHelper&& linkPipeline(api::Pipeline* rtPipeline = nullptr) { this->mPipeline = rtPipeline; return shared_from_this(); };
        SBTHelper&& linkPipelineLayout(api::PipelineLayout* rtPipelineLayout = nullptr) { this->mPipelineLayout = rtPipelineLayout; return shared_from_this(); };

        uint32_t    getGroupsStride() const;
        uint32_t    getNumGroups() const;
        uint32_t    getRaygenOffset() const;
        uint32_t    getHitGroupsOffset() const;
        uint32_t    getMissGroupsOffset() const;

        uint32_t                                   getNumStages() const;
        const api::PipelineShaderStageCreateInfo*     getStages() const;
        const api::RayTracingShaderGroupCreateInfoNV* getGroups() const;

        bool               create();
        uint32_t           getSBTSize() const;
        const api::Buffer& getSBTBuffer() const;
              api::Buffer& getSBTBuffer();

    protected:
        uint32_t                                              mShaderHeaderSize;
        uint32_t                                              mNumHitGroups;
        uint32_t                                              mNumMissGroups;
        std::vector<uint32_t>                                 mNumHitShaders;
        std::vector<uint32_t>                                 mNumMissShaders;
        std::vector<api::PipelineShaderStageCreateInfo>       mStages;
        std::vector<api::RayTracingShaderGroupCreateInfoNV>   mGroups;
        
        DeviceMaker                                           mDevice;
        BufferMaker                                           mSBT;
        Vector<>                                              vSBT; // Required Re-Assigment...
        api::Buffer*                                          pSBT;
        api::Pipeline*                                        mPipeline;
        api::PipelineLayout*                                  mPipelineLayout;
        api::DescriptorBufferInfo                             mBufInfo;
        api::RayTracingPipelineCreateInfoNV                   mRTC;
    };

#ifdef EXTENSION_GLM
    using transform3x4_t = glm::mat3x4;
    using vec4_t = glm::vec4;
    using vec3_t = glm::vec3;
#else
    using transform3x4_t = float[12];
    using vec4_t = float[4];
    using vec3_t = float[3];
#endif



#pragma pack(push, 1)
    struct GeometryInstance {
#ifdef EXTENSION_GLM
        glm::mat3x4 transform = {};
#else
        float transform[12] = {
            1.f,0.f,0.f,0.f,
            0.f,1.f,0.f,0.f,
            0.f,0.f,1.f,0.f
        };
#endif
        uint32_t instanceId : 24;
        uint32_t mask : 8;
        uint32_t instanceOffset : 24;
        uint32_t flags : 8;
        uint64_t accelerationStructureHandle;
    };
#pragma pack(pop)



    class InstancedAcceleration_T : public std::enable_shared_from_this<InstancedAcceleration_T> {
    public:
        InstancedAcceleration_T(const DeviceMaker& device = {}, const api::AccelerationStructureCreateInfoNV& accelinfo = {}, api::AccelerationStructureNV* accelerat = nullptr) : device(device), accelinfo(accelinfo), accelerat(accelerat) {

        };

        InstancedAcceleration linkAcceleration(api::AccelerationStructureNV* accelerat = nullptr) { this->accelerat = accelerat; return shared_from_this(); };
        InstancedAcceleration linkCacheRegion(const std::shared_ptr<BufferRegion_T<GeometryInstance>>& region = {}) { this->cacheBuffer = region; return shared_from_this(); };
        InstancedAcceleration linkGPURegion(const std::shared_ptr<BufferRegion_T<GeometryInstance>>& region = {}) { this->gpuBuffer = region; return shared_from_this(); };

        size_t getRange() { return std::min(instanced.size(), cacheBuffer->size()) * sizeof(GeometryInstance); };
        InstancedAcceleration uploadCmd(api::CommandBuffer& cmdbuf) { cmdbuf.copyBuffer(*cacheBuffer, *gpuBuffer, { vk::BufferCopy(cacheBuffer->offset(), gpuBuffer->offset(), this->getRange()) }); };
        InstancedAcceleration uploadCache() { memcpy(cacheBuffer->data(), instanced.data(), this->getRange()); };

        // Create Finally 
        InstancedAcceleration create() {
            accelinfo.info.type = vk::AccelerationStructureTypeNV::eTopLevel;
            accelinfo.info.instanceCount = instanced.size();

            *accelerat = device->least().createAccelerationStructureNV(accelinfo);
            return shared_from_this();
        };

        // Instance Pusher 
        InstancedAcceleration pushInstance(const GeometryInstance& instance = {}) { instanced.push_back(instance); };

        // Instance Getter 
        GeometryInstance& getInstance() { return instanced.back(); };
        const GeometryInstance& getInstance() const { return instanced.back(); };


    protected:
        DeviceMaker device = {};
        std::shared_ptr<BufferRegion_T<GeometryInstance>> cacheBuffer = {}; // Cache Buffer Should Be Mapped!
        std::shared_ptr<BufferRegion_T<GeometryInstance>> gpuBuffer = {};
        api::AccelerationStructureCreateInfoNV accelinfo = {};
        api::AccelerationStructureNV* accelerat = nullptr;
        std::vector<GeometryInstance> instanced = {};
    };



    class GeometryAcceleration_T : public std::enable_shared_from_this<GeometryAcceleration_T> {
    public:
        GeometryAcceleration_T(const DeviceMaker& device = {}, const api::AccelerationStructureCreateInfoNV& accelinfo = {}, api::AccelerationStructureNV* accelerat = nullptr) : device(device), accelinfo(accelinfo), accelerat(accelerat) {

        };

        GeometryAcceleration linkAcceleration(api::AccelerationStructureNV* accelerat = nullptr) { this->accelerat = accelerat; return shared_from_this(); };
        GeometryAcceleration create() {
            accelinfo.info.type = vk::AccelerationStructureTypeNV::eBottomLevel;
            accelinfo.info.geometryCount = geometries.size();
            accelinfo.info.pGeometries = geometries.data();

            *accelerat = device->least().createAccelerationStructureNV(accelinfo);
            return shared_from_this();
        };

        // Use some Vookoo style 
        GeometryAcceleration beginTriangles(const api::GeometryNV& geometry = {}) {
            geometries.push_back(geometry);
            geometries.back().geometryType = api::GeometryTypeNV::eTriangles;
            return shared_from_this();
        };

        // TODO: Assert by Triangles
        const api::GeometryAABBNV& getAABB() const {
            return geometries.back().geometry.aabbs;
        };

        // TODO: Assert by Triangles
        api::GeometryAABBNV& getAABB() {
            return geometries.back().geometry.aabbs;
        };

        // TODO: Assert by AABB
        const api::GeometryTrianglesNV& getTriangles() const {
            return geometries.back().geometry.triangles;
        };

        // TODO: Assert by AABB
        api::GeometryTrianglesNV& getTriangles() {
            return geometries.back().geometry.triangles;
        };

        // fVec4
        GeometryAcceleration setVertex4x32f(const std::shared_ptr<BufferRegion_T<vec4_t>>& vertex = {}) {
            this->getTriangles().vertexFormat = vk::Format::eR32G32B32A32Sfloat;
            this->getTriangles().vertexCount = vertex->size();
            this->getTriangles().vertexData = *vertex;
            this->getTriangles().vertexOffset = vertex->offset();
            this->getTriangles().vertexStride = sizeof(vec4_t);
            return shared_from_this();
        };

        // fVec3
        GeometryAcceleration setVertex3x32f(const std::shared_ptr<BufferRegion_T<vec3_t>>& vertex = {}) {
            this->getTriangles().vertexFormat = vk::Format::eR32G32B32Sfloat;
            this->getTriangles().vertexCount = vertex->size();
            this->getTriangles().vertexData = *vertex;
            this->getTriangles().vertexOffset = vertex->offset();
            this->getTriangles().vertexStride = sizeof(vec3_t);
            return shared_from_this();
        };

        // Transform Buffer 3x4
        GeometryAcceleration setTransform3x4(const std::shared_ptr<BufferRegion_T<transform3x4_t>>& matrix = {}) {
            this->getTriangles().transformData = *matrix;
            this->getTriangles().transformOffset = matrix->offset();
            return shared_from_this();
        };

        // Uint32_T
        GeometryAcceleration setIndices32u(const std::shared_ptr<BufferRegion_T<uint32_t>>& indices = {}) {
            this->getTriangles().indexType = api::IndexType::eUint32;
            this->getTriangles().indexData = *indices;
            this->getTriangles().indexOffset = indices->offset();
            this->getTriangles().indexCount = indices->size();
            return shared_from_this();
        };

        // Uint16_T
        GeometryAcceleration setIndices16u(const std::shared_ptr<BufferRegion_T<uint16_t>>& indices = {}) {
            this->getTriangles().indexType = api::IndexType::eUint16;
            this->getTriangles().indexData = *indices;
            this->getTriangles().indexOffset = indices->offset();
            this->getTriangles().indexCount = indices->size();
            return shared_from_this();
        };

        // Uint8_T
        GeometryAcceleration setIndices8u(const std::shared_ptr<BufferRegion_T<uint8_t>>& indices = {}) {
            this->getTriangles().indexType = api::IndexType::eUint8EXT;
            this->getTriangles().indexData = *indices;
            this->getTriangles().indexOffset = indices->offset();
            this->getTriangles().indexCount = indices->size();
            return shared_from_this();
        };

        // SHOULD BE READY
        GeometryAcceleration writeForInstance(GeometryInstance& instance) {
            device->least().getAccelerationStructureHandleNV(*accelerat, sizeof(uint64_t), &instance.accelerationStructureHandle);
        };

    protected:
        DeviceMaker device = {};
        api::AccelerationStructureCreateInfoNV accelinfo = {};
        api::AccelerationStructureNV* accelerat = nullptr;
        std::vector<api::GeometryNV> geometries = {};
    };

#endif

// TODO: Re-Implement SBT Helper 
#ifdef EXTENSION_RTX_IMPLEMENT
    // SBT Helper class

    SBTHelper&& SBTHelper_T::initialize(const uint32_t& numHitGroups, const uint32_t& numMissGroups, const uint32_t& shaderHeaderSize) {
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

    SBTHelper&& SBTHelper_T::setRaygenStage(const api::PipelineShaderStageCreateInfo& stage) {
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

    SBTHelper&& SBTHelper_T::addStageToHitGroup(const std::vector<api::PipelineShaderStageCreateInfo>& stages, const uint32_t& groupIndex) {
        // raygen stage should go first!
        assert(groupIndex < mNumHitShaders.size());
        assert(!mStages.empty());
        assert(!stages.empty() && stages.size() <= 3);// only 3 hit shaders per group (intersection, any-hit and closest-hit)
        assert(mNumHitShaders[groupIndex] == 0);

        uint32_t offset = 1u; // there's always raygen shader
        for (uint32_t i = 0u; i <= groupIndex; ++i) { offset += mNumHitShaders[i]; };

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

    SBTHelper&& SBTHelper_T::addStageToMissGroup(const api::PipelineShaderStageCreateInfo& stage, const uint32_t& groupIndex) {
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

    bool SBTHelper_T::create() {
        const size_t sbtSize = this->getSBTSize();

        VmaAllocationCreateInfo allocInfo = {};
        allocInfo.usage = VMA_MEMORY_USAGE_CPU_TO_GPU;
        allocInfo.flags = VMA_ALLOCATION_CREATE_MAPPED_BIT;

        (mSBT = mDevice->createBufferMaker(api::BufferCreateInfo().setSize(sbtSize).setUsage(
            api::BufferUsageFlagBits::eRayTracingNV|
            api::BufferUsageFlagBits::eTransferDst|
            api::BufferUsageFlagBits::eTransferSrc
        ), pSBT))->allocate(mDevice->getAllocatorPtr(),(uintptr_t)(&allocInfo));
        vSBT = Vector<>(mSBT->createRegion(&mBufInfo,0u,sbtSize));

        // Assign Truth
        mRTC.groupCount = this->getNumGroups();
        mRTC.pGroups = this->getGroups();
        mRTC.stageCount = this->getNumStages();
        mRTC.pStages = this->getStages();
        mRTC.layout = *this->mPipelineLayout;

        // Create Pipeline And SBT
        const auto result = mDevice->least().getRayTracingShaderGroupHandlesNV(
            (*mPipeline = mDevice->least().createRayTracingPipelineNV(mDevice->getPipelineCache(), mRTC)),0u,this->getNumGroups(),sbtSize,mSBT->getMapped());

        // 
        return (result == api::Result::eSuccess);
    };

    api::Buffer& SBTHelper_T::getSBTBuffer() { return *pSBT; };
    const api::Buffer& SBTHelper_T::getSBTBuffer() const { return *pSBT; };
#endif

};
