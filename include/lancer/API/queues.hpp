#pragma once

#include <cstdint>
#include "../lib/core.hpp"
#include "../API/memory.hpp"


namespace lancer {
    // TODO: Add Threads, Queues and Commands Support 
    // TODO: Advanced Commands Managment...
    // TODO: Add Semaphores and Fences Support...
    // TODO: Add Multi-Threading Support... 

    class Task_T;
    class CommandRecord_T;
    class Queue_T;

    using Task = std::shared_ptr<Task_T>;
    using CommandRecord = std::shared_ptr<CommandRecord_T>;
    using QueueHelper = std::shared_ptr<Queue_T>;

    enum class CommandType: uint32_t {
        eCustom = 0u
    };

    struct Command_T {
        uint32_t wType = 0u;
        uintptr_t _command = uintptr_t(nullptr);
    };

    struct CustomCommand {
        std::function<void(api::CommandBuffer&)> caller;
    };

    // MULTI-THREADING COMMAND CALLER
    class Task_T : public std::enable_shared_from_this<Task_T> {
        protected: 
            QueueMaker _queue = {};
            api::CommandBuffer* _cmdbuf = nullptr;
            std::function<std::future<api::Result>(api::CommandBuffer&, const uintptr_t&)> _caller = {};

        public: 
            Task_T(QueueMaker queue, const std::function<std::future<api::Result>(api::CommandBuffer&, const uintptr_t&)>& caller = {}, api::CommandBuffer* cmdbuf = nullptr): _caller(caller), _queue(queue), _cmdbuf(cmdbuf) {
                
            };
            Task linkCommandBuffer(api::CommandBuffer& cmdbuf) { this->_cmdbuf = &cmdbuf; return shared_from_this(); };
            std::future<api::Result> operator()(const uintptr_t& param_ptr = nullptr) { return this->_caller(*this->_cmdbuf, param_ptr); };
    };

    // Command Pool alternative (command manager, command create helper)
    class CommandRecord_T : public std::enable_shared_from_this<CommandRecord_T> {
        protected: 
            QueueMaker queue = {};
            api::CommandBufferAllocateInfo cmdinfo = {};
            api::CommandPool* cmdpool = nullptr;
            std::vector<Command_T> commands = {};
            bool generated = false;

        public: 
            CommandRecord_T(const QueueMaker& queue, api::CommandPool* cmdpool = nullptr) : queue(queue), cmdpool(cmdpool) {
                this->reset();
            };

            api::CommandBufferAllocateInfo& getAllocInfo() { return cmdinfo; };
            const api::CommandBufferAllocateInfo& getAllocInfo() const { return cmdinfo; };
            
            CommandRecord reset() {
                cmdinfo = {}, commands = {}, generated = false;
                return shared_from_this(); };
            CommandRecord pushCommand(const Command_T& command = {}) {
                commands.push_back(command);
                return shared_from_this(); };
            CommandRecord finish(api::CommandBuffer& cbuf = {}, const bool& secondary = false) { // TODO: Generate Commands and Buffer
                //cbuf = QueueMaker->createCommandBuffer(cmdinfo); // use queue for create command 
                for (auto& cmd : commands) { switch(CommandType(cmd.wType)) {
                    case CommandType::eCustom: ((CustomCommand*)(cmd._command)).caller(cbuf); break; // Currently only that type supported 
                    default: 
                };};
                generated = true; return shared_from_this(); };
            CommandRecord finish(const bool& primary = false) {
                cmdinfo.level = secondary ? api::CommandBufferLevel::eSecondary : api::CommandBufferLevel::ePrimary;
                cmdinfo.commandPool = *cmdpool;
                cmdinfo.commandBufferCount = 1u;
                return this->finish(QueueMaker->createCommandBuffer(cmdinfo)); // currently useless until return api::CommandBuffer 
            };
    };

    // PLANNED MULTI-THREADING SUPPORT
    // Direct Queue, Command Pools and Buffers Managment 
    class Queue_T : public std::enable_shared_from_this<Queue_T> {
        protected: 
            friend Task_T;
            DeviceMaker device = {};
            api::CommandPoolCreateInfo cpool = {};
            std::vector<api::CommandBuffer> store = {};
            std::vector<api::CommandPool> pools = {};
            std::vector<std::weak_ptr<Task_T>> tasks = {}; // TODO: Tasks Tracking System 
            api::Queue *queue = nullptr;
            uint32_t queueFamilyIndex = 0u;
            
        public: 
            Queue_T(const DeviceMaker& device = {}, const uint32_t queueFamilyIndex& = 0u, api::Queue* queue = nullptr) device(device), queueFamilyIndex(queueFamilyIndex), queue(queue) {
                
            };

            // Bit Stupid Function for create command buffer 
            api::CommandBuffer& createCommandBuffer(const api::CommandBufferAllocateInfo& info = {}) { this->store.push_back(device->least().allocateCommandBuffers(info)[0]); return store.back(); }; // Through Store Buffer 
            QueueHelper createCommandBuffer(api::CommandBuffer& cbuf = {}, const api::CommandBufferAllocateInfo& info = {}) { cbuf = device->least().allocateCommandBuffers(info)[0]; return shared_from_this(); }; // Through Directly 
            QueueHelper linkQueueFamilyIndex(const uint32_t& queueFamilyIndex = 0u) { this->queueFamilyIndex = queueFamilyIndex; return shared_from_this(); };
            QueueHelper linkQueue(api::Queue& queue) { this->queue = &queue; return shared_from_this(); };
            CommandRecord createCommandRecord() {
                cpool.queueFamilyIndex = this->queueFamilyIndex;
                pools.push_back(device->least().createCommandPool(&cpool));
                return std::make_shared<CommandRecord_T>(shared_from_this(),pools.back());
            };

            // TODO: Generate Execution Function 
            Task createTask(api::CommandBuffer& cbuf = {}) { return std::make_shared<Task_T>(shared_from_this(), {}, &cbuf); };
            Task createTask() { return this->createTask(store.back()); }; // Use Last Used api::CommandBuffer in store arrays 
    };


};
