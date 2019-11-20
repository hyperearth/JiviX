#pragma once

#include "../lib/core.hpp"
#include "../API/memory.hpp"
#include <algorithm>
#include <fstream>
#include <iostream>
#include <functional>

namespace lancer {

    // TODO: planned utils for simplify Vulkan API 

    template <typename T>
    static inline auto sgn(const T& val) { return (T(0) < val) - (val < T(0)); }

    template<class T = uint64_t>
    static inline T tiled(const T& sz, const T& gmaxtile) {
        // return (int32_t)ceil((double)sz / (double)gmaxtile);
        return sz <= 0 ? 0 : (sz / gmaxtile + sgn(sz % gmaxtile));
    }

    template <class T>
    static inline auto strided(const size_t& sizeo) { return sizeof(T) * sizeo; }

    // read binary (for SPIR-V)
    static inline auto readBinary(const std::string& filePath ) {
        std::ifstream file(filePath, std::ios::in | std::ios::binary | std::ios::ate);
        std::vector<uint32_t> data = {};
        if (file.is_open()) {
            std::streampos size = file.tellg();
            data.resize(tiled(size_t(size), sizeof(uint32_t)));
            file.seekg(0, std::ios::beg);
            file.read((char *)data.data(), size);
            file.close();
        } else {
            std::cerr << "Failure to open " + filePath << std::endl;
        }
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
    static inline auto createShaderModuleIntrusive(const api::Device& device, const std::vector<uint32_t>& code, api::ShaderModule& hndl) {
        return (hndl = device.createShaderModule(makeShaderModuleInfo(code)));
    };

    static inline auto createShaderModule(const api::Device& device, const std::vector<uint32_t>& code) {
        auto sm = api::ShaderModule{}; return createShaderModuleIntrusive(device, code, sm); return sm;
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
    static inline auto&& makeComputePipelineStageInfo(const api::Device& device, const std::vector<uint32_t>& code, const char * entry = "main", const uint32_t& subgroupSize = 0u) {
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
        memoryBarrier.srcAccessMask = (api::AccessFlagBits::eTransferWrite); 
        memoryBarrier.dstAccessMask = (api::AccessFlagBits::eShaderRead | api::AccessFlagBits::eTransferRead | api::AccessFlagBits::eUniformRead);
        cmdBuffer.pipelineBarrier(
            //api::PipelineStageFlagBits::eTransfer | api::PipelineStageFlagBits::eComputeShader,
            api::PipelineStageFlagBits::eComputeShader,
            api::PipelineStageFlagBits::eTransfer | api::PipelineStageFlagBits::eComputeShader, {}, { memoryBarrier }, {}, {});
    };


	// create secondary command buffers for batching compute invocations
	static inline auto createCommandBuffer(api::Device device, api::CommandPool cmdPool, bool secondary = true, bool once = true) {
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
	static inline api::Result cmdDispatch(api::CommandBuffer cmd, api::Pipeline pipeline, uint32_t x = 1, uint32_t y = 1, uint32_t z = 1, bool barrier = true) {
        cmd.bindPipeline(api::PipelineBindPoint::eCompute, pipeline);
        cmd.dispatch(x, y, z);
		if (barrier) {
			commandBarrier(cmd); // put shader barrier
		}
		return api::Result::eSuccess;
	};

	// low level copy command between (prefer for host and device)
	static inline api::Result cmdCopyBufferL(api::CommandBuffer cmd, api::Buffer srcBuffer, api::Buffer dstBuffer, const std::vector<api::BufferCopy>& regions, std::function<void(api::CommandBuffer)> barrierFn = commandBarrier) {
		if (srcBuffer && dstBuffer && regions.size() > 0) {
			api::CommandBuffer(cmd).copyBuffer(srcBuffer, dstBuffer, regions); barrierFn(cmd); // put copy barrier
		};
		return api::Result::eSuccess;
	};


	// short data set with command buffer (alike push constant)
	template<class T>
	static inline api::Result cmdUpdateBuffer(api::CommandBuffer cmd, api::Buffer dstBuffer, api::DeviceSize offset, const std::vector<T>& data) {
		api::CommandBuffer(cmd).updateBuffer(dstBuffer, offset, data);
		//updateCommandBarrier(cmd);
		return api::Result::eSuccess;
	};

	// short data set with command buffer (alike push constant)
	template<class T>
	static inline api::Result cmdUpdateBuffer(api::CommandBuffer cmd, api::Buffer dstBuffer, api::DeviceSize offset, api::DeviceSize size, const T* data) {
		api::CommandBuffer(cmd).updateBuffer(dstBuffer, offset, size, data);
		//updateCommandBarrier(cmd);
		return api::Result::eSuccess;
	};


	// template function for fill buffer by constant value
	// use for create repeat variant
	template<uint32_t Rv>
	static inline api::Result cmdFillBuffer(api::CommandBuffer cmd, api::Buffer dstBuffer, api::DeviceSize size = 0xFFFFFFFF, api::DeviceSize offset = 0) {
		api::CommandBuffer(cmd).fillBuffer(api::Buffer(dstBuffer), offset, size, Rv);
		//updateCommandBarrier(cmd);
		return api::Result::eSuccess;
	};


	// submit command (with async wait)
	static inline void submitCmd(api::Device device, api::Queue queue, std::vector<api::CommandBuffer> cmds, api::SubmitInfo smbi = {}) {
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
	static inline void submitOnce(api::Device device, api::Queue queue, api::CommandPool cmdPool, std::function<void(api::CommandBuffer)> cmdFn = {}, api::SubmitInfo smbi = {}) {
		auto cmdBuf = createCommandBuffer(device, cmdPool, false); cmdFn(cmdBuf); cmdBuf.end();
		submitCmd(device, queue, { cmdBuf }); device.freeCommandBuffers(cmdPool, 1, &cmdBuf); // free that command buffer
	};

	// submit command (with async wait)
	static inline void submitCmdAsync(api::Device device, api::Queue queue, std::vector<api::CommandBuffer> cmds, std::function<void()> asyncCallback = {}, api::SubmitInfo smbi = {}) {
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
	static inline void submitOnceAsync(api::Device device, api::Queue queue, api::CommandPool cmdPool, std::function<void(api::CommandBuffer)> cmdFn = {}, std::function<void(api::CommandBuffer)> asyncCallback = {}, api::SubmitInfo smbi = {}) {
		auto cmdBuf = createCommandBuffer(device, cmdPool, false); cmdFn(cmdBuf); cmdBuf.end();
		submitCmdAsync(device, queue, { cmdBuf }, [&]() {
			asyncCallback(cmdBuf); // call async callback
            device.freeCommandBuffers(cmdPool, 1, &cmdBuf); // free that command buffer
		});
	};

	template <class T> static inline auto makeVector(const T * ptr, size_t size = 1) { std::vector<T>v(size); memcpy(v.data(), ptr, strided<T>(size)); return v; };

	// create fence function
	static inline api::Fence createFence(api::Device device, bool signaled = true) {
		api::FenceCreateInfo info = {};
		if (signaled) info.setFlags(api::FenceCreateFlagBits::eSignaled);
		return api::Device(device).createFence(info);
	};


    // TODO: dedicated class file 
    class RenderpassMaker : public std::enable_shared_from_this<RenderpassMaker> {
        public:
            RenderpassMaker() {}

            inline api::AttachmentDescription& GetAttachmentDescription() { return attachmentDescriptions.back(); };
            inline api::SubpassDescription& GetSubpassDescription() { return subpassDescriptions.back(); };
            inline api::SubpassDependency& GetSubpassDependency() { return subpassDependencies.back(); };

            inline const api::AttachmentDescription& GetAttachmentDescription() const { return attachmentDescriptions.back(); };
            inline const api::SubpassDescription& GetSubpassDescription() const { return subpassDescriptions.back(); };
            inline const api::SubpassDependency& GetSubpassDependency() const { return subpassDependencies.back(); };


            /// Begin an attachment description.
            /// After this you can call attachment* many times
            inline std::shared_ptr<RenderpassMaker>&& attachmentBegin(api::Format format) {
                api::AttachmentDescription desc{{}, format};
                s.attachmentDescriptions.push_back(desc);
                return shared_from_this();
            };


            inline std::shared_ptr<RenderpassMaker>&& attachmentFlags(api::AttachmentDescriptionFlags value) { s.attachmentDescriptions.back().flags = value; return shared_from_this(); };
            inline std::shared_ptr<RenderpassMaker>&& attachmentFormat(api::Format value) { s.attachmentDescriptions.back().format = value; return shared_from_this(); };
            inline std::shared_ptr<RenderpassMaker>&& attachmentSamples(api::SampleCountFlagBits value) { s.attachmentDescriptions.back().samples = value; return shared_from_this(); };
            inline std::shared_ptr<RenderpassMaker>&& attachmentLoadOp(api::AttachmentLoadOp value) { s.attachmentDescriptions.back().loadOp = value; return shared_from_this(); };
            inline std::shared_ptr<RenderpassMaker>&& attachmentStoreOp(api::AttachmentStoreOp value) { s.attachmentDescriptions.back().storeOp = value; return shared_from_this(); };
            inline std::shared_ptr<RenderpassMaker>&& attachmentStencilLoadOp(api::AttachmentLoadOp value) { s.attachmentDescriptions.back().stencilLoadOp = value; return shared_from_this(); };
            inline std::shared_ptr<RenderpassMaker>&& attachmentStencilStoreOp(api::AttachmentStoreOp value) { s.attachmentDescriptions.back().stencilStoreOp = value; return shared_from_this(); };
            inline std::shared_ptr<RenderpassMaker>&& attachmentInitialLayout(api::ImageLayout value) { s.attachmentDescriptions.back().initialLayout = value; return shared_from_this(); };
            inline std::shared_ptr<RenderpassMaker>&& attachmentFinalLayout(api::ImageLayout value) { s.attachmentDescriptions.back().finalLayout = value; return shared_from_this(); };

            /// Start a subpass description.
            /// After this you can can call subpassColorAttachment many times
            /// and subpassDepthStencilAttachment once.
            inline std::shared_ptr<RenderpassMaker>&& subpassBegin(api::PipelineBindPoint bp) {
                api::SubpassDescription desc{};
                desc.pipelineBindPoint = bp;
                s.subpassDescriptions.push_back(desc);
                return shared_from_this();
            }

            inline std::shared_ptr<RenderpassMaker>&& subpassColorAttachment(api::ImageLayout layout, uint32_t attachment) {
                api::SubpassDescription &subpass = s.subpassDescriptions.back();
                auto *p = getAttachmentReference();
                p->layout = layout;
                p->attachment = attachment;
                if (subpass.colorAttachmentCount == 0) {
                    subpass.pColorAttachments = p;
                }
                subpass.colorAttachmentCount++;
                return shared_from_this();
            }

            inline std::shared_ptr<RenderpassMaker>&& subpassDepthStencilAttachment(api::ImageLayout layout, uint32_t attachment) {
                api::SubpassDescription &subpass = s.subpassDescriptions.back();
                auto *p = getAttachmentReference();
                p->layout = layout;
                p->attachment = attachment;
                subpass.pDepthStencilAttachment = p;
                return shared_from_this();
            }

            inline api::RenderPass create(const api::Device &device) const {
                api::RenderPassCreateInfo renderPassInfo{};
                renderPassInfo.attachmentCount = (uint32_t)s.attachmentDescriptions.size();
                renderPassInfo.pAttachments = s.attachmentDescriptions.data();
                renderPassInfo.subpassCount = (uint32_t)s.subpassDescriptions.size();
                renderPassInfo.pSubpasses = s.subpassDescriptions.data();
                renderPassInfo.dependencyCount = (uint32_t)s.subpassDependencies.size();
                renderPassInfo.pDependencies = s.subpassDependencies.data();
                return device.createRenderPass(renderPassInfo);
            }

            inline std::shared_ptr<RenderpassMaker>&& dependencyBegin(uint32_t srcSubpass, uint32_t dstSubpass) {
                api::SubpassDependency desc{};
                desc.srcSubpass = srcSubpass;
                desc.dstSubpass = dstSubpass;
                s.subpassDependencies.push_back(desc);
                return shared_from_this();
            }

            inline std::shared_ptr<RenderpassMaker>&& dependencySrcSubpass(uint32_t value) { s.subpassDependencies.back().srcSubpass = value; return shared_from_this(); };
            inline std::shared_ptr<RenderpassMaker>&& dependencyDstSubpass(uint32_t value) { s.subpassDependencies.back().dstSubpass = value; return shared_from_this(); };
            inline std::shared_ptr<RenderpassMaker>&& dependencySrcStageMask(api::PipelineStageFlags value) { s.subpassDependencies.back().srcStageMask = value; return shared_from_this(); };
            inline std::shared_ptr<RenderpassMaker>&& dependencyDstStageMask(api::PipelineStageFlags value) { s.subpassDependencies.back().dstStageMask = value; return shared_from_this(); };
            inline std::shared_ptr<RenderpassMaker>&& dependencySrcAccessMask(api::AccessFlags value) { s.subpassDependencies.back().srcAccessMask = value; return shared_from_this(); };
            inline std::shared_ptr<RenderpassMaker>&& dependencyDstAccessMask(api::AccessFlags value) { s.subpassDependencies.back().dstAccessMask = value; return shared_from_this(); };
            inline std::shared_ptr<RenderpassMaker>&& dependencyDependencyFlags(api::DependencyFlags value) { s.subpassDependencies.back().dependencyFlags = value; return shared_from_this(); };
            
        private:
            constexpr static int max_refs = 64;

            inline api::AttachmentReference *getAttachmentReference() {
                return (s.num_refs < max_refs) ? &s.attachmentReferences[s.num_refs++] : nullptr;
            };
            
            struct State {
                std::vector<api::AttachmentDescription> attachmentDescriptions;
                std::vector<api::SubpassDescription> subpassDescriptions;
                std::vector<api::SubpassDependency> subpassDependencies;
                std::array<api::AttachmentReference, max_refs> attachmentReferences;
                int num_refs = 0;
                bool ok_ = false;
            };

            State s;
    };





};
