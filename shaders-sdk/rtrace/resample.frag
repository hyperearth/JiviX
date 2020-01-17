#version 460 core
#extension GL_GOOGLE_include_directive  : require
#extension GL_EXT_scalar_block_layout   : require
#extension GL_EXT_shader_realtime_clock : require
#extension GL_EXT_nonuniform_qualifier  : require

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
layout (location = 0) in vec4 gColor;
//layout (location = 1) in vec4 gSample;
layout (location = DIFFUSED) out vec4 oDiffused;
layout (location = SAMPLING) out vec4 oSampling;

// 
void main() {
    const ivec2 size = imageSize(writeImages[0]  );
    const ivec2 f2fx = ivec2(gl_FragCoord.xy);
    //const vec4 positions = imageLoad(writeImages[SAMPLING],f2fx); // re-set position
    const vec4 positions = vec4(divW(vec4((f2fx/size)*2.f-1.f,gl_FragCoord.z,1.f)*projectionInv),1.f); // re-set position

    oDiffused = gColor;
    oSampling = positions; // pick up as least
};
