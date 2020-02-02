#version 460 core // #
#extension GL_GOOGLE_include_directive : require
#extension GL_NV_ray_tracing           : require
#include "./driver.glsl"

rayPayloadInNV RayPayloadData PrimaryRay;
hitAttributeNV vec2 baryCoord;

#define IndexU8 1000265000
#define IndexU16 0
#define IndexU32 1

void main() {
    const uint globalInstanceID = gl_InstanceID;

    const mat3x4 matras = mat3x4(vec4(1.f,0.f.xxx),vec4(0.f,1.f,0.f.xx),vec4(0.f.xx,1.f,0.f));
    const mat4x4 matra4 = mat4x4(matras[0],matras[1],matras[2],vec4(0.f.xxx,1.f));

    const mat3x4 transp = rtxInstances[globalInstanceID].transform;
    const mat4x4 trans4 = mat4x4(transp[0],transp[1],transp[2],vec4(0.f.xxx,1.f));

    const mat4x4 normalTransform = (inverse(transpose(matra4)));
    const mat4x4 normInTransform = (inverse(transpose(trans4)));

    uvec3 idx3 = uvec3(gl_PrimitiveID*3u+0u,gl_PrimitiveID*3u+1u,gl_PrimitiveID*3u+2u);

    // type definition
    int IdxType = int(meshInfo[gl_InstanceCustomIndexNV].indexType)-1;
    if (IdxType == IndexU8 ) { idx3 = uvec3(load_u8 (idx3.x*1u, 8u, gl_InstanceCustomIndexNV),load_u32(idx3.y*1u, 8u, gl_InstanceCustomIndexNV),load_u32(idx3.z*1u, 8u, gl_InstanceCustomIndexNV)); };
    if (IdxType == IndexU16) { idx3 = uvec3(load_u16(idx3.x*2u, 8u, gl_InstanceCustomIndexNV),load_u16(idx3.y*2u, 8u, gl_InstanceCustomIndexNV),load_u16(idx3.z*2u, 8u, gl_InstanceCustomIndexNV)); };
    if (IdxType == IndexU32) { idx3 = uvec3(load_u32(idx3.x*4u, 8u, gl_InstanceCustomIndexNV),load_u32(idx3.y*4u, 8u, gl_InstanceCustomIndexNV),load_u32(idx3.z*4u, 8u, gl_InstanceCustomIndexNV)); };

    // mesh definition 
    PrimaryRay.position.xyz = vec4(triangulate(idx3, 0u, gl_InstanceCustomIndexNV,vec3(1.f-baryCoord.x-baryCoord.y,baryCoord)).xyz,1.f) * transp;
    PrimaryRay.texcoords    = vec4(triangulate(idx3, 1u, gl_InstanceCustomIndexNV,vec3(1.f-baryCoord.x-baryCoord.y,baryCoord)).xyz,0.f);
    PrimaryRay.normals      = vec4(triangulate(idx3, 2u, gl_InstanceCustomIndexNV,vec3(1.f-baryCoord.x-baryCoord.y,baryCoord)).xyz,0.f);
    PrimaryRay.tangents     = vec4(triangulate(idx3, 3u, gl_InstanceCustomIndexNV,vec3(1.f-baryCoord.x-baryCoord.y,baryCoord)).xyz,0.f);
    PrimaryRay.fdata.xyz    = vec3(baryCoord, gl_HitTNV);
    PrimaryRay.udata        = uvec4(idx3, gl_InstanceCustomIndexNV);

    const mat3x3 mc = mat3x3(
        vec4(get_vec4(idx3[0], 0u, gl_InstanceCustomIndexNV).xyz,1.f)*transp,
        vec4(get_vec4(idx3[1], 0u, gl_InstanceCustomIndexNV).xyz,1.f)*transp,
        vec4(get_vec4(idx3[2], 0u, gl_InstanceCustomIndexNV).xyz,1.f)*transp
    );

    const mat3x3 tx = mat3x3(
        vec4(get_vec4(idx3[0], 1u, gl_InstanceCustomIndexNV).xyz,1.f),
        vec4(get_vec4(idx3[1], 1u, gl_InstanceCustomIndexNV).xyz,1.f),
        vec4(get_vec4(idx3[2], 1u, gl_InstanceCustomIndexNV).xyz,1.f)
    );

    const vec3 dp1 = mc[1] - mc[0], dp2 = mc[2] - mc[0];
    const vec3 tx1 = tx[1] - tx[0], tx2 = tx[2] - tx[0];
    const float coef = 1.f / (tx1.x * tx2.y - tx2.x * tx1.y);
    const vec3 tangent = (dp1.xyz * tx2.yyy + dp2.xyz * tx1.yyy) * coef;

    // normals 
    if (dot(PrimaryRay.normals.xyz,PrimaryRay.normals.xyz) > 0.001f && hasNormal(meshInfo[gl_InstanceCustomIndexNV])) {
        PrimaryRay.normals.xyz = normalize((PrimaryRay.normals * normalTransform * normInTransform).xyz);
    } else {
        PrimaryRay.normals.xyz = normalize(cross(mc[1].xyz-mc[0].xyz,mc[2].xyz-mc[0].xyz));
    };

    // tangents
    if (dot(PrimaryRay.tangents.xyz,PrimaryRay.tangents.xyz) > 0.001f && hasTangent(meshInfo[gl_InstanceCustomIndexNV])) {
        PrimaryRay.tangents.xyz = normalize((PrimaryRay.tangents * normalTransform * normInTransform).xyz);
    } else {
        PrimaryRay.tangents.xyz = tangent;
    };

    //PrimaryRay.normals.xyz = normalize(cross(mc[1].xyz-mc[0].xyz,mc[2].xyz-mc[0].xyz));
};
