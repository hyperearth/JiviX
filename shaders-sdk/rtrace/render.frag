#version 460 core
#extension GL_GOOGLE_include_directive          : require
#extension GL_EXT_scalar_block_layout           : require
#extension GL_EXT_shader_realtime_clock         : require
#extension GL_EXT_samplerless_texture_functions : require

#extension GL_EXT_shader_explicit_arithmetic_types         : require
#extension GL_EXT_shader_explicit_arithmetic_types_int8    : require
#extension GL_EXT_shader_explicit_arithmetic_types_int16   : require
#extension GL_EXT_shader_explicit_arithmetic_types_int32   : require
#extension GL_EXT_shader_explicit_arithmetic_types_int64   : require
#extension GL_EXT_shader_explicit_arithmetic_types_float16 : require
#extension GL_EXT_shader_explicit_arithmetic_types_float32 : require
#extension GL_EXT_shader_explicit_arithmetic_types_float64 : require
#extension GL_EXT_shader_subgroup_extended_types_int8      : require
#extension GL_EXT_shader_subgroup_extended_types_int16     : require
#extension GL_EXT_shader_subgroup_extended_types_int64     : require
#extension GL_EXT_shader_subgroup_extended_types_float16   : require
#extension GL_EXT_shader_16bit_storage                     : require
#extension GL_EXT_shader_8bit_storage                      : require

precision highp float;
precision highp int;
#include "./index.glsl"

// 
layout ( location = 0 ) in vec2 vcoord;
layout ( location = 0 ) out vec4 uFragColor;

// 
void main() {
    const vec2 size = imageSize(writeImages[DIFFUSED_FLIP1]);
    vec2 coord = gl_FragCoord.xy; coord.y = size.y - coord.y;
    vec4 samples = max(imageLoad(writeImages[DIFFUSED_FLIP1],ivec2(coord)),0.0001f.xxxx); samples.xyz /= samples.w;
    if (samples.w >= 0.001f) uFragColor = vec4(samples.xyz*vec3(1.f,1.f,1.f),1.f);
    //if (samples.w >= 0.001f) uFragColor = vec4(samples.xyz*texelFetch(frameBuffers[COLORING],ivec2(coord),0).xyz,1.f);
};
