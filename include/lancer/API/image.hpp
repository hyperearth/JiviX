#pragma once

#include "../lib/core.hpp"
#include "../API/memory.hpp"

namespace lancer {

    #define DEFAULT_COMPONENTS api::ComponentMapping{api::ComponentSwizzle::eR,api::ComponentSwizzle::eG,api::ComponentSwizzle::eB,api::ComponentSwizzle::eA}
    #define DEFAULT_IMC api::ImageCreateInfo().setSharingMode(api::SharingMode::eExclusive).setInitialLayout(api::ImageLayout::eUndefined)

    class Sampler_T : public std::enable_shared_from_this<Sampler_T> {
        protected: 
            DeviceMaker device = {};
            api::Sampler* sampler = nullptr; // least allocation, may be vector 
            api::SamplerCreateInfo smc = {};

        public: 
            ~Sampler_T(){};
             Sampler_T(const DeviceMaker& device = {}, const api::SamplerCreateInfo& smc = {}, api::Sampler* sampler = nullptr) : device(device), sampler(sampler), smc(smc) {
             };

            // 
            inline SamplerMaker link(api::Sampler* smi) { sampler = smi; return shared_from_this(); };

            // Editable Fields 
            inline api::SamplerCreateInfo& getCreateInfo() { return smc; };
            inline const api::SamplerCreateInfo& getCreateInfo() const { return smc; };

            // Get original Vulkan link 
            inline api::Sampler& least() { return *sampler; };
            inline const api::Sampler least() const { return *sampler; };

            // Get original Vulkan link 
            operator api::Sampler&() { return *sampler; };
            operator const api::Sampler&() const { return *sampler; };

            // Create Sampler 
            inline SamplerMaker create(const api::Format& format = api::Format::eR8G8B8A8Unorm, const uint32_t&w = 1u) {
                *sampler = device->least().createSampler(smc);
                return shared_from_this(); };

            // Stub for write both in descriptor
            inline SamplerMaker writeForDIF(api::DescriptorImageInfo* imd){
                imd->sampler = *sampler;
                return shared_from_this(); };
    };


    // Vookoo-Like 
    class Image_T : public std::enable_shared_from_this<Image_T> {
        protected: 
            DeviceMaker device = {};
            MemoryAllocation allocation = {}; // least allocation, may be vector 
            api::Image* lastimg = nullptr; // least allocation, may be vector 
            api::ImageView* lastimv = nullptr;
            api::ImageViewCreateInfo imv = {};
            api::ImageCreateInfo imc = {};
            api::ImageLayout originLayout = api::ImageLayout::eUndefined;
            api::ImageLayout targetLayout = api::ImageLayout::eGeneral;
            api::ImageSubresourceRange sbr = { api::ImageAspectFlagBits::eColor, 0u, 1u, 0u, 1u };
            bool smartFree = false;

        public: 
             Image_T(const DeviceMaker& device, const api::ImageCreateInfo& imc = DEFAULT_IMC, api::Image* lastimg = nullptr) : lastimg(lastimg),imc(imc),device(device) {
                 if (this->imc.mipLevels < 1u) { this->imc.mipLevels = 1u; }; // fix dirty bug
                 if (this->imc.extent.width < 1u) { this->imc.extent.width = 1u; };
                 if (this->imc.extent.height < 1u) { this->imc.extent.height = 1u; };
                 if (this->imc.extent.depth < 1u) { this->imc.extent.depth = 1u; };
                 if (this->imc.arrayLayers < 1u) { this->imc.arrayLayers = 1u; };
                 if (this->imc.format == api::Format::eUndefined) { this->imc.format = api::Format::eR8G8B8A8Unorm; };
             };
             ~Image_T() {
                 if (smartFree) { allocation->freeImage(shared_from_this()); };
            }; // Here will notification about free memory

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
            inline auto imageBarrier(api::CommandBuffer& cmd) {
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
            inline MemoryAllocation& getAllocation() {
                return this->allocation;
            };

            // 
            const inline MemoryAllocation& getAllocation() const {
                return this->allocation;
            };

            // Memory now can binded 
            inline ImageMaker linkAllocation(const MemoryAllocation& allocation = {}, const vk::BindImageMemoryInfo& bindinf = {}) {
                if (!this->allocation) {
                    this->allocation = allocation;
                    if (lastimg && (*lastimg)) {
                        const auto mem = allocation->getMemory();
                        if (!!mem) { device->least().bindImageMemory2(vk::BindImageMemoryInfo(bindinf).setImage(*lastimg).setMemory(mem).setMemoryOffset(allocation->getMemoryOffset())); };
                    };
                };
                return shared_from_this();
            };

            // 
            inline ImageMaker setImageSubresourceRange(const api::ImageSubresourceRange& subres = {}) {
                sbr = subres; // For ImageView create 
                return shared_from_this(); };

            // 
            inline ImageMaker mipLevels(const uint32_t& mipLevels = 1) {
                imc.mipLevels = mipLevels;
                return shared_from_this(); };

            // 
            inline ImageMaker sampleCount(const api::SampleCountFlagBits& samples = api::SampleCountFlagBits::e1) {
                imc.samples = samples;
                return shared_from_this(); };

            // 
            inline ImageMaker tiling(const api::ImageTiling& tiling = api::ImageTiling::eOptimal) {
                imc.tiling = tiling;
                return shared_from_this(); };

            // 
            inline ImageMaker usage(const api::ImageUsageFlags& usage = api::ImageUsageFlagBits::eStorage) {
                imc.usage = usage;
                return shared_from_this(); };

            // 
            inline ImageMaker queueFamilyIndices(const std::vector<uint32_t>& indices = {}) {
                imc.queueFamilyIndexCount = indices.size();
                imc.pQueueFamilyIndices = indices.data();
                return shared_from_this(); };

            // 
            inline ImageMaker linkImageView(api::ImageView* view = nullptr) { lastimv = view; return shared_from_this(); };
            inline ImageMaker link(api::Image* img = nullptr) { lastimg = img; return shared_from_this(); };
            inline ImageMaker allocate(const MemoryAllocator& mem = {}, const uintptr_t& ptx = 0u) {
                if (!this->allocation) {
                    if (lastimg && (*lastimg)) { // But... Image Already Created...
                        auto req = device->least().getImageMemoryRequirements2(vk::ImageMemoryRequirementsInfo2().setImage(*lastimg));
                        mem->allocateForRequirements(allocation = mem->createAllocation(), req, ptx);
                        device->least().bindImageMemory2(vk::BindImageMemoryInfo().setImage(*lastimg).setMemory(allocation->getMemory()).setMemoryOffset(allocation->getMemoryOffset()));
                    }
                    else {
                        mem->allocateForImage(lastimg, allocation = mem->createAllocation(), imc, ptx);
                    };
                };
                return shared_from_this(); };

            inline ImageMaker allocate(const uintptr_t& ptx = 0u) { return this->allocate(device->getAllocatorPtr(),ptx); };
            inline ImageMaker free() { allocation->smartFree(); smartFree = true; return shared_from_this(); };

            // Create 1D "Canvas" 
            inline ImageMaker create(const api::ImageType& type = api::ImageType::e1D, const api::Format& format = api::Format::eR8G8B8A8Unorm, const uint32_t&w = 1u, const uint32_t&h = 1u, const uint32_t&d = 1u, const vk::BindImageMemoryInfo& bindinf = {}) {
                //api::ImageCreateInfo imv = {};
                imc.imageType = type;
                imc.extent = api::Extent3D{w,h,d};
                imc.arrayLayers = d;
                imc.format = format;

                if (lastimg && !(*lastimg)) { // Create Image Manually
                    *lastimg = device->least().createImage(imc); // Allocator Will Create Image Anyways 
                };

                if (!!allocation) { // Bind Memory When Possible 
                    const auto mem = allocation->getMemory();
                    if (!!mem) { device->least().bindImageMemory2(vk::BindImageMemoryInfo(bindinf).setImage(*lastimg).setMemory(mem).setMemoryOffset(allocation->getMemoryOffset())); };
                };

                return shared_from_this(); };

            inline ImageMaker create1D(const api::Format& format = api::Format::eR8G8B8A8Unorm, const uint32_t&w = 1u, const vk::BindImageMemoryInfo& bindinf = {}) { return this->create(api::ImageType::e1D,format,w,1u,1u,bindinf); };
            inline ImageMaker create2D(const api::Format& format = api::Format::eR8G8B8A8Unorm, const uint32_t&w = 1u, const uint32_t&h = 1u, const vk::BindImageMemoryInfo& bindinf = {}) { return this->create(api::ImageType::e2D,format,w,h,1u,bindinf); };
            inline ImageMaker create3D(const api::Format& format = api::Format::eR8G8B8A8Unorm, const uint32_t&w = 1u, const uint32_t&h = 1u, const uint32_t&d = 1u, const vk::BindImageMemoryInfo& bindinf = {}) { return this->create(api::ImageType::e3D,format,w,h,d,bindinf); };

            // Create ImageView 
            inline ImageMaker createImageView(const api::ImageViewType& viewType = api::ImageViewType::e2D, const api::Format& format = api::Format::eUndefined, const api::ComponentMapping& compmap = DEFAULT_COMPONENTS) {
                imv.image = *this->lastimg;
                imv.viewType = viewType;
                imv.format = format != api::Format::eUndefined ? format : imc.format;
                imv.components = compmap;
                imv.subresourceRange = this->sbr;
                *this->lastimv = this->device->least().createImageView(imv);
                return shared_from_this();
            };

            inline ImageMaker createImageView(api::ImageView* imgv, const api::ImageViewType& viewType = api::ImageViewType::e2D, const api::Format& format = api::Format::eUndefined, const api::ComponentMapping& compmap = DEFAULT_COMPONENTS){
                return this->linkImageView(imgv)->createImageView(viewType, format, compmap); };

            // Stub for write both in descriptor
            inline ImageMaker writeForDIF(api::DescriptorImageInfo* imd){
                imd->imageView = *lastimv;
                imd->imageLayout = targetLayout;
                return shared_from_this(); };

            // TODO: re-make or improve  api::DescriptorImageInfo  interaction
    };

};
