// 
#include "./classes/API/unified/device.hpp"
#include "./classes/API/unified/allocator.hpp"

// 
#include "./factory/API/unified/image.hpp"
#include "./classes/API/unified/image.hpp"

namespace svt {
    namespace api {
        namespace classes {
#ifdef USE_VULKAN
            // TODO: reorder arguments
            svt::core::handle_ref<image, core::api::result_t> image::create(const stu::allocator& allocator_, const image_create_info& info, const uintptr_t& info_ptr, const image_layout& initial_layout) {
                this->image_ = allocator_->create_image(device_, device_->queue_family_indices_, info, info_ptr, initial_layout);
                this->allocator_ = allocator_; return { *this, core::api::result_t(0u) };
            };
#endif
        };
    };
};
