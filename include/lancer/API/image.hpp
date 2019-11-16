#pragma once

#include "../lib/core.hpp"

namespace lancer {

    // Vookoo-Like 
    class Image : public std::enable_shared_from_this<Image> {
        protected: 
            api::ImageCreateInfo imc = {};
            api::Image lastimg = {}; // least allocation, may be vector 
            std::shared_ptr<Allocation> allocation = {}; // least allocation, may be vector 

        public: 
            Image() {
                
            };

            ~Image(){ // Here will notification about free memory

            };

            void Create(api::Image* img, const std::shared_ptr<Allocator>& mem) {
                mem->AllocateForImage(img,allocation,imc); if (img) lastimg = *img;
            };
    };

};
