#version 460 core // #
#extension GL_GOOGLE_include_directive : require
#extension GL_EXT_ray_tracing           : require
#include "./driver.glsl"

layout(location = 0) rayPayloadInEXT RayPayloadData PrimaryRay;
hitAttributeEXT vec2 baryCoord;

#define IndexU8 1000265000
#define IndexU16 0
#define IndexU32 1

mat4x4 regen4(in mat3x4 T) {
    return mat4x4(T[0],T[1],T[2],vec4(0.f.xxx,1.f));
}

mat3x3 regen3(in mat3x4 T) {
    return mat3x3(T[0].xyz,T[1].xyz,T[2].xyz);
}

vec4 mul4(in vec4 v, in mat3x4 M) {
    return vec4(v*M,1.f);
}

// TODO: add Any-Hit shaders for remove transparent intersections
void main() {
    const uint globalInstanceID = gl_InstanceID;

    // By Geometry Data
    mat3x4 matras = mat3x4(instances[gl_InstanceCustomIndexEXT].transform[gl_GeometryIndexEXT]);
    if (!hasTransform(meshInfo[gl_InstanceCustomIndexEXT])) {
        matras = mat3x4(vec4(1.f,0.f.xxx),vec4(0.f,1.f,0.f.xx),vec4(0.f.xx,1.f,0.f));
    };

    // By Instance Data
    const mat3x4 matra4 = rtxInstances[globalInstanceID].transform;

    // Native Normal Transform
    const mat3x3 normalTransform = inverse(transpose(regen3(matras)));
    const mat3x3 normInTransform = inverse(transpose(regen3(matra4)));

    // type definition
    int IdxType = int(meshInfo[gl_InstanceCustomIndexEXT].indexType)-1;
    uvec3 idx3 = uvec3(gl_PrimitiveID*3u+0u,gl_PrimitiveID*3u+1u,gl_PrimitiveID*3u+2u);
    if (IdxType == IndexU8 ) { idx3 = uvec3(load_u8 (idx3.x*1u, 8u, gl_InstanceCustomIndexEXT),load_u32(idx3.y*1u, 8u, gl_InstanceCustomIndexEXT),load_u32(idx3.z*1u, 8u, gl_InstanceCustomIndexEXT)); };
    if (IdxType == IndexU16) { idx3 = uvec3(load_u16(idx3.x*2u, 8u, gl_InstanceCustomIndexEXT),load_u16(idx3.y*2u, 8u, gl_InstanceCustomIndexEXT),load_u16(idx3.z*2u, 8u, gl_InstanceCustomIndexEXT)); };
    if (IdxType == IndexU32) { idx3 = uvec3(load_u32(idx3.x*4u, 8u, gl_InstanceCustomIndexEXT),load_u32(idx3.y*4u, 8u, gl_InstanceCustomIndexEXT),load_u32(idx3.z*4u, 8u, gl_InstanceCustomIndexEXT)); };

    // mesh definition 
    PrimaryRay.position  = mul4(mul4(vec4(triangulate(idx3, 0u, gl_InstanceCustomIndexEXT,vec3(1.f-baryCoord.x-baryCoord.y,baryCoord)).xyz,1.f), matras), matra4);
    PrimaryRay.fdata.xyz = vec3(baryCoord, gl_HitTEXT);
    PrimaryRay.udata     = uvec4(idx3, gl_InstanceCustomIndexEXT);

    // 
    vec4 gTexcoord = vec4(triangulate(idx3, 1u, gl_InstanceCustomIndexEXT,vec3(1.f-baryCoord.x-baryCoord.y,baryCoord)).xyz,0.f);
    vec4 gNormal = vec4(triangulate(idx3, 2u, gl_InstanceCustomIndexEXT,vec3(1.f-baryCoord.x-baryCoord.y,baryCoord)).xyz,0.f);
    vec4 gTangent = vec4(triangulate(idx3, 3u, gl_InstanceCustomIndexEXT,vec3(1.f-baryCoord.x-baryCoord.y,baryCoord)).xyz,0.f);
    vec4 gBinormal = vec4(triangulate(idx3, 4u, gl_InstanceCustomIndexEXT,vec3(1.f-baryCoord.x-baryCoord.y,baryCoord)).xyz,0.f);

    // 
    PrimaryRay.texcoord.xy = gTexcoord.xy;
    PrimaryRay.normals.xyz = normalize(gNormal.xyz * normalTransform * normInTransform);
    PrimaryRay.binorml.xyz = normalize(gBinormal.xyz * normalTransform * normInTransform);
    PrimaryRay.tangent.xyz = normalize(gTangent.xyz * normalTransform * normInTransform);
};
