#pragma once

#include "../lib/core.hpp"
#include "../API/memory.hpp"

namespace lancer {

    #define DEFAULT_COMPONENTS api::ComponentMapping{api::ComponentSwizzle::eR,api::ComponentSwizzle::eG,api::ComponentSwizzle::eB,api::ComponentSwizzle::eA}

    // TODO: Add Sampler Support 

    #define DEFAULT_IMC api::ImageCreateInfo().setSharingMode(api::SharingMode::eExclusive).setInitialLayout(api::ImageLayout::eUndefined)

    // Vookoo-Like 
    class Image : public std::enable_shared_from_this<Image> {
        protected: 
            std::shared_ptr<Device> device = {};
            std::shared_ptr<Allocation> allocation = {}; // least allocation, may be vector 
            api::Image* lastimg = nullptr; // least allocation, may be vector 
            api::ImageCreateInfo imc = {};
            api::ImageView* lastimv = nullptr;
            api::ImageViewCreateInfo imv = {};
            api::ImageLayout originLayout = api::ImageLayout::eUndefined;
            api::ImageLayout targetLayout = api::ImageLayout::eGeneral;
            api::ImageSubresourceRange sbr = { api::ImageAspectFlagBits::eColor, 0u, 1u, 0u, 1u };

        public: 
             Image(const std::shared_ptr<Device>& device, api::Image* lastimg = nullptr, api::ImageCreateInfo imc = DEFAULT_IMC) : lastimg(lastimg),imc(imc),device(device) { imc.extent = {1u,1u,1u}; };
            ~Image(){}; // Here will notification about free memory

            // Get original Vulkan link 
            api::Image& Least() { return *lastimg; };
            operator api::Image&() { return *lastimg; };
            const api::Image& Least() const { return *lastimg; };
            operator const api::Image&() const { return *lastimg; };


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
            std::shared_ptr<Image>&& Allocate(const std::shared_ptr<Allocator>& mem) {
                mem->AllocateForImage(lastimg,allocation,imc);
                return shared_from_this(); };

            // TODO: unify types 
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
