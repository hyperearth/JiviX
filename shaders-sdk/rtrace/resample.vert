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
layout (location = 0) out vec4 gColor;
layout (location = 1) out vec4 gSample;

// 
void main() {
    //const uint idx = gl_VertexIndex;
    const ivec2 size = imageSize(writeImages[0]  );
    const ivec2 f2fx = ivec2(gl_VertexIndex, gl_InstanceIndex);

    // 
    const vec4 positions = imageLoad(writeImages[SAMPLING],f2fx); // from previous frame 
    const vec4 diffcolor = imageLoad(writeImages[DIFFUSED],f2fx); // 

    // 
    gl_PointSize = 0; gColor = 0.f.xxxx;
    if (diffcolor.w > 0.f) { // set into current 
        gl_Position = vec4(vec4(positions.xyz,1.f) * modelview, 1.f) * projection, gl_PointSize = 1, gl_Position.y *= -1.f;
        gColor = clamp(diffcolor, 0.001f, 10000000.f);
        gSample = vec4(gl_Position.xyz / gl_Position.w,1.f);
    };
};
