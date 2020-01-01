#pragma once
#include "./config.hpp"
#include "./driver.hpp"

namespace lancer {

    class Thread : public std::enable_shared_from_this<Thread> { public: 
        Thread() {};

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

        // casting access
        operator Driver&() { return *driver; };
        operator const Driver&() const { return *driver; };

        // indirect access
        Driver& operator*() { return *driver; };
        Driver* operator->() { return &(*driver); };

        // indirect const access
        const Driver& operator*() const { return *driver; };
        const Driver* operator->() const { return &(*driver); };

    protected: friend Thread; friend Driver; // 
        vk::Queue queue = {};
        vk::CommandPool commandPool = {};
        vk::DescriptorPool descriptorPool = {};
        std::shared_ptr<Driver> driver = {};
    };

};
