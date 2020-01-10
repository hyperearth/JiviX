#version 460 core
#extension GL_NV_ray_tracing : require
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




layout ( location = 0 ) rayPayloadInNV RayPayloadData PrimaryRay;


void main() {
    // PrimaryRay.fdata = vec4();

    vec3 raydir = gl_WorldRayDirectionNV;
    vec3 origin = gl_WorldRayOriginNV;
    
    PrimaryRay.fdata.xyz = vec3(0.f,0.f,10000.f); //gl_HitTNV

    //float depth = raySphereIntersect(origin,raydir,vec3(10.f,10.f,10.f),2.f);
    

    
    //if (depth > 0.f) {
    //    PrimaryRay.udata.x = 1u;
    //    PrimaryRay.fdata.xyz = vec3(10.f, 10.f, 10.f);
    //} else {
    //    PrimaryRay.udata.x = 1u;
    //    PrimaryRay.fdata.xyz = vec3(0.9f, 0.96f, 0.99f) * 0.5f;
    //};

};
