#pragma one
#include "../core/core.hpp"
#include "../static/API/buffer.hpp"
#include "../static/API/device.hpp"

namespace svt {


    class buffer { 
        protected: 
            std::shared_ptr<data::buffer_t> buffer_t = {};
            std::shared_ptr<data::device_t> device_t = {};

        public: 
            // structs by C++20
            struct create_info {
        
            };

            buffer(const buffer& buffer_t) : buffer_t(buffer_t), device_t(buffer_t) {};
            buffer(const std::shared_ptr<data::buffer_t>& buffer_t = {}) : buffer_t(buffer_t) {};
            buffer(const std::shared_ptr<data::device_t>& device_t = {}, const std::shared_ptr<data::buffer_t>& buffer_t = {}) : buffer_t(buffer_t), device_t(device_t) {};

            // 
            operator std::shared_ptr<data::buffer_t>&() { return buffer_t; };
            operator std::shared_ptr<data::device_t>&() { return device_t; };
            operator const std::shared_ptr<data::buffer_t>&() const { return buffer_t; };
            operator const std::shared_ptr<data::device_t>&() const { return device_t; };

            // 
            handle_ref<buffer,api::Result> create(const create_info& info = {});

    };

};
