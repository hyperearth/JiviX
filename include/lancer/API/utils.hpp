#pragma once

#include "../lib/core.hpp"
#include "../API/memory.hpp"
#include <algorithm>
#include <fstream>
#include <iostream>
#include <functional>

namespace lancer {

    template <typename T>
    static inline auto sgn(const T& val) { return (T(0) < val) - (val < T(0)); }

    template<class T = uint64_t>
    static inline T tiled(const T& sz, const T& gmaxtile) {
        // return (int32_t)ceil((double)sz / (double)gmaxtile);
        return sz <= 0 ? 0 : (sz / gmaxtile + sgn(sz % gmaxtile));
    }

    template <class T>
    static inline auto strided(const size_t& sizeo) { return sizeof(T) * sizeo; }

    // Read binary (for SPIR-V)
    // Updated 03.12.2019 (add No Data error)
    static inline auto readBinary(const std::string& filePath ) {
        std::ifstream file(filePath, std::ios::in | std::ios::binary | std::ios::ate);
        std::vector<uint32_t> data = {};
        if (file.is_open()) {
            std::streampos size = file.tellg();
            data.resize(tiled(size_t(size), sizeof(uint32_t)));
            file.seekg(0, std::ios::beg);
            file.read((char *)data.data(), size);
            file.close();
        }
        else {
            std::cerr << "Failure to open " + filePath << std::endl;
        };
        if (data.size() < 1u) { std::cerr << "NO DATA " + filePath << std::endl; };
        return data;
    };

    // read source (unused)
    static inline auto readSource(const std::string& filePath, bool lineDirective = false ) {
        std::string content = "";
        std::ifstream fileStream(filePath, std::ios::in);
        if (!fileStream.is_open()) {
            std::cerr << "Could not read file " << filePath << ". File does not exist." << std::endl; return content;
        }
        std::string line = "";
        while (!fileStream.eof()) {
            std::getline(fileStream, line);
            if (lineDirective || line.find("#line") == std::string::npos) content.append(line + "\n");
        }
        fileStream.close();
        return content;
    };

    static inline auto makeShaderModuleInfo(const std::vector<uint32_t>& code) {
        auto smi = api::ShaderModuleCreateInfo{};
        smi.pCode = (uint32_t *)code.data();
        smi.codeSize = code.size()*4;
        smi.flags = {};
        return smi;
    };

    // create shader module
    static inline auto& createShaderModuleIntrusive(const api::Device& device, const std::vector<uint32_t>& code, api::ShaderModule& hndl) {
        return (hndl = device.createShaderModule(makeShaderModuleInfo(code)));
    };

    static inline auto createShaderModule(const api::Device& device, const std::vector<uint32_t>& code) {
        auto sm = api::ShaderModule{}; return std::move(createShaderModuleIntrusive(device, code, sm));
    };

    struct FixConstruction {
        api::PipelineShaderStageCreateInfo spi = {};
        api::PipelineShaderStageRequiredSubgroupSizeCreateInfoEXT sgmp = {};

        operator api::PipelineShaderStageCreateInfo& () { return spi; };
        operator const api::PipelineShaderStageCreateInfo& () const { return spi; };
        operator api::PipelineShaderStageRequiredSubgroupSizeCreateInfoEXT& () { return sgmp; };
        operator const api::PipelineShaderStageRequiredSubgroupSizeCreateInfoEXT& () const { return sgmp; };
    };

    // create shader module
    static inline auto makePipelineStageInfo(const api::Device& device, const std::vector<uint32_t>& code, const char * entry = "main", api::ShaderStageFlagBits stage = api::ShaderStageFlagBits::eCompute) {
        api::PipelineShaderStageCreateInfo spi = {};
        createShaderModuleIntrusive(device, code, spi.module);
        spi.pName = entry;
        spi.stage = stage;
        spi.pSpecializationInfo = {};
        return std::move(spi);
    };

    // create shader module
    static inline auto makeComputePipelineStageInfo(const api::Device& device, const std::vector<uint32_t>& code, const char * entry = "main", const uint32_t& subgroupSize = 0u) {
        auto f = FixConstruction{};

        f.spi = api::PipelineShaderStageCreateInfo{};;
        f.spi.flags = api::PipelineShaderStageCreateFlagBits::eRequireFullSubgroupsEXT;
        createShaderModuleIntrusive(device, code, f.spi.module);
        f.spi.pName = entry;
        f.spi.stage = api::ShaderStageFlagBits::eCompute;
        f.spi.pSpecializationInfo = {};

        f.sgmp = api::PipelineShaderStageRequiredSubgroupSizeCreateInfoEXT{};
        f.sgmp.requiredSubgroupSize = subgroupSize;
        if (subgroupSize) f.spi.pNext = &f.sgmp;

        return std::move(f);
    };

    // create compute pipelines
    static inline auto createCompute(const api::Device& device, const FixConstruction& spi, const api::PipelineLayout& layout, const api::PipelineCache& cache = {}, const uint32_t& subgroupSize = 0u) {
        auto cmpi = api::ComputePipelineCreateInfo{};
        cmpi.flags = {};
        cmpi.layout = layout;
        cmpi.stage = spi;
        cmpi.basePipelineIndex = -1;
        return device.createComputePipeline(cache, cmpi);
    };

    // create compute pipelines
    static inline auto createCompute(const api::Device& device, const std::vector<uint32_t>& code, const api::PipelineLayout& layout, const api::PipelineCache& cache = {}, const uint32_t& subgroupSize = 0u) {
        auto f = makeComputePipelineStageInfo(device, code, "main", subgroupSize);
        if (subgroupSize) f.spi.pNext = &f.sgmp; // fix link
        return createCompute(device, f, layout, cache, subgroupSize);
    };

    // create compute pipelines
    static inline auto createCompute(const api::Device& device, const std::string& path, const api::PipelineLayout& layout, const api::PipelineCache& cache = {}, const uint32_t& subgroupSize = 0u) {
        return createCompute(device, readBinary(path), layout, cache, subgroupSize);
    };

    // general command buffer pipeline barrier
    static inline void commandBarrier(const api::CommandBuffer& cmdBuffer) {
        api::MemoryBarrier memoryBarrier = {};
        memoryBarrier.srcAccessMask = (api::AccessFlagBits::eShaderWrite | api::AccessFlagBits::eTransferWrite | api::AccessFlagBits::eMemoryWrite);
        memoryBarrier.dstAccessMask = (api::AccessFlagBits::eShaderRead | api::AccessFlagBits::eTransferRead | api::AccessFlagBits::eMemoryRead | api::AccessFlagBits::eUniformRead);
        const auto srcStageMask = api::PipelineStageFlagBits::eTransfer | api::PipelineStageFlagBits::eAllGraphics | api::PipelineStageFlagBits::eComputeShader | api::PipelineStageFlagBits::eRayTracingShaderNV | api::PipelineStageFlagBits::eAccelerationStructureBuildNV;
        const auto dstStageMask = srcStageMask;
        cmdBuffer.pipelineBarrier(srcStageMask, dstStageMask, {}, { memoryBarrier }, {}, {});
    };

	// create secondary command buffers for batching compute invocations
	static inline auto createCommandBuffer(const api::Device& device, const api::CommandPool& cmdPool, bool secondary = true, bool once = true) {
        api::CommandBuffer cmdBuffer = {};

        api::CommandBufferAllocateInfo cmdi = api::CommandBufferAllocateInfo{};
		cmdi.commandPool = cmdPool;
		cmdi.level = (secondary ? api::CommandBufferLevel::eSecondary : api::CommandBufferLevel::ePrimary);
		cmdi.commandBufferCount = 1;
        cmdBuffer = (device.allocateCommandBuffers(cmdi))[0];

        api::CommandBufferInheritanceInfo inhi = api::CommandBufferInheritanceInfo{};
		inhi.pipelineStatistics = api::QueryPipelineStatisticFlagBits::eComputeShaderInvocations;

        api::CommandBufferBeginInfo bgi = api::CommandBufferBeginInfo{};
		bgi.flags = {};
		bgi.flags = once ? api::CommandBufferUsageFlagBits::eOneTimeSubmit : api::CommandBufferUsageFlagBits::eSimultaneousUse;
		bgi.pInheritanceInfo = secondary ? &inhi : nullptr;
        cmdBuffer.begin(bgi);

		return cmdBuffer;
	};

	// add dispatch in command buffer (with default pipeline barrier)
	static inline api::Result cmdDispatch(const api::CommandBuffer& cmd, const api::Pipeline& pipeline, uint32_t x = 1, uint32_t y = 1, uint32_t z = 1, bool barrier = true) {
        cmd.bindPipeline(api::PipelineBindPoint::eCompute, pipeline);
        cmd.dispatch(x, y, z);
		if (barrier) {
			commandBarrier(cmd); // put shader barrier
		}
		return api::Result::eSuccess;
	};

	// low level copy command between (prefer for host and device)
	static inline api::Result cmdCopyBufferL(const api::CommandBuffer& cmd, const api::Buffer& srcBuffer, const api::Buffer& dstBuffer, const std::vector<api::BufferCopy>& regions, std::function<void(api::CommandBuffer)> barrierFn = commandBarrier) {
		if (srcBuffer && dstBuffer && regions.size() > 0) {
			api::CommandBuffer(cmd).copyBuffer(srcBuffer, dstBuffer, regions); barrierFn(cmd); // put copy barrier
		};
		return api::Result::eSuccess;
	};


	// short data set with command buffer (alike push constant)
	template<class T>
	static inline api::Result cmdUpdateBuffer(const api::CommandBuffer& cmd, const api::Buffer& dstBuffer, const api::DeviceSize& offset, const std::vector<T>& data) {
		api::CommandBuffer(cmd).updateBuffer(dstBuffer, offset, data);
		//updateCommandBarrier(cmd);
		return api::Result::eSuccess;
	};

	// short data set with command buffer (alike push constant)
	template<class T>
	static inline api::Result cmdUpdateBuffer(const api::CommandBuffer& cmd, const api::Buffer& dstBuffer, const api::DeviceSize& offset, const api::DeviceSize& size, const T* data = nullptr) {
		api::CommandBuffer(cmd).updateBuffer(dstBuffer, offset, size, data);
		//updateCommandBarrier(cmd);
		return api::Result::eSuccess;
	};


	// template function for fill buffer by constant value
	// use for create repeat variant
	template<uint32_t Rv>
	static inline api::Result cmdFillBuffer(const api::CommandBuffer& cmd, const api::Buffer& dstBuffer, const api::DeviceSize& size = 0xFFFFFFFF, const api::DeviceSize& offset = 0) {
		api::CommandBuffer(cmd).fillBuffer(api::Buffer(dstBuffer), offset, size, Rv);
		//updateCommandBarrier(cmd);
		return api::Result::eSuccess;
	};


	// submit command (with async wait)
    // TODO: return api::Result 
	static inline void submitCmd(const api::Device& device, const api::Queue& queue, const std::vector<api::CommandBuffer>& cmds, api::SubmitInfo smbi = {}) {
		// no commands 
		if (cmds.size() <= 0) return;
		smbi.commandBufferCount = cmds.size();
		smbi.pCommandBuffers = (api::CommandBuffer*)cmds.data();

        api::Fence fence = {}; api::FenceCreateInfo fin = api::FenceCreateInfo{};
        device.createFence(&fin, nullptr, &fence);
        queue.submit(smbi, fence);
        device.waitForFences(1, &fence, true, INT64_MAX);
        device.destroyFence(fence, nullptr);
	};

	// once submit command buffer
	static inline void submitOnce(const api::Device& device, const api::Queue& queue, const api::CommandPool& cmdPool, const std::function<void(api::CommandBuffer&)>& cmdFn = {}, const api::SubmitInfo& smbi = {}) {
		auto cmdBuf = createCommandBuffer(device, cmdPool, false); cmdFn(cmdBuf); cmdBuf.end();
		submitCmd(device, queue, { cmdBuf }); device.freeCommandBuffers(cmdPool, 1, &cmdBuf); // free that command buffer
	};

	// submit command (with async wait)
	static inline void submitCmdAsync(const api::Device& device, const api::Queue& queue, const std::vector<api::CommandBuffer>& cmds, const std::function<void()>& asyncCallback = {}, api::SubmitInfo smbi = {}) {
		// no commands 
		if (cmds.size() <= 0) return;
		smbi.commandBufferCount = cmds.size();
		smbi.pCommandBuffers = (api::CommandBuffer*)cmds.data();

        api::Fence fence = {}; api::FenceCreateInfo fin = api::FenceCreateInfo{};
        device.createFence(&fin, nullptr, &fence);
        queue.submit(1, (const api::SubmitInfo*)& smbi, fence);
        device.waitForFences(1, &fence, true, INT64_MAX);
        device.destroyFence(fence, nullptr);
		if (asyncCallback) asyncCallback();
	};

	// once submit command buffer
	static inline void submitOnceAsync(const api::Device& device, const api::Queue& queue, const api::CommandPool& cmdPool, const std::function<void(api::CommandBuffer&)>& cmdFn = {}, const std::function<void(api::CommandBuffer)>& asyncCallback = {}, const api::SubmitInfo& smbi = {}) {
		auto cmdBuf = createCommandBuffer(device, cmdPool, false); cmdFn(cmdBuf); cmdBuf.end();
		submitCmdAsync(device, queue, { cmdBuf }, [&]() {
			asyncCallback(cmdBuf); // call async callback
            device.freeCommandBuffers(cmdPool, 1, &cmdBuf); // free that command buffer
		});
	};

	template <class T> static inline auto makeVector(const T * ptr, const size_t& size = 1) { std::vector<T>v(size); memcpy(v.data(), ptr, strided<T>(size)); return v; };

	// create fence function
	static inline api::Fence createFence(const api::Device& device, const bool& signaled = true) {
		api::FenceCreateInfo info = {};
		if (signaled) info.setFlags(api::FenceCreateFlagBits::eSignaled);
		return api::Device(device).createFence(info);
	};

};
