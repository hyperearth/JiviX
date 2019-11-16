#pragma once

#include "../lib/core.hpp"

namespace lancer {

    class Allocation : public std::enable_shared_from_this<Allocation> {
        protected: 
            std::Memory memory = {};
            uint8_t* mapped = nullptr;

        public: 
            virtual void Free() {};
            virtual uintptr_t GetPtr() { return 0u; }; // xPEH TB
            virtual uint8_t* GetMapped() {  return nullptr; };
            virtual uintptr_t GetCIP() { return 0u; };
            virtual void SetCIP(const uintptr_t& cip) {};

            Allocation(){};

            ~Allocation(){
                this->Free();
            }
    };

    class Allocator : public std::enable_shared_from_this<Allocator> {
        protected: 
            std::Memory memory = {};
            uint8_t* mapped = nullptr;
            
        public: 
            Allocator(){};
            
            virtual void AllocateForBuffer(api::Buffer* buffer, std::shared_ptr<Allocation>& allocation, api::BufferCreateInfo bfc = {});
            virtual void AllocateForImage(api::Image* image, std::shared_ptr<Allocation>& allocation, api::ImageCreateInfo bfc = {});
    };


};
