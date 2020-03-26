#version 460 core // #
#extension GL_GOOGLE_include_directive : require
#extension GL_EXT_ray_tracing           : require
#include "./driver.glsl"

layout(location = 0) rayPayloadInEXT RayPayloadData PrimaryRay;
hitAttributeEXT vec2 baryCoord;

// TODO: add Any-Hit shaders for remove transparent intersections
void main() {
    // Full Instance ID of Node (BY GEOMETRY INSTATNCE!!)
    const uint nodeMeshID = gl_InstanceCustomIndexEXT; // Mesh ID from Node Mesh List (because indexing)
    const uint geometryInstanceID = gl_GeometryIndexEXT; // TODO: Using In Ray Tracing (and Query) shaders!
    const uint globalInstanceID = gl_InstanceID;

    // type definition
    const int IdxType = int(meshInfo[nodeMeshID].indexType)-1;
    uvec3 idx3 = uvec3(gl_PrimitiveID*3u+0u,gl_PrimitiveID*3u+1u,gl_PrimitiveID*3u+2u);
    if (IdxType == IndexU8 ) { idx3 = uvec3(load_u8 (idx3.x*1u, 8u, nodeMeshID),load_u32(idx3.y*1u, 8u, nodeMeshID),load_u32(idx3.z*1u, 8u, nodeMeshID)); };
    if (IdxType == IndexU16) { idx3 = uvec3(load_u16(idx3.x*2u, 8u, nodeMeshID),load_u16(idx3.y*2u, 8u, nodeMeshID),load_u16(idx3.z*2u, 8u, nodeMeshID)); };
    if (IdxType == IndexU32) { idx3 = uvec3(load_u32(idx3.x*4u, 8u, nodeMeshID),load_u32(idx3.y*4u, 8u, nodeMeshID),load_u32(idx3.z*4u, 8u, nodeMeshID)); };

    // Interpolate In Ray-Tracing 
    vec4 gTexcoord = vec4(triangulate(idx3, 1u, nodeMeshID,vec3(1.f-baryCoord.x-baryCoord.y,baryCoord)).xyz,0.f);
    vec4 gNormal = vec4(triangulate(idx3, 2u, nodeMeshID,vec3(1.f-baryCoord.x-baryCoord.y,baryCoord)).xyz,0.f);
    vec4 gTangent = vec4(triangulate(idx3, 3u, nodeMeshID,vec3(1.f-baryCoord.x-baryCoord.y,baryCoord)).xyz,0.f);
    vec4 gBinormal = vec4(triangulate(idx3, 4u, nodeMeshID,vec3(1.f-baryCoord.x-baryCoord.y,baryCoord)).xyz,0.f);

    // By Geometry Data
    mat3x4 matras = mat3x4(instances[nodeMeshID].transform[geometryInstanceID]);
    if (!hasTransform(meshInfo[nodeMeshID])) {
        matras = mat3x4(vec4(1.f,0.f.xxx),vec4(0.f,1.f,0.f.xx),vec4(0.f.xx,1.f,0.f));
    };

    // By Instance Data
    const mat3x4 matra4 = rtxInstances[globalInstanceID].transform;

    // Native Normal Transform
    const mat3x3 normalTransform = inverse(transpose(regen3(matras)));
    const mat3x3 normInTransform = inverse(transpose(regen3(matra4)));

    // mesh definition 
    PrimaryRay.position  = mul4(mul4(vec4(triangulate(idx3, 0u, nodeMeshID,vec3(1.f-baryCoord.x-baryCoord.y,baryCoord)).xyz,1.f), matras), matra4);
    PrimaryRay.fdata.xyz = vec3(baryCoord, gl_HitTEXT);
    PrimaryRay.udata     = uvec4(idx3, nodeMeshID);

    // 
    PrimaryRay.texcoord.xy = gTexcoord.xy;
    PrimaryRay.normals.xyz = normalize(gNormal.xyz * normalTransform * normInTransform);
    PrimaryRay.binorml.xyz = normalize(gBinormal.xyz * normalTransform * normInTransform);
    PrimaryRay.tangent.xyz = normalize(gTangent.xyz * normalTransform * normInTransform);
};
