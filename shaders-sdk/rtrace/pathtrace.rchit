#version 460 core
#extension GL_NV_ray_tracing : require
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

layout ( location = 0 ) rayPayloadInNV RayPayloadData PrimaryRay;
                        hitAttributeNV vec2 baryCoord;

void main() {
    const uvec3 indices = uvec3(imageLoad(indices,int(gl_PrimitiveID*3u+0u)).r, imageLoad(indices,int(gl_PrimitiveID*3u+1u)).r, imageLoad(indices,int(gl_PrimitiveID*3u+2u)).r);
    PrimaryRay.position  = triangulate(indices, 0u, gl_InstanceCustomIndexNV,vec3(1.f-baryCoord.x-baryCoord.y,baryCoord));
    PrimaryRay.texcoords = triangulate(indices, 1u, gl_InstanceCustomIndexNV,vec3(1.f-baryCoord.x-baryCoord.y,baryCoord));
    PrimaryRay.normals   = triangulate(indices, 2u, gl_InstanceCustomIndexNV,vec3(1.f-baryCoord.x-baryCoord.y,baryCoord));
    PrimaryRay.fdata.xyz = vec3(baryCoord, gl_HitTNV);
    PrimaryRay.udata.xyz = indices;
};
