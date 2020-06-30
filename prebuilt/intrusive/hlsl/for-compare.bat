spirv-dis                      denoise.comp.spv >> denoise.comp.dis
spirv-cross --vulkan-semantics denoise.comp.spv >> denoise.comp.glsl
spirv-dis                       transform.vert.spv >> transform.vert.dis
spirv-cross --vulkan-semantics  transform.vert.spv >> transform.vert.glsl
spirv-dis                       rasterize.vert.spv >> rasterize.vert.dis
spirv-cross --vulkan-semantics  rasterize.vert.spv >> rasterize.vert.glsl
pause
