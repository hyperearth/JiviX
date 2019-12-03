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

//#ifndef EXTENSION_RTX
//#define EXTENSION_RTX
//#endif

#ifdef EXTENSION_RTX // TODO: Create Ray Tracing Pipeline

    // Declare SBT Class 
    class SBTHelper_T : public std::enable_shared_from_this<SBTHelper_T> {
    public:
        SBTHelper_T(const DeviceMaker& device = {}, const api::RayTracingPipelineCreateInfoNV& rpt = {}, api::Pipeline* rtPipeline = nullptr) : mDevice(device), mPipeline(rtPipeline), mNumHitGroups(0u), mNumMissGroups(0u), mRTC(rpt) {
            if (mRTC.maxRecursionDepth < 1u) { mRTC.maxRecursionDepth = 4u; };
        };
        ~SBTHelper_T() { this->destroy(); };

        inline void      destroy();
        inline SBTHelper initialize(const uint32_t& numHitGroups = 0u, const uint32_t& numMissGroups = 0u, const uint32_t& shaderHeaderSize = 8u);
        inline SBTHelper setRaygenStage(const api::PipelineShaderStageCreateInfo& stage = {});
        inline SBTHelper addStageToHitGroup(const std::vector<api::PipelineShaderStageCreateInfo>& stages = {}, const uint32_t& groupIndex = 0u);
        inline SBTHelper addStageToMissGroup(const api::PipelineShaderStageCreateInfo& stage = {}, const uint32_t& groupIndex = 0u);
        inline SBTHelper linkDevice(const DeviceMaker& device = {}) { this->mDevice = device; return shared_from_this(); };
        inline SBTHelper linkBuffer(api::Buffer* buffer = nullptr) { pSBT = buffer; };
        inline SBTHelper linkPipeline(api::Pipeline* rtPipeline = nullptr) { this->mPipeline = rtPipeline; return shared_from_this(); };
        inline SBTHelper linkPipelineLayout(api::PipelineLayout* rtPipelineLayout = nullptr) { this->mPipelineLayout = rtPipelineLayout; return shared_from_this(); };

        inline uint32_t    getGroupsStride() const;
        inline uint32_t    getNumGroups() const;
        inline uint32_t    getRaygenOffset() const;
        inline uint32_t    getHitGroupsOffset() const;
        inline uint32_t    getMissGroupsOffset() const;

        inline uint32_t                                   getNumStages() const;
        inline const api::PipelineShaderStageCreateInfo*     getStages() const;
        inline const api::RayTracingShaderGroupCreateInfoNV* getGroups() const;

        inline bool               create();
        inline uint32_t           getSBTSize() const;
        inline const api::Buffer& getSBTBuffer() const;
        inline       api::Buffer& getSBTBuffer();

    protected:
        uint32_t                                              mShaderHeaderSize = 8u;
        uint32_t                                              mNumHitGroups = 0u;
        uint32_t                                              mNumMissGroups = 0u;
        std::vector<uint32_t>                                 mNumHitShaders = {};
        std::vector<uint32_t>                                 mNumMissShaders = {};
        std::vector<api::PipelineShaderStageCreateInfo>       mStages = {};
        std::vector<api::RayTracingShaderGroupCreateInfoNV>   mGroups = {};
        
        DeviceMaker                                           mDevice = {};
        BufferMaker                                           mSBT = {};
        Vector<>                                              vSBT = {}; // Required Re-Assigment...
        api::Buffer*                                          pSBT = nullptr;
        api::Pipeline*                                        mPipeline = nullptr;
        api::PipelineLayout*                                  mPipelineLayout = nullptr;
        api::DescriptorBufferInfo                             mBufInfo = {};
        api::RayTracingPipelineCreateInfoNV                   mRTC = {};
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


    // TODO: Update Scratch Memory Support
    // TODO: Smart Destructors Support
    class InstancedAcceleration_T : public std::enable_shared_from_this<InstancedAcceleration_T> {
    public:
        InstancedAcceleration_T(const DeviceMaker& device = {}, const api::AccelerationStructureCreateInfoNV& accelinfo = {}, api::AccelerationStructureNV* accelerat = nullptr) : device(device), accelinfo(accelinfo), accelerat(accelerat) {

        };

        inline size_t getRange() { return std::min(instanced.size(), cacheBuffer->size()) * sizeof(GeometryInstance); };
        inline InstancedAcceleration uploadCache() { memcpy(cacheBuffer->data(), instanced.data(), this->getRange()); };

        // Command Buffer Required Operations
        inline InstancedAcceleration uploadCmd(api::CommandBuffer& cmdbuf) { cmdbuf.copyBuffer(*cacheBuffer, *gpuBuffer, { vk::BufferCopy(cacheBuffer->offset(), gpuBuffer->offset(), this->getRange()) }); return shared_from_this(); };
        inline InstancedAcceleration createCmd(api::CommandBuffer& cmdbuf, const bool& update = false) { cmdbuf.buildAccelerationStructureNV(accelinfo.info, *gpuBuffer, gpuBuffer->offset(), update, *accelerat, {}, *scratch, scratch->offset()); return shared_from_this(); };

        // Create Finally 
        inline InstancedAcceleration create() {
            accelinfo.info.type = vk::AccelerationStructureTypeNV::eTopLevel;
            accelinfo.info.instanceCount = instanced.size();
            *accelerat = device->least().createAccelerationStructureNV(accelinfo);
            return shared_from_this();
        };

        // // Memory Manipulations
        inline InstancedAcceleration linkAcceleration(api::AccelerationStructureNV* accelerat = nullptr) { this->accelerat = accelerat; return shared_from_this(); };
        inline InstancedAcceleration linkCacheRegion(const std::shared_ptr<BufferRegion_T<GeometryInstance>>& region = {}) { this->cacheBuffer = region; return shared_from_this(); };
        inline InstancedAcceleration linkGPURegion(const std::shared_ptr<BufferRegion_T<GeometryInstance>>& region = {}) { this->gpuBuffer = region; return shared_from_this(); };
        inline InstancedAcceleration linkScratch(const std::shared_ptr<BufferRegion_T<uint8_t>>& scratch = {}) { this->scratch = scratch; return shared_from_this(); };
        inline InstancedAcceleration allocate(const MemoryAllocator& mem, const uintptr_t& ptx = 0u, const bool& reallocScratch = false) {
            // Set Structures Memory 
            auto requirements = device->least().getAccelerationStructureMemoryRequirementsNV(vk::AccelerationStructureMemoryRequirementsInfoNV().setAccelerationStructure(*accelerat).setType(vk::AccelerationStructureMemoryRequirementsTypeNV::eObject));
            mem->allocateForRequirements(allocation = mem->createAllocation(ptx), requirements);
            device->least().bindAccelerationStructureMemoryNV(vk::BindAccelerationStructureMemoryInfoNV().setMemory(allocation->getMemory()).setAccelerationStructure(*accelerat).setMemoryOffset(allocation->getMemoryOffset()));

            // Realloc when required
            if (reallocScratch || !scratch->least()->getAllocation()) {
                requirements = device->least().getAccelerationStructureMemoryRequirementsNV(vk::AccelerationStructureMemoryRequirementsInfoNV().setAccelerationStructure(*accelerat).setType(vk::AccelerationStructureMemoryRequirementsTypeNV::eBuildScratch));
                mem->allocateForRequirements(scratchall = mem->createAllocation(ptx), requirements);
                scratch->least()->linkAllocation(scratchall);
            };

            // 
            return shared_from_this();
        };

        // Device Native Allocator Support 
        inline InstancedAcceleration allocate(const uintptr_t& ptx = 0u, const bool& reallocScratch = false) {
            return this->allocate(device->getAllocatorPtr(), ptx, reallocScratch);
        };

        // Instance Pusher 
        inline InstancedAcceleration pushInstance(const GeometryInstance& instance = {}) { instanced.push_back(instance); };

        // Get Reference Of AS Info 
        inline const api::AccelerationStructureCreateInfoNV& getCreateInfo() const { return accelinfo; };
        inline api::AccelerationStructureCreateInfoNV& getCreateInfo() { return accelinfo; };

        // Instances List Getter
        inline std::vector<GeometryInstance>& getInstances() { return instanced; };
        inline const std::vector<GeometryInstance>& getInstances() const { return instanced; };

        // Scratch Buffer Getter 
        inline std::shared_ptr<BufferRegion_T<uint8_t>>& getScratch() { return scratch; };
        inline const std::shared_ptr<BufferRegion_T<uint8_t>>& getScratch() const { return scratch; };

        // Cache Buffer Getter
        inline std::shared_ptr<BufferRegion_T<GeometryInstance>>& getCacheBuffer() { return cacheBuffer; };
        inline const std::shared_ptr<BufferRegion_T<GeometryInstance>>& getCacheBuffer() const { return cacheBuffer; };

        // GPU Buffer Getter
        inline std::shared_ptr<BufferRegion_T<GeometryInstance>>& getGPUBuffer() { return gpuBuffer; };
        inline const std::shared_ptr<BufferRegion_T<GeometryInstance>>& getGPUBuffer() const { return gpuBuffer; };

        // Instance Getter 
        inline GeometryInstance& getInstance() { return instanced.back(); };
        inline const GeometryInstance& getInstance() const { return instanced.back(); };

        // Write For Descriptor
        inline InstancedAcceleration writeForDescription(api::WriteDescriptorSetAccelerationStructureNV* AS = nullptr) {
            AS->accelerationStructureCount = 1u, AS->pAccelerationStructures = accelerat;
        };

    protected:
        DeviceMaker device = {};
        std::shared_ptr<BufferRegion_T<GeometryInstance>> cacheBuffer = {}; // Cache Buffer Should Be Mapped!
        std::shared_ptr<BufferRegion_T<GeometryInstance>> gpuBuffer = {};
        std::shared_ptr<BufferRegion_T<uint8_t>> scratch = {};
        api::AccelerationStructureCreateInfoNV accelinfo = {};
        api::AccelerationStructureNV* accelerat = nullptr;
        std::vector<GeometryInstance> instanced = {};
        MemoryAllocation allocation = {};
        MemoryAllocation scratchall = {};
    };


    // TODO: Custom Vertex Buffers Support (And Custom Host Types)
    // TODO: Update Scratch Memory Support
    // TODO: Smart Destructors Support 
    class GeometryAcceleration_T : public std::enable_shared_from_this<GeometryAcceleration_T> {
    public:
        GeometryAcceleration_T(const DeviceMaker& device = {}, const api::AccelerationStructureCreateInfoNV& accelinfo = {}, api::AccelerationStructureNV* accelerat = nullptr) : device(device), accelinfo(accelinfo), accelerat(accelerat) {

        };

        // Create Finally 
        inline GeometryAcceleration create() {
            accelinfo.info.type = vk::AccelerationStructureTypeNV::eBottomLevel;
            accelinfo.info.geometryCount = geometries.size();
            accelinfo.info.pGeometries = geometries.data();

            // create itself 
            *accelerat = device->least().createAccelerationStructureNV(accelinfo);
            return shared_from_this();
        };

        // Command Buffer Required Operations
        inline GeometryAcceleration createCmd(api::CommandBuffer& cmdbuf, const bool& update = false) { cmdbuf.buildAccelerationStructureNV(accelinfo.info, {}, 0u, update, *accelerat, {}, *scratch, scratch->offset()); return shared_from_this(); };

        // Memory Manipulations
        inline GeometryAcceleration linkScratch(const std::shared_ptr<BufferRegion_T<uint8_t>>& scratch = {}) { this->scratch = scratch; return shared_from_this(); };
        inline GeometryAcceleration linkAcceleration(api::AccelerationStructureNV* accelerat = nullptr) { this->accelerat = accelerat; return shared_from_this(); };
        inline GeometryAcceleration allocate(const MemoryAllocator& mem, const uintptr_t& ptx = 0u, const bool& reallocScratch = false) {
            // Set Structures Memory 
            auto requirements = device->least().getAccelerationStructureMemoryRequirementsNV(vk::AccelerationStructureMemoryRequirementsInfoNV().setAccelerationStructure(*accelerat).setType(vk::AccelerationStructureMemoryRequirementsTypeNV::eObject));
            mem->allocateForRequirements(allocation = mem->createAllocation(ptx), requirements);
            device->least().bindAccelerationStructureMemoryNV(vk::BindAccelerationStructureMemoryInfoNV().setMemory(allocation->getMemory()).setAccelerationStructure(*accelerat).setMemoryOffset(allocation->getMemoryOffset()));

            // Realloc when required
            if (reallocScratch || !scratch->least()->getAllocation()) {
                requirements = device->least().getAccelerationStructureMemoryRequirementsNV(vk::AccelerationStructureMemoryRequirementsInfoNV().setAccelerationStructure(*accelerat).setType(vk::AccelerationStructureMemoryRequirementsTypeNV::eBuildScratch));
                mem->allocateForRequirements(scratchall = mem->createAllocation(ptx), requirements);
                scratch->least()->linkAllocation(scratchall);
            };
            
            // 
            return shared_from_this();
        };

        // Device Native Allocator Support 
        inline GeometryAcceleration allocate(const uintptr_t& ptx = 0u, const bool& reallocScratch = false) {
            return this->allocate(device->getAllocatorPtr(), ptx, reallocScratch);
        };

        // Use some Vookoo style 
        inline GeometryAcceleration beginTriangles(const api::GeometryNV& geometry = {}) {
            geometries.push_back(geometry);
            geometries.back().geometryType = api::GeometryTypeNV::eTriangles;
            return shared_from_this();
        };

        // TODO: Assert by Triangles
        inline const api::GeometryAABBNV& getAABB() const { return geometries.back().geometry.aabbs; };
        inline api::GeometryAABBNV& getAABB() { return geometries.back().geometry.aabbs; };

        // TODO: Assert by AABB
        inline const api::GeometryTrianglesNV& getTriangles() const { return geometries.back().geometry.triangles; };
        inline api::GeometryTrianglesNV& getTriangles() { return geometries.back().geometry.triangles; };

        // Get Reference Of AS Info 
        inline const api::AccelerationStructureCreateInfoNV& getCreateInfo() const { return accelinfo;  };
        inline api::AccelerationStructureCreateInfoNV& getCreateInfo() { return accelinfo; };

        // Scratch Buffer Getter 
        inline std::shared_ptr<BufferRegion_T<uint8_t>>& getScratch() { return scratch; };
        inline const std::shared_ptr<BufferRegion_T<uint8_t>>& getScratch() const { return scratch; };

        // fVec4
        inline GeometryAcceleration setVertex4x32f(const std::shared_ptr<BufferRegion_T<vec4_t>>& vertex = {}) {
            this->getTriangles().vertexFormat = vk::Format::eR32G32B32A32Sfloat;
            this->getTriangles().vertexCount = vertex->size();
            this->getTriangles().vertexData = *vertex;
            this->getTriangles().vertexOffset = vertex->offset();
            this->getTriangles().vertexStride = sizeof(vec4_t);
            return shared_from_this();
        };

        // fVec3
        inline GeometryAcceleration setVertex3x32f(const std::shared_ptr<BufferRegion_T<vec3_t>>& vertex = {}) {
            this->getTriangles().vertexFormat = vk::Format::eR32G32B32Sfloat;
            this->getTriangles().vertexCount = vertex->size();
            this->getTriangles().vertexData = *vertex;
            this->getTriangles().vertexOffset = vertex->offset();
            this->getTriangles().vertexStride = sizeof(vec3_t);
            return shared_from_this();
        };

        // Transform Buffer 3x4
        inline GeometryAcceleration setTransform3x4(const std::shared_ptr<BufferRegion_T<transform3x4_t>>& matrix = {}) {
            this->getTriangles().transformData = *matrix;
            this->getTriangles().transformOffset = matrix->offset();
            return shared_from_this();
        };

        // Uint32_T
        inline GeometryAcceleration setIndices32u(const std::shared_ptr<BufferRegion_T<uint32_t>>& indices = {}) {
            this->getTriangles().indexType = api::IndexType::eUint32;
            this->getTriangles().indexData = *indices;
            this->getTriangles().indexOffset = indices->offset();
            this->getTriangles().indexCount = indices->size();
            return shared_from_this();
        };

        // Uint16_T
        inline GeometryAcceleration setIndices16u(const std::shared_ptr<BufferRegion_T<uint16_t>>& indices = {}) {
            this->getTriangles().indexType = api::IndexType::eUint16;
            this->getTriangles().indexData = *indices;
            this->getTriangles().indexOffset = indices->offset();
            this->getTriangles().indexCount = indices->size();
            return shared_from_this();
        };

        // Uint8_T
        inline GeometryAcceleration setIndices8u(const std::shared_ptr<BufferRegion_T<uint8_t>>& indices = {}) {
            this->getTriangles().indexType = api::IndexType::eUint8EXT;
            this->getTriangles().indexData = *indices;
            this->getTriangles().indexOffset = indices->offset();
            this->getTriangles().indexCount = indices->size();
            return shared_from_this();
        };

        // SHOULD BE READY
        inline GeometryAcceleration writeForInstance(GeometryInstance& instance) {
            device->least().getAccelerationStructureHandleNV(*accelerat, sizeof(uint64_t), &instance.accelerationStructureHandle);
            return shared_from_this();
        };

    protected:
        DeviceMaker device = {};
        std::shared_ptr<BufferRegion_T<uint8_t>> scratch = {};
        api::AccelerationStructureCreateInfoNV accelinfo = {};
        api::AccelerationStructureNV* accelerat = nullptr;
        std::vector<api::GeometryNV> geometries = {};
        MemoryAllocation allocation = {};
        MemoryAllocation scratchall = {};
    };


// For Debug, Enable Extension
#ifndef EXTENSION_RTX_IMPLEMENT
#define EXTENSION_RTX_IMPLEMENT
#endif

#endif

// TODO: Re-Implement SBT Helper 
#ifdef EXTENSION_RTX_IMPLEMENT
    // SBT Helper class

    inline SBTHelper SBTHelper_T::initialize(const uint32_t& numHitGroups, const uint32_t& numMissGroups, const uint32_t& shaderHeaderSize) {
        this->mShaderHeaderSize = shaderHeaderSize;
        this->mNumHitGroups = numHitGroups;
        this->mNumMissGroups = numMissGroups;

        this->mNumHitShaders.resize(numHitGroups, 0u);
        this->mNumMissShaders.resize(numMissGroups, 0u);

        this->mStages.clear();
        this->mGroups.clear();

        // Get Directly from Device 
        if (!!this->mDevice) {
            auto rtx_prop = api::PhysicalDeviceRayTracingPropertiesNV{};
            auto [properties, features] = this->mDevice->getHelper()->getFeaturesWithProperties(&rtx_prop);
            this->mShaderHeaderSize = rtx_prop.shaderGroupHandleSize;
        };

        return shared_from_this();
    };

    inline void SBTHelper_T::destroy() {
        mNumHitShaders.clear();
        mNumMissShaders.clear();
        mStages.clear();
        mGroups.clear();
    };

    inline SBTHelper SBTHelper_T::setRaygenStage(const api::PipelineShaderStageCreateInfo& stage) {
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
    };

    inline SBTHelper SBTHelper_T::addStageToHitGroup(const std::vector<api::PipelineShaderStageCreateInfo>& stages, const uint32_t& groupIndex) {
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
            }
            else if (stageInfo.stage == api::ShaderStageFlagBits::eAnyHitNV) {
                groupInfo.anyHitShader = shaderIdx; // Pricol 
            }
        };

        mGroups.insert((mGroups.begin() + 1 + groupIndex), groupInfo);
        mNumHitShaders[groupIndex] += static_cast<uint32_t>(stages.size());

        return shared_from_this();
    };

    inline SBTHelper SBTHelper_T::addStageToMissGroup(const api::PipelineShaderStageCreateInfo& stage, const uint32_t& groupIndex) {
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

        return shared_from_this();
    };

    inline uint32_t SBTHelper_T::getGroupsStride() const { return mShaderHeaderSize; };
    inline uint32_t SBTHelper_T::getNumGroups() const { return 1 + mNumHitGroups + mNumMissGroups; };
    inline uint32_t SBTHelper_T::getRaygenOffset() const { return 0; };
    inline uint32_t SBTHelper_T::getHitGroupsOffset() const { return 1 * mShaderHeaderSize; };
    inline uint32_t SBTHelper_T::getMissGroupsOffset() const { return (1 + mNumHitGroups) * mShaderHeaderSize; };
    inline uint32_t SBTHelper_T::getNumStages() const { return static_cast<uint32_t>(mStages.size()); };
    inline uint32_t SBTHelper_T::getSBTSize() const { return this->getNumGroups() * mShaderHeaderSize; };

    inline const api::PipelineShaderStageCreateInfo* SBTHelper_T::getStages() const { return mStages.data(); };
    inline const api::RayTracingShaderGroupCreateInfoNV* SBTHelper_T::getGroups() const { return mGroups.data(); };
    inline const api::Buffer& SBTHelper_T::getSBTBuffer() const { return *pSBT; };
    inline api::Buffer& SBTHelper_T::getSBTBuffer() { return *pSBT; };

    inline bool SBTHelper_T::create() {
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
        mRTC.stageCount = this->getNumStages();
        mRTC.pGroups = this->getGroups();
        mRTC.pStages = this->getStages();
        mRTC.layout = *this->mPipelineLayout;

        // Create Pipeline And SBT
        const auto result = mDevice->least().getRayTracingShaderGroupHandlesNV(
            (*mPipeline = mDevice->least().createRayTracingPipelineNV(mDevice->getPipelineCache(), mRTC)),0u,this->getNumGroups(),sbtSize,mSBT->getMapped());

        // 
        return (result == api::Result::eSuccess);
    };
#endif

};
