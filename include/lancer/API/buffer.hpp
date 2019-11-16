#pragma once

#include "../lib/core.hpp"
#include "../API/memory.hpp"

namespace lancer {
    // Vookoo-Like 
    class Buffer : public std::enable_shared_from_this<Buffer> {
        protected: 
            // TODO: add device as tool
            api::BufferCreateInfo bfc = {};
            api::Buffer lastbuf = {}; // least allocation, may be vector 
            std::shared_ptr<Allocation> allocation = {}; // least allocation, may be vector 

        public: 
            Buffer(const std::shared_ptr<Device>& device, const size_t& size = 16u, const api::BufferUsageFlags& usage = api::BufferUsageFlagBits::eStorageBuffer) {
                bfc.sharingMode = api::SharingMode::eExclusive;
                bfc.usage = usage;
                bfc.size = size;
            };

            ~Buffer(){ // Here will notification about free memory
                
            };

            void Create(api::Buffer* buf, const std::shared_ptr<Allocator>& mem){
                mem->AllocateForBuffer(buf,allocation,bfc); if (buf) lastbuf = *buf;
            };

            // TODO: create buffer view itself 
            void CreateView(api::BufferView* bfv, const api::Buffer& buf, const api::Format& format, const uintptr_t& offset = 0u, const size_t& size = 16u) {
                auto civ = api::BufferViewCreateInfo{ {}, buf?buf:lastbuf, format, offset, size };
            };

            void CreateRegion(api::DescriptorBufferInfo* reg, const api::Buffer& buf, const uintptr_t& offset = 0u, const size_t& size = 16u) {
                *reg = {buf, offset, size};
            };
    };
};
