#pragma once

#include "../lib/core.hpp"


namespace lancer {
    
    class Device;

    class Instance : public std::enable_shared_from_this<Instance> {
        protected: 
            std::vector<api::PhysicalDevice> devices = {};
            api::Instance* lastinst = {};

        public: 
            Instance(api::Instance* instance = nullptr, vk::InstanceCreateInfo info = {}) : lastinst(instance) {
            };
    };

    class Device : public std::enable_shared_from_this<Device> {
        protected: 
            api::Device* lastdev = nullptr;
            api::DeviceCreateInfo div = {};

        public: 
            Device(const std::shared_ptr<Instance>& instance) {
            };

            void Enumerate(){
            }; // TODO search needed device 

            void Create() {
            }; // TODO: create device 

            // Get original Vulkan link 
            vk::Device& Least() { return *lastdev; };
            operator vk::Device&() { return *lastdev; };
            const vk::Device& Least() const { return *lastdev; };
            operator const vk::Device&() const { return *lastdev; };

            // 
            std::shared_ptr<Device>& Link(api::Device& dev) { lastdev = &dev; return shared_from_this(); };
    };


};
