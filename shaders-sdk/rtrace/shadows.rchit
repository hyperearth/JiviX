#version 460 core
#extension GL_NV_ray_tracing : require
#extension GL_GOOGLE_include_directive  : require
#include "./driver.glsl"

layout (location = 1) rayPayloadInNV RayPayloadData PrimaryRay;
hitAttributeNV vec2 baryCoord;

void main() {
    const uvec3 indices = uvec3(imageLoad(indices,int(gl_PrimitiveID*3u+0u)).r, imageLoad(indices,int(gl_PrimitiveID*3u+1u)).r, imageLoad(indices,int(gl_PrimitiveID*3u+2u)).r);
    PrimaryRay.position  = triangulate(indices, 0u, gl_InstanceCustomIndexNV,vec3(1.f-baryCoord.x-baryCoord.y,baryCoord));
    PrimaryRay.texcoords = triangulate(indices, 1u, gl_InstanceCustomIndexNV,vec3(1.f-baryCoord.x-baryCoord.y,baryCoord));
    PrimaryRay.normals   = triangulate(indices, 2u, gl_InstanceCustomIndexNV,vec3(1.f-baryCoord.x-baryCoord.y,baryCoord));
    PrimaryRay.fdata.xyz = vec3(baryCoord, gl_HitTNV);
    PrimaryRay.udata.xyz = indices;
};
