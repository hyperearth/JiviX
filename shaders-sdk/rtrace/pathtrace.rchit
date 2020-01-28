#version 460 core // #
#extension GL_NV_ray_tracing : require
#extension GL_GOOGLE_include_directive  : require
#include "./driver.glsl"

rayPayloadInNV RayPayloadData PrimaryRay;
hitAttributeNV vec2 baryCoord;

void main() {
    const uint globalInstanceID = gl_InstanceID;
    
    const mat3x4 matras = mat3x4(vec4(1.f,0.f.xxx),vec4(0.f,1.f,0.f.xx),vec4(0.f.xx,1.f,0.f));
    const mat4x4 matra4 = mat4x4(matras[0],matras[1],matras[2],vec4(0.f.xxx,1.f));

    const mat3x4 transp = rtxInstances[globalInstanceID].transform;
    const mat4x4 trans4 = mat4x4(transp[0],transp[1],transp[2],vec4(0.f.xxx,1.f));

    const mat4x4 normalTransform = (inverse(transpose(matra4)));
    const mat4x4 normInTransform = (inverse(transpose(trans4)));

    uvec3 idx3 = uvec3(gl_PrimitiveID*3u+0u,gl_PrimitiveID*3u+1u,gl_PrimitiveID*3u+2u);
    if (meshInfo[gl_InstanceCustomIndexNV].hasIndex == 1) {
        idx3 = uvec3(imageLoad(indices,int(gl_PrimitiveID*3u+0u)).r, imageLoad(indices,int(gl_PrimitiveID*3u+1u)).r, imageLoad(indices,int(gl_PrimitiveID*3u+2u)).r);
    };

    PrimaryRay.position.xyz = vec4(triangulate(idx3, 0u, gl_InstanceCustomIndexNV,vec3(1.f-baryCoord.x-baryCoord.y,baryCoord)).xyz,1.f) * transp;
    PrimaryRay.texcoords    = vec4(triangulate(idx3, 1u, gl_InstanceCustomIndexNV,vec3(1.f-baryCoord.x-baryCoord.y,baryCoord)).xyz,0.f);
    PrimaryRay.normals      = vec4(triangulate(idx3, 2u, gl_InstanceCustomIndexNV,vec3(1.f-baryCoord.x-baryCoord.y,baryCoord)).xyz,0.f) * normalTransform * normInTransform;
    PrimaryRay.fdata.xyz    = vec3(baryCoord, gl_HitTNV);
    PrimaryRay.udata        = uvec4(idx3, gl_InstanceCustomIndexNV);
    PrimaryRay.normals.xyz  = normalize(PrimaryRay.normals.xyz);

    const mat3x4 mc = mat3x4(
        get_vec4(idx3[0], 0u, gl_InstanceCustomIndexNV),
        get_vec4(idx3[1], 0u, gl_InstanceCustomIndexNV),
        get_vec4(idx3[2], 0u, gl_InstanceCustomIndexNV)
    );

    //PrimaryRay.normals.xyz = normalize(cross(mc[1].xyz-mc[0].xyz,mc[2].xyz-mc[0].xyz));
};
