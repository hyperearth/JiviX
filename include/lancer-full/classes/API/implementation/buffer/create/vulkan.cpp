#include "./factory/API/unified/device.hpp"
#include "./factory/API/unified/allocator.hpp"
#include "./factory/API/unified/buffer.hpp"
#include "./classes/API/unified/buffer.hpp"


namespace svt {
    namespace api {
        namespace classes {
#ifdef USE_VULKAN
            // TODO: reorder arguments
            svt::core::handle_ref<buffer,core::api::result_t> buffer::create(const stu::allocator& allocator, const buffer_create_info& info, const uintptr_t& info_ptr) {
                buffer_ = allocator->create_buffer(device_, device_->queue_family_indices_, info, info_ptr);
                return { *this, core::api::result_t(0u) };
            };
#endif
        };
    };
};
