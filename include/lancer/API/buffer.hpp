#pragma once

#include "../lib/core.hpp"
#include "../API/memory.hpp"

namespace lancer {
    // Vookoo-Like 
    class Buffer : public std::enable_shared_from_this<Buffer> {
        protected: 
            std::shared_ptr<Device> device = {};
            std::shared_ptr<Allocation> allocation = {}; // least allocation, may be vector 
            api::Buffer* lastbuf = nullptr;
            api::BufferCreateInfo bfc = {};

        public: 
            Buffer(const std::shared_ptr<Device>& device, api::Buffer* lastbuf = nullptr, api::BufferCreateInfo bfc = api::BufferCreateInfo().setSharingMode(vk::SharingMode::eExclusive)) : lastbuf(lastbuf), bfc(bfc), device(device) {
            };

            ~Buffer(){
            }; // Here will notification about free memory

            // Get original Vulkan link 
            vk::Buffer& Least() { return *lastbuf; };
            operator vk::Buffer&() { return *lastbuf; };
            const vk::Buffer& Least() const { return *lastbuf; };
            operator const vk::Buffer&() const { return *lastbuf; };


            //  
            std::shared_ptr<Buffer>& QueueFamilyIndices(const std::vector<uint32_t>& indices = {}) {
                bfc.queueFamilyIndexCount = indices.size();
                bfc.pQueueFamilyIndices = indices.data();
                return shared_from_this(); };

            // Link Editable Buffer 
            std::shared_ptr<Buffer>& Link(api::Buffer& buf) { lastbuf = &buf; 
                return shared_from_this(); };

            // 
            std::shared_ptr<Buffer>& Allocate(const std::shared_ptr<Allocator>& mem) {
                mem->AllocateForBuffer(lastbuf,allocation,bfc); 
                return shared_from_this(); };

            // 
            std::shared_ptr<Buffer>& Create() { // 
                *lastbuf = device->Least().createBuffer(bfc);
                return shared_from_this(); };

            // Create With Buffer View 
            std::shared_ptr<Buffer>& CreateView(api::BufferView& bfv, const api::Format& format, const uintptr_t& offset = 0u, const size_t& size = 16u) {
                (bfv = allocation->GetDevice()->Least().createBufferView(api::BufferViewCreateInfo{{}, *lastbuf, format, offset, size}));
                return shared_from_this(); };

            // Create With Region
            std::shared_ptr<Buffer>& CreateRegion(api::DescriptorBufferInfo& reg, const uintptr_t& offset = 0u, const size_t& size = 16u) {
                (reg = api::DescriptorBufferInfo{*lastbuf, offset, size});
                return shared_from_this(); };
    };


    // TODO: planned to write buffer-based vectors (import from RadX)

};
