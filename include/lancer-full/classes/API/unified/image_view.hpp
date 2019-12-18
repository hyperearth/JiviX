#pragma once

#include "./classes/API/types.hpp"
#include "./classes/API/unified/image.hpp"

namespace svt {
    namespace api {
        namespace classes {

            // TODO: create image_view method
            class image_view { public: 

            protected: 
                stu::image_view image_view_ = {};
                stu::image image_ = {};
                stu::sampler sampler_ = {};
                stu::device device_ = {};
            };

        };
    };
};
