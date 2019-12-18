#pragma once

#include "./classes/API/types.hpp"
#include "./classes/API/unified/buffer.hpp"
#include "./classes/API/unified/vector.hpp"
#include "./factory/API/unified/buffer_view.hpp"

namespace svt {
    namespace api {
        namespace classes {

            // TODO: auto format for types and template<T>
            class buffer_view { public:
                buffer_view(const buffer_view& buffer_view) : buffer_view_t(buffer_view) { };

                // TODO: move into `.cpp` file
                buffer_view& operator=(const buffer_view &buffer_view) { 
                    this->buffer_view_t = buffer_view;
                    this->vector_t = buffer_view;
                    this->device_t = buffer_view;
                    return *this;
                };
                
                // TODO: move into `.cpp` file
                operator const stu::vector&() const { return vector_t; };
                operator const stu::device&() const { return device_t; };
                operator const stu::buffer_view&() const { return buffer_view_t; };
                operator stu::vector&() { return vector_t; };
                operator stu::device&() { return device_t; };
                operator stu::buffer_view&() { return buffer_view_t; };

                // TODO: move into `.cpp` file
                api::factory::buffer_view_t* operator->() { return &(*this->buffer_view_t); };
                const api::factory::buffer_view_t* operator->() const { return &(*this->buffer_view_t); };

                // TODO: complete that versions
                template<class T = float>
                svt::core::handle_ref<buffer_view,core::api::result_t> create(const vector<T>& vector);
                svt::core::handle_ref<buffer_view,core::api::result_t> create(const stu::vector& vector, const format& format = format::t_undefined);

                // TODO: move into `.cpp` file
                // TODO: better type for dual-version-returnement
                std::pair<buffer_view&, description_handle&> write_into_description(description_handle& handle, const uint32_t& idx = 0u) {
                    handle.offset<core::api::buffer_view_t>(idx) = (*this->buffer_view_t);
                    return {*this, handle};
                };

            protected: friend buffer_view;
                stu::buffer_view buffer_view_t = {};
                stu::vector vector_t = {};
                stu::device device_t = {};
            };

        };
    };
};
