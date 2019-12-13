#pragma once

#include "./classes/API/types.hpp"
#include "./dynamic/API/unified/allocator.hpp"

namespace svt {
    namespace api {
        namespace classes {
            // Agregated type from `allocator_t`, and can be created by dedicated utils (such as VMA)
            class allocator {
                protected: friend allocator;
                    std::shared_ptr<api::dynamic::allocator_t> allocator_t;
                    std::shared_ptr<api::statics::device_t> device_t;

                public: 
                    allocator(const allocator& allocator) : device_t(allocator.device_t), allocator_t(allocator.allocator_t) {};
                    allocator(const std::shared_ptr<api::dynamic::allocator_t>& allocator = {}) : allocator_t(allocator) {};
                    allocator(const std::shared_ptr<api::statics::device_t>& device, const std::shared_ptr<api::dynamic::allocator_t>& allocator = {})  : allocator_t(allocator), device_t(device) {};
            };
        };

    };
};
