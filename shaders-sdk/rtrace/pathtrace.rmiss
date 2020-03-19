#version 460 core // #
#extension GL_GOOGLE_include_directive : require
#extension GL_EXT_ray_tracing           : require
#include "./driver.glsl"

rayPayloadInEXT RayPayloadData PrimaryRay;

void main() {
    vec3 raydir = gl_WorldRayDirectionEXT;
    vec3 origin = gl_WorldRayOriginEXT;
    
    PrimaryRay.position  = vec4(0.f);
    PrimaryRay.normals   = vec4(0.f);
    PrimaryRay.fdata.xyz = vec3(0.f,0.f,10000.f); //gl_HitTEXT
    PrimaryRay.udata.xyz = uvec3(0u);
};
