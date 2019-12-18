#pragma once

#include "./factory/API/unified/device.hpp"
#include "./factory/API/unified/descriptor_set.hpp"
#include "./classes/API/unified/descriptor_set.hpp"


namespace svt {
    namespace api {
        namespace classes {
#ifdef USE_VULKAN
            template<class T>
            inline description_handle descriptor_set::push_description( const description_entry& entry_ ) { // Un-Safe API again
                const uintptr_t pt0 = heap.size();
                heap.resize(pt0+sizeof(T)*entry_.descriptor_count, 0u);
                entries.push_back(vk::DescriptorUpdateTemplateEntry{entry_.dst_binding,entry_.dst_array_element,entry_.descs_count,(vk::DescriptorType)(entry_),pt0,sizeof(T)});
                //return (T*)(&heap[pt0]);
                return cpu_handle{ &entries.back(), &heap.back() };
            };
#endif
        };
    };
};
