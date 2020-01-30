#version 460 core // #
#extension GL_GOOGLE_include_directive : require
#extension GL_NV_ray_tracing           : require
#include "./driver.glsl"

rayPayloadInNV RayPayloadData PrimaryRay;

void main() {
    vec3 raydir = gl_WorldRayDirectionNV;
    vec3 origin = gl_WorldRayOriginNV;
    
    PrimaryRay.position  = vec4(0.f);
    PrimaryRay.texcoords = vec4(0.f);
    PrimaryRay.normals   = vec4(0.f);
    PrimaryRay.fdata.xyz = vec3(0.f,0.f,10000.f); //gl_HitTNV
    PrimaryRay.udata.xyz = uvec3(0u);
};
