# LancER

[![License](https://img.shields.io/badge/License-BSD%203--Clause-blue.svg)](https://opensource.org/licenses/BSD-3-Clause)

Real-Time oriented Vulkan API rendering and computing library...
Writing for C++20 prior...
Planned to beat world record as most perfect and most long-time development product...

## Additional Links

- [VKt/VKh](https://github.com/world8th/vkt), simpler and unified helpers and mini-framework for Vulkan API...
- [MineRTX](https://github.com/hyperearth/MineRTX), spin-off project for add RTX ray tracing for Minecraft (Java Edition)...
- [hE-ART](https://github.com/hyperearth/hE-ART), terminal of projects, central...
- [TankAPI](https://github.com/world8th/vRt/tree/TankAPI) itself, moved into as part of vRt project. 
- [Hello triangle](https://github.com/helixd-2k18/VK_KHR_ray_tracing) example, based on [VK_KHR_ray_tracing](https://github.com/helixd-2k18/VK_KHR_ray_tracing) extension, modified

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

Date:[09.04.2020]:[helix.d@IMN]
```

### TODO Lists (Gen 3 and 3.1)

- [x] Diffuse layer with global illumination (Path Tracing)
- [x] Reflection layers (Partially Reprojected)
- [x] Support of `VK_KHR_ray_tracing`
- [x] Materials support (Beta)
- [x] Glossy reflections (Beta)
- [x] Transparency layers (Alpha)
- [ ] Better command construction
- [ ] OpenGL compatibility (Transform Feedback and Materials)
- [ ] New optional rasterization base (writing to dedicated buffers)
- [ ] Adaptive denoise (i.e. depended by trace length and roughness)
- [ ] Variable shading rate (faster ray-tracing)
- [ ] Anti-aliasing and better denoise
- [ ] Refraction support 
- [ ] Water shader (for games)
- [ ] Optifine support (Minecraft)
- [ ] Nova renderer support (Minecraft)
- [ ] ReShade extensions support


## Authors P.S.

- helix.d@IMN
- RTX@NVIDIA
