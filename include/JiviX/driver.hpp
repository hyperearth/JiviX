#pragma once

#include <memory>
#include "JVI/driver.hpp"

namespace jvx { 
    class Driver {
    public:
        Driver() {};
        Driver(const Driver& driver) : object(driver.object) {};
        Driver(const std::shared_ptr<jvi::Driver>& object) : object(object) {};

        // 
        virtual Driver& operator=(const std::shared_ptr<jvi::Driver>& object) { this->object = object; return *this; };
        virtual Driver& operator=(const Driver& object) { this->object = object.object; return *this; };

        // 
        virtual operator std::shared_ptr<jvi::Driver>& () { return object; };
        virtual operator const std::shared_ptr<jvi::Driver>& () const { return object; };

        // 
        virtual jvi::Driver* operator->() { return &(*object); };
        virtual jvi::Driver& operator*() { return (*object); };
        virtual const jvi::Driver* operator->() const { return &(*object); };
        virtual const jvi::Driver& operator*() const { return (*object); };

    protected: friend jvx::Driver; friend jvi::Driver; // 
             std::shared_ptr<jvi::Driver> object = {};
    };
};
