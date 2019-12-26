#include "./factory/API/unified/device.hpp"
#include "./factory/API/unified/allocator.hpp"
#include "./classes/API/unified/buffer.hpp"
#include "./classes/API/unified/vector.hpp"
#include "./classes/API/unified/buffer_view.hpp"

namespace svt {
    namespace api {
        namespace classes {
#ifdef USE_VULKAN
            svt::core::handle_ref<buffer_view,core::api::result_t> buffer_view::create(const stu::device& device_, const stu::vector& vector_, const format& format_) {
                vk::BufferViewCreateInfo info = {};
                info.buffer = vector_->handle;
                info.flags = {};
                info.format = vk::Format(format_);
                info.offset = vector_->offset;
                info.range = vector_->range;
                this->vector_ = vector_;
                this->device_ = device_;
                (*buffer_view_).format_ = format_;
                (*buffer_view_) = (*device_)->createBufferView(info);
                return {*this,core::api::result_t(0u)};
            };
#endif
        };
    };
};
