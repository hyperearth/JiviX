#include "./classes/API/unified/buffer.hpp"
#include "./dynamic/API/unified/allocator.hpp"

namespace svt {
    namespace api {
        
#ifdef USE_VULKAN
        svt::core::handle_ref<buffer,core::result_t> buffer::create(const buffer::create_info& info, const allocator& allocator) {
            // SHOULD OVERRIDE `std::shared_ptr<data::dynamic::buffer_t>`
            return {*this,core::result_t(0u)};
        };
#endif

    };
};
