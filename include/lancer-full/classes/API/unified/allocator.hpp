#pragma once

#include "./classes/API/types.hpp"

namespace svt {
    namespace api {
        
        // Agregated type from `allocator_t`, and can be created by dedicated utils (such as VMA)
        class allocator {
            protected: 
                std::shared_ptr<data::dynamic::allocator_t> allocator;
                std::shared_ptr<data::device_t> device;

            public: // TODO: create_buffer, create_image, methods
                
        };

    };
};
