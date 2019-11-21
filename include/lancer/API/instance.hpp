#pragma once

#include "../lib/core.hpp"

// TODO: Add Instance Methods (Enumerations)

namespace lancer {
    class Instance_T : public std::enable_shared_from_this<Instance_T> {
        protected: 
            std::vector<api::PhysicalDevice> devices = {};
            api::Instance* lastinst = {};
            api::InstanceCreateInfo cif = {};
            
        public: 
            Instance(api::Instance* instance = nullptr, api::InstanceCreateInfo info = {}) : lastinst(instance), cif(info) {
                *instance = api::createInstance(info);
            };

            inline api::Instance& least() { return *lastinst; };
            inline const api::Instance& least() const { return *lastinst; };
            operator api::Instance&() { return *lastinst; };
            operator const api::Instance&() const { return *lastinst; };
    };

};
