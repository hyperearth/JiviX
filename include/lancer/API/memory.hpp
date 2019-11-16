#pragma once

#include "../lib/core.hpp"

namespace lancer {
    
    /*
    template<class A>
    class Memory : public std::enable_shared_from_this<Memory<A>> {
        protected: 
            std::shared_ptr<A> allocator = nullptr;


        public: 

        Memory(){ allocator = std::make_shared_ptr<A>(); };
        Memory(const std::shared_ptr<A>& ptr) : allocator(ptr) {};

        void SetSize(const size_t& size = 0ull) {
            allocator->SetSize(size);
        };

        void Allocate(const uintptr_t& ptr = 0ull) {
            allocator->Allocate(ptr);
        };

        void AllocateForBuffer(const Buffer& buffer, const uintptr_t& ptr = 0ull){
            allocator->AllocateForBuffer(buffer, ptr);
        };

        void AllocateForImage(const Image& image, const uintptr_t& ptr = 0ull){
            allocator->AllocateForImage(image, ptr);
        };
    };
    */

};
