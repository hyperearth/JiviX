#pragma once
#include "./config.hpp"
#include "./driver.hpp"

namespace lancer {
    class Thread : public std::enable_shared_from_this<Thread> { public: 
        Thread() {};
        Thread(const std::shared_ptr<Driver>& driver) { // derrivate from driver framework
            this->driver = driver;
            this->queue = *driver;
            this->commandPool = *driver;
            this->descriptorPool = *driver;
        };

        // TODO: create dedicated thread pool
        std::shared_ptr<Thread> createThreadPool() {
            
            return shared_from_this();
        };

        // 
        std::shared_ptr<Thread> setDriver(const std::shared_ptr<Driver>& driver) {
            this->driver = driver;
            return shared_from_this();
        };

        // 
        std::shared_ptr<Thread> createQueue() {

            return shared_from_this();
        };
        
        // 
        std::shared_ptr<Thread> createCommandPool() {
            
            return shared_from_this();
        };

        // 
        std::shared_ptr<Thread> createDescriptorPool() {

            return shared_from_this();
        };

        // Getter Operators
        vk::CommandPool& getCommandPool() { return commandPool; };
        vk::DescriptorPool& getDescriptorPool() { return descriptorPool; };
        vk::Queue& getQueue() { return queue; };
        vk::Device& getDevice() { return driver->getDevice(); };
        std::shared_ptr<Driver>& getDriverPtr() { return driver; };
        Driver& getDriver() { return *driver; };

        // 
        const vk::CommandPool& getCommandPool() const { return commandPool; };
        const vk::DescriptorPool& getDescriptorPool() const { return descriptorPool; };
        const vk::Queue& getQueue() const { return queue; };
        const vk::Device& getDevice() const { return driver->getDevice(); };
        const std::shared_ptr<Driver>& getDriverPtr() const { return driver; };
        const Driver& getDriver() const { return *driver; };
        
        // Getter Operators
        operator vk::CommandPool&() { return commandPool; };
        operator vk::DescriptorPool&() { return descriptorPool; };
        operator vk::Queue&() { return queue; };
        operator vk::Device&() { return driver->getDevice(); };
        operator std::shared_ptr<Driver>&() { return driver; };
        operator Driver&() { return *driver; };

        // 
        operator const vk::CommandPool&() const { return commandPool; };
        operator const vk::DescriptorPool&() const { return descriptorPool; };
        operator const vk::Queue&() const { return queue; };
        operator const vk::Device&() const { return driver->getDevice(); };
        operator const std::shared_ptr<Driver>&() const { return driver; };
        operator const Driver&() const { return *driver; };

        // indirect access
        Driver& operator*() { return *driver; };
        Driver* operator->() { return &(*driver); };

        // indirect const access
        const Driver& operator*() const { return *driver; };
        const Driver* operator->() const { return &(*driver); };

        // 
        std::shared_ptr<Thread> submitOnce(const std::function<void(vk::CommandBuffer&)>& cmdFn = {}, const vk::SubmitInfo& smbi = {}) {
            vkt::submitOnce(*this, *this, *this, cmdFn, smbi);
            return shared_from_this();
        };

        // Async Version
        auto submitOnceAsync(const std::function<void(vk::CommandBuffer&)>& cmdFn = {}, const vk::SubmitInfo& smbi = {}) {
            return std::async(std::launch::async | std::launch::deferred, [=, this]() {
                vkt::submitOnceAsync(*this, *this, *this, cmdFn, smbi).get();
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
