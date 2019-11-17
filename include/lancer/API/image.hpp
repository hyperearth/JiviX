#pragma once

#include "../lib/core.hpp"
#include "../API/memory.hpp"

namespace lancer {

    // Vookoo-Like 
    class Image : public std::enable_shared_from_this<Image> {
        protected: 
            std::shared_ptr<Device> device = {};
            std::shared_ptr<Allocation> allocation = {}; // least allocation, may be vector 
            api::Image* lastimg = nullptr; // least allocation, may be vector 
            api::ImageCreateInfo imc = {};

        public: 
            Image(const std::shared_ptr<Device>& device, api::Image* lastimg = nullptr, api::ImageCreateInfo imc = {}) : lastimg(lastimg),imc(imc),device(device) {
            };

            ~Image(){
            }; // Here will notification about free memory

            // Get original Vulkan link 
            vk::Image& Least() { return *lastimg; };
            operator vk::Image&() { return *lastimg; };
            const vk::Image& Least() const { return *lastimg; };
            operator const vk::Image&() const { return *lastimg; };

            // 
            std::shared_ptr<Image>& Link(api::Image& img) { lastimg = &img; return shared_from_this(); };
            std::shared_ptr<Image>& Allocate(const std::shared_ptr<Allocator>& mem) {
                mem->AllocateForImage(lastimg,allocation,imc); 
                return shared_from_this();
            };

            // 
            std::shared_ptr<Image>& Create() {
                *lastimg = device->Least().createImage(imc);
                return shared_from_this();
            };

            // TODO: create ImageView 
    };

};
