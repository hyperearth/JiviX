#pragma once

#include "../lib/core.hpp"

namespace lancer {

    class Allocation : public std::enable_shared_from_this<Allocation> {
        protected: 
            api::Memory memory = {};
            uint8_t* mapped = nullptr;
            std::shared_ptr<Allocator> allocator = nullptr;

        public: 
            virtual void Free() {};
            virtual uintptr_t GetPtr() { return 0u; }; // xPEH TB
            virtual uint8_t* GetMapped() {  return nullptr; };
            virtual uintptr_t GetCIP() { return 0u; };
            virtual void SetCIP(const uintptr_t& cip) {};

            Allocation(){};
            ~Allocation(){ this->Free(); };

            virtual const std::shared_ptr<Device>& GetDevice() const {
                return allocator->GetDevice();
            };
    };

    class Allocator : public std::enable_shared_from_this<Allocator> {
        protected: 
            api::Memory memory = {};
            uint8_t* mapped = nullptr;
            std::shared_ptr<Device> device = nullptr;
            std::vector<std::weak_ptr<Allocation>> allocations = {};
            
        public: 
            Allocator(){};
            ~Allocator(){};

            virtual void Free() {};
            virtual void AllocateForBuffer(api::Buffer* buffer, std::shared_ptr<Allocation>& allocation, api::BufferCreateInfo bfc = {});
            virtual void AllocateForImage(api::Image* image, std::shared_ptr<Allocation>& allocation, api::ImageCreateInfo bfc = {});

            virtual const std::shared_ptr<Device>& GetDevice() const {
                return device;
            };
    };


};
