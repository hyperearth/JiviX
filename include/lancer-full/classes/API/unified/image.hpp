#pragma once

#include "../types.hpp"

namespace svt {
    namespace api {

        class image {
            protected: 
                std::shared_ptr<data::dynamic::image_t> image_t = {};
                std::shared_ptr<data::device_t> device_t = {};

            public: 

        };
    };
};
