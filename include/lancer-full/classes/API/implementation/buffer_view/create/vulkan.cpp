#include "./factory/API/unified/device.hpp"
#include "./factory/API/unified/allocator.hpp"
#include "./classes/API/unified/buffer.hpp"
#include "./classes/API/unified/vector.hpp"
#include "./classes/API/unified/buffer_view.hpp"

namespace svt {
    namespace api {
        namespace classes {
#ifdef USE_VULKAN
            svt::core::handle_ref<buffer_view,core::api::result_t> buffer_view::create(const stu::vector& vector, const format& format) {
                vk::BufferViewCreateInfo info = {};
                info.buffer = vector->handle;
                info.flags = {};
                info.format = vk::Format(format);
                info.offset = vector->offset;
                info.range = vector->range;
                vector_t = vector;
                buffer_view_t->format_t = format;
                buffer_view_t->buffer_view = (device_t->device).createBufferView(info);
                return {*this,core::api::result_t(0u)};
            };
#endif
        };
    };
};
