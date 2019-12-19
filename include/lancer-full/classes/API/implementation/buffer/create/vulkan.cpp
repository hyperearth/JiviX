// 
#include "./classes/API/unified/device.hpp"
#include "./classes/API/unified/allocator.hpp"

// 
#include "./factory/API/unified/buffer.hpp"
#include "./classes/API/unified/buffer.hpp"

namespace svt {
    namespace api {
        namespace classes {
#ifdef USE_VULKAN
            // TODO: reorder arguments
            svt::core::handle_ref<buffer,core::api::result_t> buffer::create(const stu::allocator& allocator_, const buffer_create_info& info, const uintptr_t& info_ptr) {
                this->buffer_ = allocator_->create_buffer(device_, device_->queue_family_indices_, info, info_ptr);
                this->allocator_ = allocator_; return { *this, core::api::result_t(0u) };
            };
#endif
        };
    };
};
