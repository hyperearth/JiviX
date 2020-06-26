#version 460 core // #
#extension GL_GOOGLE_include_directive  : require
#include "./driver.glsl"

// Left Oriented
layout (location = 0) in float3 iPosition;
layout (location = 1) in float2 iTexcoord;
layout (location = 2) in float3 iNormals;
layout (location = 3) in float4 iTangent;
layout (location = 4) in float4 iBinormal;

// Right Oriented
layout (location = 0) out float4 fPosition;
layout (location = 1) out float4 fTexcoord;
layout (location = 2) out float4 fBarycent;
layout (location = 3) flat out uint4 uData;

// 
const float3 bary[3] = { float3(1.f,0.f,0.f), float3(0.f,1.f,0.f), float3(0.f,0.f,1.f) };
void main() {
    // Full Instance ID of Node (BY GEOMETRY INSTATNCE!!)
    //const uint primitiveID = uint(gl_PrimitiveIndex.x);
    const uint geometryInstanceID = uint(gl_InstanceIndex.x);
    const uint nodeMeshID = drawInfo.data.x;
    const uint globalInstanceID = drawInfo.data.z;
    const uint idx = uint(gl_VertexIndex.x);

    // Use Apple-Like Attributes
    //const float4 iPosition = get_float4(idx, 0u, nodeMeshID);
    //const float4 iTexcoord = get_float4(idx, 1u, nodeMeshID);
    //const float4 iNormals  = get_float4(idx, 2u, nodeMeshID);
    //const float4 iTangent  = get_float4(idx, 3u, nodeMeshID);
    //const float4 iBinormal = get_float4(idx, 4u, nodeMeshID);

    // By Geometry Data
    float3x4 matras = float3x4(instances[nodeMeshID].transform[geometryInstanceID]);
    if (!hasTransform(meshInfo[nodeMeshID])) {
        matras = float3x4(float4(1.f,0.f.xxx),float4(0.f,1.f,0.f.xx),float4(0.f.xx,1.f,0.f));
    };

    // By Instance Data
    const float3x4 matra4 = rtxInstances[globalInstanceID].transform;

    // Native Normal Transform
    const float3x3 normalTransform = inverse(transpose(regen3(matras))); // Geometry ID (Mesh)
    const float3x3 normInTransform = inverse(transpose(regen3(matra4))); // Instance ID (Node)

    // Just Remap Into... 
      fTexcoord = float4(iTexcoord.xy, 0.f.xx);
      fPosition = mul4(mul4(float4(iPosition.xyz, 1.f), matras), matra4); // CORRECT
      fBarycent = float4(bary[idx%3u], 0.f);
      uData = uint4(gl_InstanceIndex, 0u.xxx);

    // 
    gl_Position = float4(fPosition * modelview, 1.f) * projection;
};
