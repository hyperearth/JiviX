#pragma once

#include "./factory/API/unified/device.hpp"
#include "./factory/API/unified/descriptor_set.hpp"
#include "./classes/API/unified/descriptor_set.hpp"


namespace svt {
    namespace api {
        namespace classes {
#ifdef USE_VULKAN
            description_handle descriptor_set::push_description( const description_entry& entry_ ) {
                if (entry_.type == description_type::t_storage_buffer || entry_.type == description_type::t_uniform_buffer) {
                    return _push_description<core::api::buffer_region_t>(entry_);
                } else 
                if (entry_.type == description_type::t_storage_texel_buffer || entry_.type == description_type::t_uniform_texel_buffer) {
                    return _push_description<core::api::buffer_view_t>(entry_);
                } else 
                if (entry_.type == description_type::t_acceleration_structure) {
                    return _push_description<uintptr_t>(entry_); // TODO: Acceleration Structure Support
                } else {
                    return _push_description<core::api::image_desc_t>(entry_);
                };
            };
#endif
        };
    };
};
