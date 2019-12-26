#pragma once

#include "./classes/API/types.hpp"
#include "./factory/API/unified/device.hpp"
#include "./factory/API/unified/queue.hpp"

namespace svt {
    namespace api {
        namespace classes {
            class queue { public: 
                queue(const queue& queue) : queue_(queue) {};
                queue(const stu::queue& queue_ = {}) : queue_(queue_) {};

                // TODO: add assigment by core types and shared_ptr types
                queue& operator=(const queue &queue) { this->device_ = (stu::device_t&)queue; this->queue_ = queue; return *this; };

                // TODO: move into `.cpp` file
                operator stu::queue&() { return queue_; };
                operator stu::device&() { return device_; };
                operator stu::device_t&() { return device_; };
                operator const stu::queue&() const { return queue_; };
                operator const stu::device&() const { return device_; };
                operator const stu::device_t&() const { return device_; };

                // 
                api::factory::queue_t* operator->() { return &(*this->queue_); };
                const api::factory::queue_t* operator->() const { return &(*this->queue_); };
                api::factory::queue_t& operator*() { return (*this->queue_); };
                const api::factory::queue_t& operator*() const { return (*this->queue_); };

                // 
                operator uintptr_t&() { return (*queue_); };
                operator const uintptr_t&() const { return (*queue_); };
                
                operator core::api::device_t&() { return device_; };
                operator const core::api::device_t&() const { return device_; };

                operator core::api::physical_device_t&() { return device_; };
                operator const core::api::physical_device_t&() const { return device_; };

                operator core::api::queue_t&() { return *queue_; };
                operator const core::api::queue_t&() const { return *queue_; };
                
            //
            protected: friend queue;
                stu::queue queue_ = {};
                stu::device_t device_ = {};
            };
        };
    };
};
