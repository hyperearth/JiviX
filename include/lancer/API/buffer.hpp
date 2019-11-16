#pragma once

#include "../lib/core.hpp"

namespace lancer {
    // Vookoo-Like 
    class Buffer : public std::enable_shared_from_this<Buffer> {
        protected: 
            api::BufferCreateInfo bfc = {};
            api::Buffer lastbuf = {}; // least allocation, may be vector 
            std::shared_ptr<Allocation> allocation = {}; // least allocation, may be vector 

        public: 
            Buffer(const std::shared_ptr<Device>& device, const sizei_t& size = 16u, const vk::BufferUsageFlags& usage = vk::BufferUsage::eStorageBufferBit) {
                bfc.sharingMode = vk::SharingMode::eExclusive;
                bfc.usage = usage;
                bfc.size = size;
            };

            ~Buffer(){ // Here will notification about free memory
                
            };

            // TODO: buffer regions and views create 
            void Create(vk::Buffer* buf, const std::shared_ptr<Allocator>& mem){
                mem->AllocateForBuffer(buf,allocation,bfc); if (buf) lastbuf = *buf;
            };
    };

};
