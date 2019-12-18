#pragma once

#include "./factory/API/unified/device.hpp"
#include "./factory/API/unified/descriptor_set.hpp"
#include "./classes/API/unified/descriptor_set.hpp"


namespace svt {
    namespace api {
        namespace classes {
#ifdef USE_VULKAN
            template<class T>
            inline descriptor_set::cpu_handle descriptor_set::push_description( const descriptor_set::entry& entry_t ) { // Un-Safe API again
                const uintptr_t pt0 = heap.size();
                heap.resize(pt0+sizeof(T)*entry_t.descriptor_count, 0u);
                entries.push_back(vk::DescriptorUpdateTemplateEntry{entry_t.dst_binding,entry_t.dst_array_element,entry_t.descs_count,(vk::DescriptorType)(entry_t),pt0,sizeof(T)});
                //return (T*)(&heap[pt0]);
                return cpu_handle{ &entries.back(), &heap.back() };
            };
#endif
        };
    };
};
