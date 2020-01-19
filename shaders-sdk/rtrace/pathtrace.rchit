#version 460 core
#extension GL_NV_ray_tracing : require
#extension GL_GOOGLE_include_directive  : require
#include "./driver.glsl"

layout (location = 0) rayPayloadInNV RayPayloadData PrimaryRay;
hitAttributeNV vec2 baryCoord;

void main() {
    uvec3 idx3 = uvec3(gl_PrimitiveID*3u+0u,gl_PrimitiveID*3u+1u,gl_PrimitiveID*3u+2u);
    if (meshInfo[gl_InstanceCustomIndexNV].hasIndex == 1) {
        idx3 = uvec3(imageLoad(indices,int(gl_PrimitiveID*3u+0u)).r, imageLoad(indices,int(gl_PrimitiveID*3u+1u)).r, imageLoad(indices,int(gl_PrimitiveID*3u+2u)).r);
    };
    PrimaryRay.position  = triangulate(idx3, 0u, gl_InstanceCustomIndexNV,vec3(1.f-baryCoord.x-baryCoord.y,baryCoord));
    PrimaryRay.texcoords = triangulate(idx3, 1u, gl_InstanceCustomIndexNV,vec3(1.f-baryCoord.x-baryCoord.y,baryCoord));
    PrimaryRay.normals   = triangulate(idx3, 2u, gl_InstanceCustomIndexNV,vec3(1.f-baryCoord.x-baryCoord.y,baryCoord));
    //PrimaryRay.udata     = uvec4(idx3,gl_PrimitiveID/int(meshInfo[gl_InstanceCustomIndexNV].prmCount));
    //PrimaryRay.udata     = uvec4(idx3,gl_PrimitiveID/gl_InstanceID);
    PrimaryRay.udata     = uvec4(idx3,gl_PrimitiveID/gl_HitKindNV);
    PrimaryRay.fdata.xyz = vec3(baryCoord, gl_HitTNV);
};
