#version 460 core // #
#extension GL_GOOGLE_include_directive  : require
#include "./driver.hlsli"

#ifdef GLSL
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

#else
// 
struct VS_INPUT 
{
    float3 iPosition : LOCATION0;
    float2 iTexcoord : LOCATION1;
    float3 iNormals  : LOCATION2;
    float4 iTangent  : LOCATION3;
    float4 iBinormal : LOCATION4;
};

// 
struct GS_INPUT
{
    float4 Position              : SV_POSITION;
    float4 fPosition             : POSITION0;
    float4 fTexcoord             : TEXCOORD0;
    float4 fBarycent             : TEXCOORD1;
    nointerpolation uint4 uData  : COLOR0;
};
#endif

// 
const float3 bary[3] = { float3(1.f,0.f,0.f), float3(0.f,1.f,0.f), float3(0.f,0.f,1.f) };
void main() {
    // Full Instance ID of Node (BY GEOMETRY INSTATNCE!!)
    //const uint primitiveID = uint(gl_PrimitiveIndex.x);
    const uint geometryInstanceID = uint(gl_InstanceIndex.x);
    const uint nodeMeshID = drawInfo.data.x;
    const uint globalInstanceID = drawInfo.data.z;
    const uint idx = uint(gl_VertexIndex.x);

    // By Geometry Data
    float3x4 matras = float3x4(float4(1.f,0.f.xxx),float4(0.f,1.f,0.f.xx),float4(0.f.xx,1.f,0.f));
    float3x4 matra4 = rtxInstances[globalInstanceID].transform;
    if (hasTransform(meshInfo[nodeMeshID])) {
        matras = float3x4(instances[nodeMeshID].transform[geometryInstanceID]);
    };

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
