#pragma once

#include "../lib/core.hpp"

namespace lancer {

    class Allocation : public std::enable_shared_from_this<Allocation> {
        protected: 
            std::Memory memory = {};
            uint8_t* mapped = nullptr;

        public: 
            virtual void Free() {}

            virtual uintptr_t GetPtr() {
                return 0u; // xPEH TB
            }

            virtual uint8_t* GetMapped() {
                return nullptr;
            }

            Allocation(){};

            ~Allocation(){
                this->Free();
            }
    };

    // Vookoo-Like 
    class Buffer : public std::enable_shared_from_this<Buffer> {
        protected: 
            api::BufferCreateInfo bfc = {};
            //std::shared_ptr<Device> dvc = {};
            std::shared_ptr<Allocation> allocation = {};

        public: 
            Buffer(const std::shared_ptr<Device>& device, const sizei_t& size = 16u, const vk::BufferUsageFlags& usage = vk::BufferUsage::eStorageBufferBit) {
                bfc.sharingMode = vk::SharingMode::eExclusive;
                bfc.usage = usage;
                bfc.size = size;
            }

            ~Buffer(){ // Here will notification about free memory
                
            }

            void Create(vk::Buffer& buf, const std::shared_ptr<Allocator>& mem){
                mem->AllocateForBuffer(&buf,allocation,bfc);
            }
        };

    // Vookoo-Like 
    class Image : public std::enable_shared_from_this<Image> {
        protected: 
            api::ImageCreateInfo imc = {};
            //std::shared_ptr<Device> dvc = {};
            std::shared_ptr<Allocation> allocation = {};

        public: 
            Image(const std::shared_ptr<Device>& device) {
                
            }

            ~Image(){ // Here will notification about free memory
                
            }

            void Create(vk::Image& img, const std::shared_ptr<Allocator>& mem) {
                mem->AllocateForImage(&img,allocation,imc);
            }
    };

};
