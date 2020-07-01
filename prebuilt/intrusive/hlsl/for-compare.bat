spirv-dis                      denoise.comp.spv >> source/denoise.comp.dis
spirv-cross --vulkan-semantics denoise.comp.spv >> source/denoise.comp.glsl
spirv-dis                       transform.vert.spv >> source/transform.vert.dis
spirv-cross --vulkan-semantics  transform.vert.spv >> source/transform.vert.glsl
spirv-dis                       transform.geom.spv >> source/transform.geom.dis
spirv-cross --vulkan-semantics  transform.geom.spv >> source/transform.geom.glsl
spirv-dis                       rasterize.vert.spv >> source/rasterize.vert.dis
spirv-cross --vulkan-semantics  rasterize.vert.spv >> source/rasterize.vert.glsl
pause
