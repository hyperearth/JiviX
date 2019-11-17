#pragma once

#include "../lib/core.hpp"
#include "../API/device.hpp"

namespace lancer {

    class Allocator;

    class Allocation : public std::enable_shared_from_this<Allocation> {
        protected: 
            friend Allocator;

            std::shared_ptr<Allocator> allocator = nullptr;
            api::MemoryHeap memory = {};
            uint8_t* mapped = nullptr;

        public: 
            virtual void Free() {};
            virtual uintptr_t GetPtr() { return 0u; }; // xPEH TB
            virtual uintptr_t GetCIP() { return 0u; }; // xPEH TB
            virtual uint8_t* GetMapped() {  return nullptr; };

            ~Allocation(){ this->Free(); };
             Allocation() = default;
             Allocation(const std::shared_ptr<Allocator>& allocator){};

            const std::shared_ptr<Device>& GetDevice() const { return allocator->GetDevice(); };
    };

    class Allocator : public std::enable_shared_from_this<Allocator> {
        protected: 
            std::shared_ptr<Device> device = {};
            std::vector<std::weak_ptr<Allocation>> allocations = {};

        public: 
            ~Allocator(){};
             Allocator() = default;
             Allocator(const std::shared_ptr<Device>& dvc);

            virtual void Free() {};
            virtual void AllocateForBuffer(api::Buffer* buffer, std::shared_ptr<Allocation>& allocation, const api::BufferCreateInfo& bfc = {}, const uintptr_t& ptx = 0u);
            virtual void AllocateForImage(api::Image* image, std::shared_ptr<Allocation>& allocation, const api::ImageCreateInfo& bfc = {}, const uintptr_t& ptx = 0u);
            const std::shared_ptr<Device>& GetDevice() const { return device; };

    };

};
