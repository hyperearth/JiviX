#pragma once

#include "./classes/API/types.hpp"
#include "./factory/API/unified/allocator.hpp"

namespace svt {
    namespace api {
        namespace classes {
            // Agregated type from `allocator_t`, and can be created by dedicated utils (such as VMA)
            class allocator {
                protected: friend allocator;
                    using device_st = std::shared_ptr<api::factory::device_t>;
                    using allocator_st = std::shared_ptr<api::factory::allocator_t>;

                    allocator_st allocator_t = {};
                    device_st device_t = {};

                public: 
                    allocator(const allocator& allocator_t) : device_t(allocator_t), allocator_t(allocator_t) {};
                    allocator(                         const allocator_st& allocator_t = {}) : allocator_t(allocator_t) {};
                    allocator(const device_st& device, const allocator_st& allocator_t = {}) : allocator_t(allocator_t), device_t(device) {};

                    // TODO: merge into `.cpp`
                    operator allocator_st&() { return allocator_t; };
                    operator device_st&() { return device_t; };
                    operator const allocator_st&() const { return allocator_t; };
                    operator const device_st&() const { return device_t; };
            };
        };
    };
};
