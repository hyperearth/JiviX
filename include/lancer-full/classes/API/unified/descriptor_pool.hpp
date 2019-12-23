#pragma once

#include "./classes/API/types.hpp"
#include "./classes/API/unified/device.hpp"
#include "./factory/API/unified/descriptor_pool.hpp"
#include "./factory/API/unified/descriptor_set.hpp"

namespace svt {
    namespace api {
        namespace classes {

            // TODO: move some implementations into `.cpp`
            class descriptor_pool { public: 
                descriptor_pool(const descriptor_pool& descriptor_pool) : device_(descriptor_pool), descriptor_pool_(descriptor_pool) {};
                descriptor_pool(                              const stu::descriptor_pool& descriptor_pool_ = {}) : descriptor_pool_(descriptor_pool_) {};
                descriptor_pool(const stu::device_t& device_, const stu::descriptor_pool& descriptor_pool_ = {}) : descriptor_pool_(descriptor_pool_), device_(device_) {};

                // 
                descriptor_pool& operator=(const descriptor_pool &descriptor_pool) { 
                    this->descriptor_pool_ = descriptor_pool;
                    this->device_ = descriptor_pool;
                    return *this;
                };

                // 
                operator stu::descriptor_pool&() { return descriptor_pool_; };
                operator stu::device&() { return device_; };
                operator stu::device_t&() { return device_; };
                operator const stu::descriptor_pool&() const { return descriptor_pool_; };
                operator const stu::device&() const { return device_; };
                operator const stu::device_t&() const { return device_; };

                // 
                api::factory::descriptor_pool_t* operator->() { return &(*this->descriptor_pool_); };
                const api::factory::descriptor_pool_t* operator->() const { return &(*this->descriptor_pool_); };
                api::factory::descriptor_pool_t& operator*() { return (*this->descriptor_pool_); };
                const api::factory::descriptor_pool_t& operator*() const { return (*this->descriptor_pool_); };

                // 
                svt::core::handle_ref<descriptor_pool,core::api::result_t> create(const descriptor_pool_create_info& info = {});

                

                // 
                operator uintptr_t&() { return (*descriptor_pool_); };
                operator const uintptr_t&() const { return (*descriptor_pool_); };
                operator core::api::descriptor_pool_t&() { return (*descriptor_pool_); };
                operator const core::api::descriptor_pool_t&() const { return (*descriptor_pool_); };


            protected: friend descriptor_set; friend descriptor_pool;
                stu::descriptor_pool descriptor_pool_ = {};
                stu::device_t device_ = {};
            };


        };
    };
};
