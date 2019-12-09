#include "../../unified/buffer.hpp"

namespace svt {

    core::handle_ref<buffer,core::result_t> buffer::create(const buffer::create_info& info) {
        return {*this,core::result_t(0u)};
    };

};
