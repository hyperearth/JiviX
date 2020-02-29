#pragma once // #

#include <memory>
#include <chrono>
#include "./config.hpp"
#include "./driver.hpp"

namespace jvi {
    class Thread : public std::enable_shared_from_this<Thread> { public: 
        Thread() {};
        Thread(const std::shared_ptr<Driver>& driver) { // derrivate from driver framework
            this->driver = driver;
            this->queue = *driver;
            this->commandPool = *driver;
            this->descriptorPool = *driver;
        };

        // TODO: create dedicated thread pool
        virtual std::shared_ptr<Thread> createThreadPool() {
            
            return shared_from_this();
        };

        // 
        virtual std::shared_ptr<Thread> setDriver(const std::shared_ptr<Driver>& driver) {
            this->driver = driver;
            return shared_from_this();
        };

        // 
        virtual std::shared_ptr<Thread> createQueue() {

            return shared_from_this();
        };

        // 
        virtual std::shared_ptr<Thread> createCommandPool() {
            
            return shared_from_this();
        };

        // 
        virtual std::shared_ptr<Thread> createDescriptorPool() {

            return shared_from_this();
        };

        // Getter Operators
        virtual vk::CommandPool& getCommandPool() { return commandPool; };
        virtual vk::DescriptorPool& getDescriptorPool() { return descriptorPool; };
        virtual vk::Queue& getQueue() { return queue; };
        virtual vk::Device& getDevice() { return driver->getDevice(); };
        virtual std::shared_ptr<Driver>& getDriverPtr() { return driver; };
        virtual Driver& getDriver() { return *driver; };

        // 
        virtual const vk::CommandPool& getCommandPool() const { return commandPool; };
        virtual const vk::DescriptorPool& getDescriptorPool() const { return descriptorPool; };
        virtual const vk::Queue& getQueue() const { return queue; };
        virtual const vk::Device& getDevice() const { return driver->getDevice(); };
        virtual const std::shared_ptr<Driver>& getDriverPtr() const { return driver; };
        virtual const Driver& getDriver() const { return *driver; };
    
        // Getter Operators
        virtual operator vk::CommandPool&() { return commandPool; };
        virtual operator vk::DescriptorPool&() { return descriptorPool; };
        virtual operator vk::Queue&() { return queue; };
        virtual operator vk::Device&() { return driver->getDevice(); };
        virtual operator std::shared_ptr<Driver>&() { return driver; };
        virtual operator Driver&() { return *driver; };

        // 
        virtual operator const vk::CommandPool&() const { return commandPool; };
        virtual operator const vk::DescriptorPool&() const { return descriptorPool; };
        virtual operator const vk::Queue&() const { return queue; };
        virtual operator const vk::Device&() const { return driver->getDevice(); };
        virtual operator const std::shared_ptr<Driver>&() const { return driver; };
        virtual operator const Driver&() const { return *driver; };

        // indirect access
        virtual Driver& operator*() { return *driver; };
        virtual Driver* operator->() { return &(*driver); };

        // indirect const access
        virtual const Driver& operator*() const { return *driver; };
        virtual const Driver* operator->() const { return &(*driver); };

        // 
        virtual std::shared_ptr<Thread> submitOnce(const std::function<void(vk::CommandBuffer&)>& cmdFn = {}, const vk::SubmitInfo& smbi = {}) {
            vkt::submitOnce(*this, *this, *this, cmdFn, smbi);
            return shared_from_this();
        };

        // Async Version
        virtual std::future<std::shared_ptr<Thread>> submitOnceAsync(const std::function<void(vk::CommandBuffer&)>& cmdFn = {}, const vk::SubmitInfo& smbi = {}) {
            return std::async(std::launch::async | std::launch::deferred, [=, this]() {
                vkt::submitOnceAsync(*this, *this, *this, cmdFn, smbi).get();
                return this->shared_from_this();
            });
        };

        // 
        virtual std::shared_ptr<Thread> submitCmd(const std::vector<vk::CommandBuffer>& cmds, vk::SubmitInfo smbi = {}) {
            vkt::submitCmd(*this, *this, cmds, smbi);
            return shared_from_this();
        };

        // Async Version
        virtual std::future<std::shared_ptr<Thread>> submitCmdAsync(const std::vector<vk::CommandBuffer>& cmds, const vk::SubmitInfo& smbi = {}) {
            return std::async(std::launch::async | std::launch::deferred, [=, this]() {
                vkt::submitCmdAsync(*this, *this, cmds, smbi).get();
                return this->shared_from_this();
            });
        };

    // 
    protected: friend Thread; friend Driver; // 
        vk::Queue queue = {};
        vk::CommandPool commandPool = {};
        vk::DescriptorPool descriptorPool = {};
        std::shared_ptr<Driver> driver = {};
    };

};
