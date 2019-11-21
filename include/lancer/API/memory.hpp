#pragma once

#include "../lib/core.hpp"
//#include "../API/device.hpp"

namespace lancer {

    class Allocation;
    class Allocator;
    class Device;

    class Allocation : public std::enable_shared_from_this<Allocation> {
        protected: 
            friend Allocator;
            friend Device;

            std::shared_ptr<Allocator> allocator = nullptr;
            api::MemoryHeap memory = {};
            uint8_t* mapped = nullptr;

        public: 
            virtual void free() {};
            virtual uintptr_t getPtr() { return 0u; }; // xPEH TB
            virtual uintptr_t getCIP() { return 0u; }; // xPEH TB
            virtual uint8_t* getMapped() {  return nullptr; };

            ~Allocation(){ this->free(); };
             Allocation() = default;
             Allocation(const std::shared_ptr<Allocator>& allocator){};

            inline const std::shared_ptr<Device>& getDevice() const { return allocator->getDevice(); };
    };

    class Allocator : public std::enable_shared_from_this<Allocator> {
        protected: 
            std::shared_ptr<Device> device = {};
            std::vector<std::weak_ptr<Allocation>> allocations = {};
            friend Device;

        public: 
            ~Allocator(){};
             Allocator() = default;
             Allocator(const std::shared_ptr<Device>& dvc);

            virtual void free() {};
            virtual void allocateForBuffer(api::Buffer* buffer, std::shared_ptr<Allocation>& allocation, const api::BufferCreateInfo& bfc = {}, const uintptr_t& ptx = 0u);
            virtual void allocateForImage(api::Image* image, std::shared_ptr<Allocation>& allocation, const api::ImageCreateInfo& bfc = {}, const uintptr_t& ptx = 0u);
            virtual void initialize(const std::shared_ptr<Device>& device = {});
            virtual std::shared_ptr<Allocation>&& createAllocation() { return std::make_shared<Allocation>(*this); };
            inline const std::shared_ptr<Device>& getDevice() const { return device; };
    };

    // TODO: Any implementations after device

};
