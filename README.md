# JiviX

[![License](https://img.shields.io/badge/License-Apache%202.0-blue.svg)](https://opensource.org/licenses/Apache-2.0)

Real-Time oriented Vulkan API rendering and computing library...
Writing for C++20 prior...
Planned to beat world record as most perfect and most long-time development product...

## Project Reset...

A new project "[ValerA](https://github.com/helixd2s/ValerA)" is in development, a more classic tracing by our standards, and will be more efficiently supported in GPU Ampere ...


### Planned Features: 

- Fully refactored and rewrited code (and shorter)
- More performance and, probably, multi-threading
- Lower-Level API (than JiviX)
- Object and Data oriented
- Partial modularity and faster compilation
- No CMAKE, more direct compilation
- Multiple compute shaders rendering (include Ray Query)
- More and efficient ray-tracing 
- DirectX 12 Ultimate support and HLSL (under consideration)
- RDNA-2 support
- ...


## Additional Links

- [VKt/VKh](https://github.com/world8th/vkt), simpler and unified helpers and mini-framework for Vulkan API... **Required** for the project!
- [MineVKR](https://github.com/hyperearth/MineVKR), spin-off project for add RTX ray tracing for Minecraft (Java Edition)...
- [ValerA](https://github.com/helixd2s/ValerA) itself, XT project for replacement `vRt`. 
- [My Discord](https://discord.gg/NqjBJsG), but currently we have no focus for them... 
- [My Puzzle](https://vk.cc/afiR3v) for psychologists, psychoanalysts, psychotherapists and students ...


## Tech Spec

```MD
- Architecture    : [ARC:F2020]
- Portfolio       : Work in Progress
- License         : Apache-2.0 (Currently)
- Execute Day     : [2021:Q1]
- Commercial      : Under Consideration
- Project Type    : Experimental, Personal
- Path-Tracing    : Diffuse noiseless (almost), reflection...
- Target Games    : Minecraft Java Edition, Neverball...
- Graphical API   : Vulkan API (VK_KHR_ray_tracing)
- General To Do   : Add Transform Feedback support
- Used Libraries  : Vulkan-HPP, VKt/VKh, 
- Replacement Of  : Project "Satellite" ("engine", 2017)
- Shaders Version : v2.0
- SH&T of Шindows : R.I.P. for Visual Studio 2019 Clang-CL support... (also, broken C++20 support by MSVC)
- Thank You!      : https://developer.nvidia.com/vulkan-driver (for 451.74 version with fixes...) 

Date:[29.06.2020]:[helix.d@IMN]
```


### Visual Studio 2019 RIP... NO possible to resolve that problem... 


### TODO Lists (Gen 3...)

- [x] Diffuse layer with global illumination (Path Tracing)
- [x] Reflection layers (Partially Reprojected)
- [x] Support of `VK_KHR_ray_tracing` (used `GLSL_EXT_ray_query` in Compute shader)
- [x] Materials support (Beta)
- [x] Glossy reflections (Beta)
- [x] Transparency layers (Alpha)
- [x] Better command construction (Alpha)
- [x] Dedicated array of buffer view, instead of `BindingID*` itself (Alpha)
- [x] Transform Feedback for Tesselation and Geometry shaders support (Alpha)
- [x] Rise quads support (Alpha)
- [x] Geometry shader in resampling (Reserved)
- [x] HDRI background image and equirectangular projection (Alpha)
- [x] Support VKT-3 for 12.05.2020 and later... 
- [x] Checker-board ray-tracing optimization...
- [x] New optional rasterization base (writing to dedicated buffers)
- [x] Reorganized and better ray-tracing shaders
- [ ] Adaptive denoise (i.e. depended by trace length and roughness)
- [ ] OpenGL compatibility`**` (Transform Feedback and Materials)
- [ ] Advanced FPS counter (TODO)
- [ ] Some unit-tests`***` 
- [ ] Water shader (for games)
- [ ] Optifine support (Minecraft)
- [ ] Nova renderer support (Minecraft)
- [ ] Immersive Portals mod support (Minecraft, In-Future)


### WHY Ray Query doesn't working?

- Broken support by NVIDIA
- Forced to be opaque objects
- Too big workgroup size
- Wrong shaders (needs HLSL)
- Needs DirectX 12 Ultimate ([LaVaX](https://github.com/helixd2s/LaVaX-Overview))


### TODO Lists (Black Edition and Gen-4)

- [x] Fallback to ray-tracing shaders
- [x] Unified shaders (for [LaVaX](https://github.com/helixd2s/LaVaX-Overview)), tested only GLSL
- [ ] Refactor for DirectX 12 Ultimate compatibility (for [LaVaX](https://github.com/helixd2s/LaVaX-Overview))
- [ ] Support for [LaVaX](https://github.com/helixd2s/LaVaX-Overview)
- [ ] Using Volk instead of XVK (VKT-4 )
- [ ] RDNA-2 support
- [ ] HLSL shaders (Shader Model 6.5 and higher)
- [ ] Variable shading rate (faster ray-tracing, currently 1/2 only)
- [ ] Anti-aliasing and advanced denoise (already reserved for make)
- [ ] Fog and better transparency support 
- [ ] Refraction support
- [ ] ReShade extensions support
- [ ] DLSS 2.0?


### DirectX 12 Ultimate? (Gen-5?)

- [ ] Multi-Threading
- [ ] Advanced Portability
- [ ] Advanced Hardware Acceleration


### Remarks...

- `*  : Means shared array of buffer views for complex Node class`
- `** : Use OpenGL version of transform feedback, uses GL import, NOT supported by Ubuntu currently`
- `***: There is no priority, we prefer live testing and have MVP soon as possible`
- `MVP: Minimal Valuable Product`


### About Minecraft Mod Idea... 

- Vulkan API can import OpenGL buffer data only using copy, OpenGL can import memory directly from Vulkan API. 
- Minecraft can use Vulkan API using JNI, JavaCPP, LWJGL-3... 
- Always communication using LONG type in Java and uint64_t in C++ 
- Java may have conflicts with `VkDeviceOrHostAddressKHR` or `VkDeviceOrHostAddressConstKHR` due same LONG type... 
- Any native pointers in Java are LONG type... 


### Used Symbolic Links!

Clone with `git clone -c core.symlinks=true https://github.com/hyperearth/LancER.git --recursive`


## Authors P.S.

- helix.d@IMN
- RTX@NVIDIA

