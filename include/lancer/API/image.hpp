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
            std::shared_ptr<Sampler>&& Link(api::Sampler* smi) { sampler = smi; return shared_from_this(); };

            // Editable Fields 
            api::SamplerCreateInfo& GetCreateInfo() { return smc; };
            const api::SamplerCreateInfo& GetCreateInfo() const { return smc; };

            // Get original Vulkan link 
            api::Sampler& Least() { return *sampler; };
            const api::Sampler& Least() const { return *sampler; };

            // Get original Vulkan link 
            operator api::Sampler&() { return *sampler; };
            operator const api::Sampler&() const { return *sampler; };

            // Create Sampler 
            std::shared_ptr<Sampler>&& Create(const api::Format& format = api::Format::eR8G8B8A8Unorm, const uint32_t&w = 1u) {
                *sampler = device->Least().createSampler(smc);
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
            api::Image& Least() { return *lastimg; };
            const api::Image& Least() const { return *lastimg; };

            // Get original Vulkan link 
            operator api::Image&() { return *lastimg; };
            operator const api::Image&() const { return *lastimg; };

            // Get Image Layout for 
            const api::ImageLayout& GetOriginLayout() const { return originLayout; };
            const api::ImageLayout& GetTargetLayout() const { return targetLayout; };
            const api::ImageViewCreateInfo& GetViewCreateInfo() const { return imv; };
            const api::ImageCreateInfo& GetCreateInfo() const { return imc; };
            api::ImageLayout& GetOriginLayout() { return originLayout; };
            api::ImageLayout& GetTargetLayout() { return targetLayout; };
            api::ImageViewCreateInfo& GetViewCreateInfo() { return imv; };
            api::ImageCreateInfo& GetCreateInfo() { return imc; };

            // 
            std::shared_ptr<Image>&& ImageSubresourceRange(const api::ImageSubresourceRange& subres = {}) {
                sbr = subres; // For ImageView create 
                return shared_from_this(); };

            // 
            std::shared_ptr<Image>&& MipLevels(const uint32_t& mipLevels = 1) {
                imc.mipLevels = mipLevels;
                return shared_from_this(); };

            // 
            std::shared_ptr<Image>&& SampleCount(const api::SampleCountFlagBits& samples = api::SampleCountFlagBits::e1) {
                imc.samples = samples;
                return shared_from_this(); };

            // 
            std::shared_ptr<Image>&& Tiling(const api::ImageTiling& tiling = api::ImageTiling::eOptimal) {
                imc.tiling = tiling;
                return shared_from_this(); };

            // 
            std::shared_ptr<Image>&& Usage(const api::ImageUsageFlags& usage = api::ImageUsageFlagBits::eStorage) {
                imc.usage = usage;
                return shared_from_this(); };

            // 
            std::shared_ptr<Image>&& QueueFamilyIndices(const std::vector<uint32_t>& indices = {}) {
                imc.queueFamilyIndexCount = indices.size();
                imc.pQueueFamilyIndices = indices.data();
                return shared_from_this(); };

            // 
            std::shared_ptr<Image>&& Link(api::Image* img) { lastimg = img; return shared_from_this(); };
            std::shared_ptr<Image>&& Allocate(const std::shared_ptr<Allocator>& mem, const uintptr_t& ptx = 0u) {
                mem->AllocateForImage(lastimg,allocation=mem->CreateAllocation(),imc,ptx);
                return shared_from_this(); };

            // Create 1D "Canvas" 
            std::shared_ptr<Image>&& Create1D(const api::Format& format = api::Format::eR8G8B8A8Unorm, const uint32_t&w = 1u) {
                imc.imageType = api::ImageType::e1D;
                imc.extent = {w,1u,1u};
                imc.arrayLayers = 1u;
                imc.format = format;
                *lastimg = device->Least().createImage(imc);
                return shared_from_this(); };

            // Create 2D "Canvas" 
            std::shared_ptr<Image>&& Create2D(const api::Format& format = api::Format::eR8G8B8A8Unorm, const uint32_t&w = 1u, const uint32_t&h = 1u) {
                imc.imageType = api::ImageType::e2D;
                imc.extent = {w,h,1u};
                imc.arrayLayers = 1u;
                imc.format = format;
                *lastimg = device->Least().createImage(imc);
                return shared_from_this(); };

            // Create 3D "Canvas" 
            std::shared_ptr<Image>&& Create3D(const api::Format& format = api::Format::eR8G8B8A8Unorm, const uint32_t&w = 1u, const uint32_t&h = 1u, const uint32_t&d = 1u) {
                imc.imageType = api::ImageType::e3D;
                imc.extent = {w,h,d};
                imc.arrayLayers = d;
                imc.format = format;
                *lastimg = device->Least().createImage(imc);
                return shared_from_this(); };

            // Create ImageView 
            std::shared_ptr<Image>&& CreateImageView(api::ImageView* imgv, const api::ImageViewType& viewType = api::ImageViewType::e2D, const api::Format& format = api::Format::eUndefined, const api::ComponentMapping& compmap = DEFAULT_COMPONENTS){
                imv.image = *lastimg;
                imv.viewType = viewType;
                imv.format = format!=api::Format::eUndefined?format:imc.format;
                imv.components = compmap;
                imv.subresourceRange = sbr;
                *imgv = (device->Least().createImageView(imv)); lastimv = imgv;
                return shared_from_this(); };

            // Stub for write both in descriptor
            std::shared_ptr<Image>&& CreateImageInfo(api::DescriptorImageInfo* imd){
                imd->imageView = *lastimv;
                imd->imageLayout = targetLayout;
                return shared_from_this(); };

            // TODO: add and write into command buffer ImageLayout switch (from originLayout to targetLayout)
            // TODO: write targetLayout into api::DescriptorImageInfo 
            // TODO: create for api::DescriptorImageInfo 
    };

};
