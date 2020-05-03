#pragma once // #

#include "./config.hpp"
#include "./driver.hpp"

namespace jvi {
    class Thread : public std::enable_shared_from_this<Thread> { public: 
        Thread() {};
        Thread(const vkt::uni_ptr<Driver>& driver) { // derrivate from driver framework
            this->driver = driver;
            this->queue = *driver;
            this->commandPool = *driver;
            this->descriptorPool = *driver;
        };
        Thread(const std::shared_ptr<Driver>& driver) { // derrivate from driver framework
            this->driver = driver;
            this->queue = *driver;
            this->commandPool = *driver;
            this->descriptorPool = *driver;
        };
        ~Thread() {};

        // 
        virtual vkt::uni_ptr<Thread> sharedPtr() { return shared_from_this(); };
        //virtual vkt::uni_ptr<Thread> sharedPtr() const { return shared_from_this(); };

        // TODO: create dedicated thread pool
        virtual uPTR(Thread) createThreadPool() {
            
            return uTHIS;
        };

        // 
        virtual uPTR(Thread) setDriver(vkt::uni_ptr<Driver> driver) {
            this->driver = driver;
            return uTHIS;
        };

        // 
        virtual uPTR(Thread) createQueue() {

            return uTHIS;
        };

        // 
        virtual uPTR(Thread) createCommandPool() {
            
            return uTHIS;
        };

        // 
        virtual uPTR(Thread) createDescriptorPool() {

            return uTHIS;
        };

        // Getter Operators
        virtual vk::CommandPool& getCommandPool() { return commandPool; };
        virtual vk::DescriptorPool& getDescriptorPool() { return descriptorPool; };
        virtual vk::Queue& getQueue() { return queue; };
        virtual vk::Device& getDevice() { return driver->getDevice(); };
        virtual vkt::uni_ptr<Driver>& getDriverPtr() { return driver; };
        virtual Driver& getDriver() { return *driver; };

        // 
        virtual const vk::CommandPool& getCommandPool() const { return commandPool; };
        virtual const vk::DescriptorPool& getDescriptorPool() const { return descriptorPool; };
        virtual const vk::Queue& getQueue() const { return queue; };
        virtual const vk::Device& getDevice() const { return driver->getDevice(); };
        virtual const vkt::uni_ptr<Driver>& getDriverPtr() const { return driver; };
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
        virtual uPTR(Thread) submitOnce(const std::function<void(vk::CommandBuffer&)>& cmdFn = {}, const vkt::uni_arg<vk::SubmitInfo>& smbi = vk::SubmitInfo{}) {
            vkt::submitOnce(vk::Device(*this), vk::Queue(*this), vk::CommandPool(*this), cmdFn, smbi);
            return uTHIS;
        };

        // Async Version
        virtual std::future<uPTR(Thread)> submitOnceAsync(const std::function<void(vk::CommandBuffer&)>& cmdFn = {}, const vkt::uni_arg<vk::SubmitInfo>& smbi = vk::SubmitInfo{}) {
            return std::async(std::launch::async | std::launch::deferred, [=, this]() {
                vkt::submitOnceAsync(vk::Device(*this), vk::Queue(*this), vk::CommandPool(*this), cmdFn, smbi).get();
                return uPTR(Thread)(uTHIS);
            });
        };

        // 
        virtual uPTR(Thread) submitCmd(const std::vector<vk::CommandBuffer>& cmds, const vkt::uni_arg<vk::SubmitInfo>& smbi = vk::SubmitInfo{}) {
            vkt::submitCmd(vk::Device(*this), vk::Queue(*this), cmds, smbi);
            return uTHIS;
        };

        // Async Version
        virtual std::future<uPTR(Thread)> submitCmdAsync(const std::vector<vk::CommandBuffer>& cmds, const vkt::uni_arg<vk::SubmitInfo>& smbi = vk::SubmitInfo{}) {
            return std::async(std::launch::async | std::launch::deferred, [=, this]() {
                vkt::submitCmdAsync(vk::Device(*this), vk::Queue(*this), cmds, smbi).get();
                return uPTR(Thread)(uTHIS);
            });
        };

        // 
        virtual uPTR(Thread) submitCmd(const vkt::uni_arg<vk::CommandBuffer>& cmds, const vkt::uni_arg<vk::SubmitInfo>& smbi = vk::SubmitInfo{}) {
            return this->submitCmd({ cmds }, smbi);
        };

        // Async Version
        virtual std::future<uPTR(Thread)> submitCmdAsync(const vkt::uni_arg<vk::CommandBuffer>& cmds, const vkt::uni_arg<vk::SubmitInfo>& smbi = vk::SubmitInfo{}) {
            return this->submitCmdAsync({ cmds }, smbi);
        };

        // 
        virtual uPTR(Thread) submitCmd(const vkt::uni_arg<VkCommandBuffer>& cmds, const vkt::uni_arg<vk::SubmitInfo>& smbi = vk::SubmitInfo{}) {
            return this->submitCmd(vk::CommandBuffer(cmds), smbi);
        };

        // Async Version
        virtual std::future<uPTR(Thread)> submitCmdAsync(const vkt::uni_arg<VkCommandBuffer>& cmds, const vkt::uni_arg<vk::SubmitInfo>& smbi = vk::SubmitInfo{}) {
            return this->submitCmdAsync(vk::CommandBuffer(cmds), smbi);
        };

    // 
    protected: friend Thread; friend Driver; // 
        vk::Queue queue = {};
        vk::CommandPool commandPool = {};
        vk::DescriptorPool descriptorPool = {};
        vkt::uni_ptr<Driver> driver = {};
    };

};
