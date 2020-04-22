# LancER

[![License](https://img.shields.io/badge/License-BSD%203--Clause-blue.svg)](https://opensource.org/licenses/BSD-3-Clause)

Real-Time oriented Vulkan API rendering and computing library...
Writing for C++20 prior...
Planned to beat world record as most perfect and most long-time development product...


## Additional Links

- [VKt/VKh](https://github.com/world8th/vkt), simpler and unified helpers and mini-framework for Vulkan API... **Required** for the project!
- [MineRTX](https://github.com/hyperearth/MineRTX), spin-off project for add RTX ray tracing for Minecraft (Java Edition)...
- [hE-ART](https://github.com/hyperearth/hE-ART), terminal of projects, central...
- [TankAPI](https://github.com/world8th/vRt/tree/TankAPI) itself, moved into as part of vRt project. 
- [My Discord](https://discord.gg/NqjBJsG), but currently we have no focus for them... 
- [My Puzzle](https://vk.cc/afiR3v) for psychologists, psychoanalysts, psychotherapists and students ...


## Tech Spec

```MD
- Architecture    : [ARC:F2020]
- Portfolio       : Work in Progress
- License         : BSD-3 (Currently)
- Execute Day     : [2021:Q1]
- Support OS      : Windows 10 (2004 or Newer)
- Support HW      : RTX 2070 or familiar...
- Commercial      : Under Consideration
- Project Type    : Experimental, Personal
- Path-Tracing    : Diffuse noiseless (almost), reflection...
- Target Games    : Minecraft Java Edition, Neverball...
- Graphical API   : Vulkan API (VK_KHR_ray_tracing)
- Code Language   : C++20, [Visual Studio 2019] support
- General To Do   : Add Transform Feedback support
- Testing On GPU  : NVIDIA GeForce RTX 2070 (Gigabyte)
- Used Libraries  : Vulkan-HPP, VKt/VKh, 
- Replacement Of  : Project "Satellite" ("engine", 2017)

Date:[09.04.2020]:[helix.d@IMN]
```


### TODO Lists (Gen 3 and Next)

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
- [ ] Adaptive denoise (i.e. depended by trace length and roughness)
- [ ] New optional rasterization base (writing to dedicated buffers)
- [ ] HDRI background image and equirectangular projection (TODO)
- [ ] Advanced FPS counter (TODO)
- [ ] OpenGL compatibility`**` (Transform Feedback and Materials)
- [ ] Variable shading rate (faster ray-tracing)
- [ ] Anti-aliasing and better denoise
- [ ] Refraction support
- [ ] Some unit-tests`***`
- [ ] Water shader (for games)
- [ ] Optifine support (Minecraft)
- [ ] Nova renderer support (Minecraft)
- [ ] ReShade extensions support


### Remarks...

- `*  : Means shared array of buffer views for complex Node class`
- `** : Use OpenGL version of transform feedback, uses GL import`
- `***: There is no priority, we prefer live testing and have MVP soon as possible`
- `MVP: Minimal Valuable Product`


### About Minecraft Mod Idea... 

- Vulkan API can import OpenGL buffer data only using copy, OpenGL can import memory directly from Vulkan API. 
- Minecraft can use Vulkan API using JNI, JavaCPP, LWJGL-3... 
- Always communication using LONG type in Java and uint64_t in C++ 
- Java may have conflicts with `VkDeviceOrHostAddressKHR` or `VkDeviceOrHostAddressConstKHR` due same LONG type... 
- Any native pointers in Java are LONG type... 


## Authors P.S.

- helix.d@IMN
- RTX@NVIDIA

