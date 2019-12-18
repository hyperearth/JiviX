#pragma once

#include "./classes/API/types.hpp"
#include "./classes/API/unified/image.hpp"

namespace svt {
    namespace api {
        namespace classes {

            // TODO: image_view class
            class image_view {
                protected: 
                    stu::image_view image_view_t = {};
                    stu::sampler sampler_t = {};
                    stu::device device_t = {};
                    svt::image::image_layout layout_t = {};
                    
                public:


                    // TODO: create image_view method
            };

        };
    };
};
