#version 460 core
#extension GL_NV_ray_tracing            : require
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

layout (location = 0) rayPayloadInNV RayPayloadData PrimaryRay;

void main() {
    vec3 raydir = gl_WorldRayDirectionNV;
    vec3 origin = gl_WorldRayOriginNV;
    
    PrimaryRay.position  = vec4(0.f);
    PrimaryRay.texcoords = vec4(0.f);
    PrimaryRay.normals   = vec4(0.f);
    PrimaryRay.fdata.xyz = vec3(0.f,0.f,10000.f); //gl_HitTNV
    PrimaryRay.udata.xyz = uvec3(0u);
};
