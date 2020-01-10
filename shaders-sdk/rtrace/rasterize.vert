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

// Left Oriented
layout (location = 0) in vec3 iPosition;
layout (location = 2) in vec4 iTexcoords;
layout (location = 1) in vec4 iNormals;
layout (location = 3) in vec4 iTangents;

// Right Oriented
layout (location = 0) out vec4 gTexcoords;
layout (location = 1) out vec4 gPosition;
layout (location = 2) out vec4 gNormals;
layout (location = 3) out vec4 gTangents;

// 
void main() { // Cross-Lake
    gTexcoords = iTexcoords;
    gPosition = vec4(iPosition,1.f);
    gTangents = iTangents;
    gNormals = iNormals;

    // Final Output 
    gl_Position = vec4(vec4(iPosition,1.f) * modelview, 1.f) * projection;
};
