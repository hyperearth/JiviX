#version 460 core
#extension GL_GOOGLE_include_directive  : require
#extension GL_EXT_scalar_block_layout   : require
#extension GL_EXT_shader_realtime_clock : require

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
layout (location = 0) in vec4 gPosition;
layout (location = 1) in vec4 gNormals;
layout (location = 2) in vec4 gTexcoords;
layout (location = 3) in vec4 gTangents;
layout (location = 4) flat in ivec4 gIndexes;

// 
layout (location = 0) out vec4 colors;
layout (location = 1) out vec4 normals;
layout (location = 2) out vec4 txnormals;
layout (location = 3) out vec4 parameters;
layout (location = 4) out vec4 samples;

// 
void main() {
    colors = vec4(texture(textures[0],gTexcoords.xy).xyz,1.f);
    normals = gNormals;
    txnormals = gNormals;
    samples = gPosition;
    parameters = intBitsToFloat(gIndexes);
};
