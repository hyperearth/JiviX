#pragma once

#include "../lib/core.hpp"


namespace lancer {
    
    class Device : public std::enable_shared_from_this<Device> {
        protected: 
            std::vector<api::PhysicalDevice> devices = {};
            api::Device* lastdev = nullptr;
            api::DeviceCreateInfo div = {};
            
        public: 
            void Enumerate(){ // TODO search needed device 
                
            };

            void Create() {
                // TODO: create device 
            };

            // Get original Vulkan link 
            vk::Device& Least() { return *lastdev; };
            operator vk::Device&() { return *lastdev; };
            const vk::Device& Least() const { return *lastdev; };
            operator const vk::Device&() const { return *lastdev; };
    };


};
