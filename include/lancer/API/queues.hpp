#pragma once

#include <cstdint>
#include "../lib/core.hpp"
#include "../API/memory.hpp"


namespace lancer {
    // TODO: Add Threads, Queues and Commands Support 
    // TODO: Advanced Commands Managment...

    class Task_T;
    class CommandRecord_T;
    class Queue_T;

    using Task = std::shared_ptr<Task_T>;
    using CommandRecord = std::shared_ptr<CommandRecord_T>;
    using QueueHelper = std::shared_ptr<Queue_T>;

    struct Command_T {
        uint32_t wType = 0u;
        uintptr_t _command = uintptr_t(nullptr);
    };

    class Task_T : public std::enable_shared_from_this<Task_T> {
        protected: 
            QueueMaker _queue = {};
            api::CommandBuffer* _cmdbuf = nullptr;
            std::function<std::future<vk::Result>(api::CommandBuffer&, const uintptr_t&)> _caller = {};

        public: 
            Task_T(QueueMaker queue, const std::function<std::future<vk::Result>(api::CommandBuffer&, const uintptr_t&)>& caller = {}): _caller(caller), _queue(queue) {
                
            };
            Task link(api::CommandBuffer& cmdbuf) { this->_cmdbuf = cmdbuf; return shared_from_this(); };
            std::future<vk::Result> operator()(const uintptr_t& param_ptr = nullptr) { return this->_caller(*this->_cmdbuf, param_ptr); };
    };

    // COMMAND POOL PTR IS NECESSARY
    // COMMANDS CAN BE CREATED BY "FINISH" METHOD 
    class CommandRecord_T : public std::enable_shared_from_this<CommandRecord_T> {
        protected: 
            DeviceMaker device = {};
            api::CommandBufferAllocateInfo = {};
            api::CommandPool* cmdpool = nullptr;
            std::vector<Command_T> commands = {};
            std::vector<api::CommandBuffer> store = {}; // STORE FOR FOR Task_T

        public: 
            
    };

    // PLANNED MULTI-THREADING SUPPORT
    class Queue_T : public std::enable_shared_from_this<Queue_T> {
        protected: 
            friend Task_T;
            DeviceMaker device = {};
            std::vector<std::weak_ptr<Task_T>> tasks = {};

        public: 
            
    };


};
