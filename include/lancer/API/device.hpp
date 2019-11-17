#pragma once

#include "../lib/core.hpp"


namespace lancer {
    
    class Device;

    class Instance : public std::enable_shared_from_this<Instance> {
        protected: 
            std::vector<api::PhysicalDevice> devices = {};
            api::Instance* lastinst = {};

        public: 
            Instance(api::Instance* instance = nullptr, api::InstanceCreateInfo info = {}) : lastinst(instance) {
            };
    };

    class Device : public std::enable_shared_from_this<Device> {
        protected: 
            api::DeviceCreateInfo div = {};
            api::Device* lastdev = nullptr;
            api::DescriptorPool *dscp = nullptr;

        public: 
            Device(const std::shared_ptr<Instance>& instance) {
            };

            void Enumerate(){
            }; // TODO search needed device 

            void Create() {
            }; // TODO: create device 

            // Get original Vulkan link 
            api::Device& Least() { return *lastdev; };
            operator api::Device&() { return *lastdev; };
            const api::Device& Least() const { return *lastdev; };
            operator const api::Device&() const { return *lastdev; };

            // 
            std::shared_ptr<Device>& Link(api::Device& dev) { lastdev = &dev; return shared_from_this(); };
            std::shared_ptr<Device>& LinkDescriptorPool(api::DescriptorPool& pool) {
                dscp = &pool; return shared_from_this();
            };
    };


};
