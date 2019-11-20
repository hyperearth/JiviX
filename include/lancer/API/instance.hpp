#pragma once

#include "../lib/core.hpp"


namespace lancer {
    class Instance : public std::enable_shared_from_this<Instance> {
        protected: 
            std::vector<api::PhysicalDevice> devices = {};
            api::Instance* lastinst = {};
            api::InstanceCreateInfo cif = {};
            
        public: 
            Instance(api::Instance* instance = nullptr, api::InstanceCreateInfo info = {}) : lastinst(instance), cif(info) {
                *instance = api::createInstance(info);
            };

            api::Instance& Least() { return *lastinst; };
            operator api::Instance&() { return *lastinst; };
            const api::Instance& Least() const { return *lastinst; };
            operator const api::Instance&() const { return *lastinst; };
    };

};
