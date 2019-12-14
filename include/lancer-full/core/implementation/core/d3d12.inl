#pragma once

#ifdef USE_D3D12

#include <d3d12.h>
#ifndef D3D12_ENABLED
#define D3D12_ENABLED
#endif

#ifndef D3D12_HPP_ENABLED
#define D3D12_HPP_ENABLED
#endif

#endif

namespace svt {
#ifdef USE_D3D12
#define api d3d12
#endif

#ifdef USE_D3D12
    namespace api {
        namespace classes {

        };
    };

    namespace core {
        namespace api {
            using result_t = HRESULT;
        };
    };
#endif

#if defined(USE_D3D12) && !defined(USE_VULKAN)
    using namespace api::classes;
#endif

};
