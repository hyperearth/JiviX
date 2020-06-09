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
        public: virtual vkt::uni_ptr<Thread> sharedPtr() { return shared_from_this(); };
        //virtual vkt::uni_ptr<Thread> sharedPtr() const { return shared_from_this(); };

        // TODO: create dedicated thread pool
        protected: virtual uPTR(Thread) createThreadPool() {
            
            return uTHIS;
        };

        // 
        public: virtual uPTR(Thread) setDriver(vkt::uni_ptr<Driver> driver) {
            this->driver = driver;
            return uTHIS;
        };

        // 
        protected: virtual uPTR(Thread) createQueue() {

            return uTHIS;
        };

        // 
        protected: virtual uPTR(Thread) createCommandPool() {
            
            return uTHIS;
        };

        // 
        protected: virtual uPTR(Thread) createDescriptorPool() {

            return uTHIS;
        };

        // Getter Operators
        public: virtual VkCommandPool& getCommandPool() { return commandPool; };
        public: virtual VkDescriptorPool& getDescriptorPool() { return descriptorPool; };
        public: virtual VkQueue& getQueue() { return queue; };
        public: virtual VkDevice& getDevice() { return driver->getDevice(); };
        public: virtual vkt::uni_ptr<Driver>& getDriverPtr() { return driver; };
        public: virtual Driver& getDriver() { return *driver; };

        // 
        public: virtual const VkCommandPool& getCommandPool() const { return commandPool; };
        public: virtual const VkDescriptorPool& getDescriptorPool() const { return descriptorPool; };
        public: virtual const VkQueue& getQueue() const { return queue; };
        public: virtual const VkDevice& getDevice() const { return driver->getDevice(); };
        public: virtual const vkt::uni_ptr<Driver>& getDriverPtr() const { return driver; };
        public: virtual const Driver& getDriver() const { return *driver; };

        // Getter Operators
        public: virtual operator VkCommandPool& () { return commandPool; };
        public: virtual operator VkDescriptorPool& () { return descriptorPool; };
        public: virtual operator VkQueue& () { return queue; };
        public: virtual operator VkDevice& () { return driver->getDevice(); };
        public: virtual operator std::shared_ptr<Driver>& () { return driver; };
        public: virtual operator Driver& () { return *driver; };

        // 
        public: virtual operator const VkCommandPool& () const { return commandPool; };
        public: virtual operator const VkDescriptorPool& () const { return descriptorPool; };
        public: virtual operator const VkQueue& () const { return queue; };
        public: virtual operator const VkDevice& () const { return driver->getDevice(); };
        public: virtual operator const std::shared_ptr<Driver>& () const { return driver; };
        public: virtual operator const Driver& () const { return *driver; };


        // indirect access
        public: virtual Driver& operator*() { return *driver; };
        public: virtual Driver* operator->() { return &(*driver); };

        // indirect const access
        public: virtual const Driver& operator*() const { return *driver; };
        public: virtual const Driver* operator->() const { return &(*driver); };

        // 
        public: virtual uPTR(Thread) submitOnce(const std::function<void(VkCommandBuffer&)>& cmdFn = {}, const vkt::uni_arg<vkh::VkSubmitInfo>& smbi = vkh::VkSubmitInfo{}) {
            vkt::submitOnce(this->driver->getDeviceDispatch(), VkQueue(*this), VkCommandPool(*this), cmdFn, smbi);
            return uTHIS;
        };

        // Async Version
        public: virtual std::future<uPTR(Thread)> submitOnceAsync(const std::function<void(VkCommandBuffer&)>& cmdFn = {}, const vkt::uni_arg<vkh::VkSubmitInfo>& smbi = vkh::VkSubmitInfo{}) {
            return std::async(std::launch::async | std::launch::deferred, [=, this]() {
                vkt::submitOnceAsync(this->driver->getDeviceDispatch(), VkQueue(*this), VkCommandPool(*this), cmdFn, smbi).get();
                return uPTR(Thread)(uTHIS);
            });
        };

        // 
        public: virtual uPTR(Thread) submitCmd(const vkt::uni_arg<VkCommandBuffer>& cmds, const vkt::uni_arg<vkh::VkSubmitInfo>& smbi = vkh::VkSubmitInfo{}) {
            return this->submitCmd(std::vector<VkCommandBuffer>{ cmds }, smbi);
        };

        // 
        public: virtual uPTR(Thread) submitCmd(const std::vector<VkCommandBuffer>& cmds, const vkt::uni_arg<vkh::VkSubmitInfo>& smbi = vkh::VkSubmitInfo{}) {
            vkt::submitCmd(this->driver->getDeviceDispatch(), VkQueue(*this), cmds, smbi);
            return uTHIS;
        };

        // Async Version
        public: virtual std::future<uPTR(Thread)> submitCmdAsync(const vkt::uni_arg<VkCommandBuffer>& cmds, const vkt::uni_arg<vkh::VkSubmitInfo>& smbi = vkh::VkSubmitInfo{}) {
            return this->submitCmdAsync(std::vector<VkCommandBuffer>{ cmds }, smbi);
        };

        // Async Version
        public: virtual std::future<uPTR(Thread)> submitCmdAsync(const std::vector<VkCommandBuffer>& cmds, const vkt::uni_arg<vkh::VkSubmitInfo>& smbi = vkh::VkSubmitInfo{}) {
            return std::async(std::launch::async | std::launch::deferred, [=, this]() {
                vkt::submitCmdAsync(this->driver->getDeviceDispatch(), VkQueue(*this), cmds, smbi).get();
                return uPTR(Thread)(uTHIS);
            });
        };

    // 
    protected: friend Thread; friend Driver; // 
        VkQueue queue = VK_NULL_HANDLE;
        VkCommandPool commandPool = VK_NULL_HANDLE;
        VkDescriptorPool descriptorPool = VK_NULL_HANDLE;
        vkt::uni_ptr<Driver> driver = {};
    };

};
