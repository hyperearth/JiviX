#pragma once

#include "../lib/core.hpp"


namespace lancer {
    
    class Device : public std::enable_shared_from_this<Device> {
        protected: 
            std::vector<vk::PhysicalDevice> devices = {};
            api::DeviceCreateInfo div = {};

            
        public: 
            void Enumerate(){ // TODO search needed device 
                
            }

            void Create(vk::Device& device, const uint32_t& ID = 0u){
                
            };

    };


};
