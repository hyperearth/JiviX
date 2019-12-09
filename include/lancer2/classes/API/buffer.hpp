#pragma one
#include "../core/core.hpp"
#include "../static/API/buffer.hpp"

namespace svt {

    class buffer { protected: std::shared_ptr<data::buffer_t> buffer_t;
        public: 

        protected: 
            void create();
    };

};
