#pragma once

#include "../lib/core.hpp"
#include "../API/memory.hpp"

namespace lancer {

    #define DEFAULT_COMPONENTS api::ComponentMapping{api::ComponentSwizzle::eR,api::ComponentSwizzle::eG,api::ComponentSwizzle::eB,api::ComponentSwizzle::eA}
    #define DEFAULT_IMC api::ImageCreateInfo().setSharingMode(api::SharingMode::eExclusive).setInitialLayout(api::ImageLayout::eUndefined)

    class Sampler : public std::enable_shared_from_this<Sampler> {
        protected: 
            std::shared_ptr<Device> device = {};
            api::Sampler* sampler = nullptr; // least allocation, may be vector 
            api::SamplerCreateInfo smc = {};

        public: 
            ~Sampler(){};
             Sampler(const std::shared_ptr<Device>& device, api::Sampler* sampler = nullptr, const api::SamplerCreateInfo& smc = {}) : device(device), sampler(sampler), smc(smc) {
             };

            // 
            inline std::shared_ptr<Sampler>&& link(api::Sampler* smi) { sampler = smi; return shared_from_this(); };

            // Editable Fields 
            inline api::SamplerCreateInfo& getCreateInfo() { return smc; };
            inline const api::SamplerCreateInfo& getCreateInfo() const { return smc; };

            // Get original Vulkan link 
            inline api::Sampler& least() { return *sampler; };
            inline const api::Sampler& least() const { return *sampler; };

            // Get original Vulkan link 
            operator api::Sampler&() { return *sampler; };
            operator const api::Sampler&() const { return *sampler; };

            // Create Sampler 
            inline std::shared_ptr<Sampler>&& create(const api::Format& format = api::Format::eR8G8B8A8Unorm, const uint32_t&w = 1u) {
                *sampler = device->least().createSampler(smc);
                return shared_from_this(); };

            // Stub for write both in descriptor
            inline std::shared_ptr<Sampler>&& writeForDIF(api::DescriptorImageInfo* imd){
                imd->sampler = *sampler;
                return shared_from_this(); };
    };


    // Vookoo-Like 
    class Image : public std::enable_shared_from_this<Image> {
        protected: 
            std::shared_ptr<Device> device = {};
            std::shared_ptr<Allocation> allocation = {}; // least allocation, may be vector 
            api::Image* lastimg = nullptr; // least allocation, may be vector 
            api::ImageView* lastimv = nullptr;
            api::ImageViewCreateInfo imv = {};
            api::ImageCreateInfo imc = {};
            api::ImageLayout originLayout = api::ImageLayout::eUndefined;
            api::ImageLayout targetLayout = api::ImageLayout::eGeneral;
            api::ImageSubresourceRange sbr = { api::ImageAspectFlagBits::eColor, 0u, 1u, 0u, 1u };

        public: 
             Image(const std::shared_ptr<Device>& device, api::Image* lastimg = nullptr, api::ImageCreateInfo imc = DEFAULT_IMC) : lastimg(lastimg),imc(imc),device(device) { imc.extent = {1u,1u,1u}; };
            ~Image(){}; // Here will notification about free memory

            // 
            inline api::Image& least() { return *lastimg; };
            inline const api::Image& least() const { return *lastimg; };

            // Get original Vulkan link 
            operator api::Image&() { return *lastimg; };
            operator const api::Image&() const { return *lastimg; };

            // Get Image Layout for 
            inline const api::ImageLayout& getOriginLayout() const { return originLayout; };
            inline const api::ImageLayout& getTargetLayout() const { return targetLayout; };
            inline const api::ImageViewCreateInfo& getViewCreateInfo() const { return imv; };
            inline const api::ImageCreateInfo& getCreateInfo() const { return imc; };
            inline api::ImageLayout& getOriginLayout() { return originLayout; };
            inline api::ImageLayout& getTargetLayout() { return targetLayout; };
            inline api::ImageViewCreateInfo& getViewCreateInfo() { return imv; };
            inline api::ImageCreateInfo& getCreateInfo() { return imc; };




            // transition texture layout
            inline auto imageBarrier(const api::CommandBuffer& cmd) {
                api::Result result = api::Result::eSuccess; // planned to complete
                if (originLayout == targetLayout) return shared_from_this(); // no need transfering more

                api::ImageMemoryBarrier imageMemoryBarriers = {};
                imageMemoryBarriers.srcQueueFamilyIndex = ~0U;
                imageMemoryBarriers.dstQueueFamilyIndex = ~0U;
                imageMemoryBarriers.oldLayout = originLayout;
                imageMemoryBarriers.newLayout = targetLayout;
                imageMemoryBarriers.subresourceRange = sbr;
                imageMemoryBarriers.image = *lastimg;

                // Put barrier on top
                const auto  srcStageMask = api::PipelineStageFlags{ api::PipelineStageFlagBits::eBottomOfPipe };
                const auto  dstStageMask = api::PipelineStageFlags{ api::PipelineStageFlagBits::eTopOfPipe };
                const auto  dependencyFlags = api::DependencyFlags{};
                      auto  srcMask = api::AccessFlags{}, dstMask = api::AccessFlags{};

                typedef api::ImageLayout il;
                typedef api::AccessFlagBits afb;

                // Is it me, or are these the same?
                switch (originLayout) {
                    case il::eUndefined: break;
                    case il::eGeneral: srcMask = afb::eTransferWrite; break;
                    case il::eColorAttachmentOptimal: srcMask = afb::eColorAttachmentWrite; break;
                    case il::eDepthStencilAttachmentOptimal: srcMask = afb::eDepthStencilAttachmentWrite; break;
                    case il::eDepthStencilReadOnlyOptimal: srcMask = afb::eDepthStencilAttachmentRead; break;
                    case il::eShaderReadOnlyOptimal: srcMask = afb::eShaderRead; break;
                    case il::eTransferSrcOptimal: srcMask = afb::eTransferRead; break;
                    case il::eTransferDstOptimal: srcMask = afb::eTransferWrite; break;
                    case il::ePreinitialized: srcMask = afb::eTransferWrite | afb::eHostWrite; break;
                    case il::ePresentSrcKHR: srcMask = afb::eMemoryRead; break;
                };

                switch (targetLayout) {
                    case il::eUndefined: break;
                    case il::eGeneral: dstMask = afb::eTransferWrite; break;
                    case il::eColorAttachmentOptimal: dstMask = afb::eColorAttachmentWrite; break;
                    case il::eDepthStencilAttachmentOptimal: dstMask = afb::eDepthStencilAttachmentWrite; break;
                    case il::eDepthStencilReadOnlyOptimal: dstMask = afb::eDepthStencilAttachmentRead; break;
                    case il::eShaderReadOnlyOptimal: dstMask = afb::eShaderRead; break;
                    case il::eTransferSrcOptimal: dstMask = afb::eTransferRead; break;
                    case il::eTransferDstOptimal: dstMask = afb::eTransferWrite; break;
                    case il::ePreinitialized: dstMask = afb::eTransferWrite; break;
                    case il::ePresentSrcKHR: dstMask = afb::eMemoryRead; break;
                };

                // assign access masks
                imageMemoryBarriers.srcAccessMask = srcMask;
                imageMemoryBarriers.dstAccessMask = dstMask;

                // barrier
                cmd.pipelineBarrier(api::PipelineStageFlagBits::eAllCommands, api::PipelineStageFlagBits::eAllCommands, {}, {}, {}, std::array<api::ImageMemoryBarrier, 1>{imageMemoryBarriers});

                //image->_initialLayout = (VkImageLayout)imageMemoryBarriers.newLayout;
                return shared_from_this(); };



            // 
            inline std::shared_ptr<Image>&& imageSubresourceRange(const api::ImageSubresourceRange& subres = {}) {
                sbr = subres; // For ImageView create 
                return shared_from_this(); };

            // 
            inline std::shared_ptr<Image>&& mipLevels(const uint32_t& mipLevels = 1) {
                imc.mipLevels = mipLevels;
                return shared_from_this(); };

            // 
            inline std::shared_ptr<Image>&& sampleCount(const api::SampleCountFlagBits& samples = api::SampleCountFlagBits::e1) {
                imc.samples = samples;
                return shared_from_this(); };

            // 
            inline std::shared_ptr<Image>&& tiling(const api::ImageTiling& tiling = api::ImageTiling::eOptimal) {
                imc.tiling = tiling;
                return shared_from_this(); };

            // 
            inline std::shared_ptr<Image>&& usage(const api::ImageUsageFlags& usage = api::ImageUsageFlagBits::eStorage) {
                imc.usage = usage;
                return shared_from_this(); };

            // 
            inline std::shared_ptr<Image>&& queueFamilyIndices(const std::vector<uint32_t>& indices = {}) {
                imc.queueFamilyIndexCount = indices.size();
                imc.pQueueFamilyIndices = indices.data();
                return shared_from_this(); };

            // 
            inline std::shared_ptr<Image>&& link(api::Image* img) { lastimg = img; return shared_from_this(); };
            inline std::shared_ptr<Image>&& allocate(const std::shared_ptr<Allocator>& mem, const uintptr_t& ptx = 0u) {
                mem->allocateForImage(lastimg,allocation=mem->createAllocation(),imc,ptx);
                return shared_from_this(); };

            // Create 1D "Canvas" 
            inline std::shared_ptr<Image>&& create1D(const api::Format& format = api::Format::eR8G8B8A8Unorm, const uint32_t&w = 1u) {
                imc.imageType = api::ImageType::e1D;
                imc.extent = {w,1u,1u};
                imc.arrayLayers = 1u;
                imc.format = format;
                *lastimg = device->least().createImage(imc);
                return shared_from_this(); };

            // Create 2D "Canvas" 
            inline std::shared_ptr<Image>&& create2D(const api::Format& format = api::Format::eR8G8B8A8Unorm, const uint32_t&w = 1u, const uint32_t&h = 1u) {
                imc.imageType = api::ImageType::e2D;
                imc.extent = {w,h,1u};
                imc.arrayLayers = 1u;
                imc.format = format;
                *lastimg = device->least().createImage(imc);
                return shared_from_this(); };

            // Create 3D "Canvas" 
            inline std::shared_ptr<Image>&& create3D(const api::Format& format = api::Format::eR8G8B8A8Unorm, const uint32_t&w = 1u, const uint32_t&h = 1u, const uint32_t&d = 1u) {
                imc.imageType = api::ImageType::e3D;
                imc.extent = {w,h,d};
                imc.arrayLayers = d;
                imc.format = format;
                *lastimg = device->least().createImage(imc);
                return shared_from_this(); };

            // Create ImageView 
            inline std::shared_ptr<Image>&& createImageView(api::ImageView* imgv, const api::ImageViewType& viewType = api::ImageViewType::e2D, const api::Format& format = api::Format::eUndefined, const api::ComponentMapping& compmap = DEFAULT_COMPONENTS){
                imv.image = *lastimg;
                imv.viewType = viewType;
                imv.format = format!=api::Format::eUndefined?format:imc.format;
                imv.components = compmap;
                imv.subresourceRange = sbr;
                *imgv = (device->least().createImageView(imv)); lastimv = imgv;
                return shared_from_this(); };

            // Stub for write both in descriptor
            inline std::shared_ptr<Image>&& writeForDIF(api::DescriptorImageInfo* imd){
                imd->imageView = *lastimv;
                imd->imageLayout = targetLayout;
                return shared_from_this(); };

            
            // TODO: re-make or improve  api::DescriptorImageInfo  interaction
    };

};
