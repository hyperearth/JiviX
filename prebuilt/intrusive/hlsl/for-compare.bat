spirv-dis                      denoise.cs.spv >> denoise.cs.dis
spirv-cross --vulkan-semantics denoise.cs.spv >> denoise.cs.glsl
spirv-dis                       transform.vs.spv >> transform.vs.dis
spirv-cross --vulkan-semantics  transform.vs.spv >> transform.vs.glsl
spirv-dis                       rasterize.vs.spv >> rasterize.vs.dis
spirv-cross --vulkan-semantics  rasterize.vs.spv >> rasterize.vs.glsl
pause
