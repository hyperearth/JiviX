#pragma once

#include "./classes/API/types.hpp"

namespace svt {
    namespace api {
        namespace classes {
            class descriptor_set {
                protected: 
                    std::vector<vector<>> buffers = {};
                    std::vector<buffer_view> buffer_views = {};
                    std::vector<image_view> image_views = {};
                    
                    //std::vector<descriptor_set_entry> entry = {};
                public: 
                    
            };
        };
    };
};
