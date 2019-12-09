#pragma once

#include <d3d12.h>

#ifndef D3D12_ENABLED
#define D3D12_ENABLED
#endif

namespace svt {
#ifdef USE_D3D12
#define api d3d12
#endif

#ifdef USE_D3D12
    //namespace api { using namespace vk; }; // safer version 
    namespace api {
        using namespace svt;
        namespace core {
            using result_t = HRESULT;
        };
    };
    namespace core {
        
    };
#endif

#if defined(USE_D3D12) && !defined(USE_VULKAN)
    using namespace api;
#endif

};
