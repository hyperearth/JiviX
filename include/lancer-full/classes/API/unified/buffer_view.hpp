#pragma once

#include "./classes/API/types.hpp"
#include "./classes/API/unified/buffer.hpp"
#include "./classes/API/unified/vector.hpp"

namespace svt {
    namespace api {
        namespace classes {

            // TODO: auto format for types and template<T>
            class buffer_view {
                protected: 
                    stu::buffer_view buffer_view_t = {};
                    stu::device device_t = {};
                    svt::vector<> vector_t = {};
                    svt::api::format format_t = {};
                    
                public:
                    
            };

        };
    };
};
