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
        virtual VkCommandPool& getCommandPool() { return commandPool; };
        virtual VkDescriptorPool& getDescriptorPool() { return descriptorPool; };
        virtual VkQueue& getQueue() { return queue; };
        virtual VkDevice& getDevice() { return driver->getDevice(); };
        virtual vkt::uni_ptr<Driver>& getDriverPtr() { return driver; };
        virtual Driver& getDriver() { return *driver; };

        // 
        virtual const VkCommandPool& getCommandPool() const { return commandPool; };
        virtual const VkDescriptorPool& getDescriptorPool() const { return descriptorPool; };
        virtual const VkQueue& getQueue() const { return queue; };
        virtual const VkDevice& getDevice() const { return driver->getDevice(); };
        virtual const vkt::uni_ptr<Driver>& getDriverPtr() const { return driver; };
        virtual const Driver& getDriver() const { return *driver; };

        // Getter Operators
        virtual operator VkCommandPool&() { return commandPool; };
        virtual operator VkDescriptorPool&() { return descriptorPool; };
        virtual operator VkQueue&() { return queue; };
        virtual operator VkDevice&() { return driver->getDevice(); };
        virtual operator std::shared_ptr<Driver>&() { return driver; };
        virtual operator Driver&() { return *driver; };

        // 
        virtual operator const VkCommandPool&() const { return commandPool; };
        virtual operator const VkDescriptorPool&() const { return descriptorPool; };
        virtual operator const VkQueue&() const { return queue; };
        virtual operator const VkDevice&() const { return driver->getDevice(); };
        virtual operator const std::shared_ptr<Driver>&() const { return driver; };
        virtual operator const Driver&() const { return *driver; };

        // indirect access
        virtual Driver& operator*() { return *driver; };
        virtual Driver* operator->() { return &(*driver); };

        // indirect const access
        virtual const Driver& operator*() const { return *driver; };
        virtual const Driver* operator->() const { return &(*driver); };

        // 
        virtual uPTR(Thread) submitOnce(const std::function<void(VkCommandBuffer&)>& cmdFn = {}, const vkt::uni_arg<vkh::VkSubmitInfo>& smbi = vkh::VkSubmitInfo{}) {
            vkt::submitOnce(VkDevice(*this), VkQueue(*this), VkCommandPool(*this), cmdFn, smbi);
            return uTHIS;
        };

        // Async Version
        virtual std::future<uPTR(Thread)> submitOnceAsync(const std::function<void(VkCommandBuffer&)>& cmdFn = {}, const vkt::uni_arg<vkh::VkSubmitInfo>& smbi = vkh::VkSubmitInfo{}) {
            return std::async(std::launch::async | std::launch::deferred, [=, this]() {
                vkt::submitOnceAsync(VkDevice(*this), VkQueue(*this), VkCommandPool(*this), cmdFn, smbi).get();
                return uPTR(Thread)(uTHIS);
            });
        };

        // Async Version
        virtual std::future<uPTR(Thread)> submitCmdAsync(const std::vector<VkCommandBuffer>& cmds, const vkt::uni_arg<vkh::VkSubmitInfo>& smbi = vkh::VkSubmitInfo{}) {
            return std::async(std::launch::async | std::launch::deferred, [=, this]() {
                vkt::submitCmdAsync(VkDevice(*this), VkQueue(*this), cmds, smbi).get();
                return uPTR(Thread)(uTHIS);
            });
        };

        // 
        virtual uPTR(Thread) submitCmd(const std::vector<VkCommandBuffer>& cmds, const vkt::uni_arg<vkh::VkSubmitInfo>& smbi = vkh::VkSubmitInfo{}) {
            vkt::submitCmd(VkDevice(*this), VkQueue(*this), cmds, smbi);
            return uTHIS;
        };

        /* // USELESS FOR VKT-3
        virtual uPTR(Thread) submitCmd(const vkt::uni_arg<VkCommandBuffer>& cmds, const vkt::uni_arg<vkh::VkSubmitInfo>& smbi = vkh::VkSubmitInfo{}) {
            return this->submitCmd({ cmds }, smbi);
        };

        // Async Version
        virtual std::future<uPTR(Thread)> submitCmdAsync(const vkt::uni_arg<VkCommandBuffer>& cmds, const vkt::uni_arg<vkh::VkSubmitInfo>& smbi = vkh::VkSubmitInfo{}) {
            return this->submitCmdAsync({ cmds }, smbi);
        };

        // 
        virtual uPTR(Thread) submitCmd(const vkt::uni_arg<VkCommandBuffer>& cmds, const vkt::uni_arg<vkh::VkSubmitInfo>& smbi = vkh::VkSubmitInfo{}) {
            return this->submitCmd(cmds, smbi);
        };

        // Async Version
        virtual std::future<uPTR(Thread)> submitCmdAsync(const vkt::uni_arg<VkCommandBuffer>& cmds, const vkt::uni_arg<vkh::VkSubmitInfo>& smbi = vkh::VkSubmitInfo{}) {
            return this->submitCmdAsync(cmds, smbi);
        };
        */

    // 
    protected: friend Thread; friend Driver; // 
        VkQueue queue = {};
        VkCommandPool commandPool = {};
        VkDescriptorPool descriptorPool = {};
        vkt::uni_ptr<Driver> driver = {};
    };

};
