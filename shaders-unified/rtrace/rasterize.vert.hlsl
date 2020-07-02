#ifdef GLSL
#version 460 core // #
#extension GL_GOOGLE_include_directive  : require
#endif

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

// 
struct VS_INPUT 
{
    float3 iPosition;
    float2 iTexcoord;
    float3 iNormals;
    float4 iTangent;
    float4 iBinormal;
};

// 
struct PS_INPUT
{
    float4 Position;
    float4 fPosition;
    float4 fTexcoord;     
    float4 fBarycent;
    float4 uData;
    float PointSize;
};

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
struct PS_INPUT
{
    float4 Position              : SV_POSITION;
               float4 fPosition  : POSITION0;
               float4 fTexcoord  : TEXCOORD0;     
               float4 fBarycent  : TEXCOORD1;
    nointerpolation float4 uData : COLOR0;
    float PointSize              : PSIZE0;
};

#endif


// 
const float3 bary[3] = { float3(1.f,0.f,0.f), float3(0.f,1.f,0.f), float3(0.f,0.f,1.f) };
#ifdef GLSL
void main() 
#else
PS_INPUT main(in VS_INPUT input, in uint InstanceIndex : SV_InstanceID, in uint VertexIndex : SV_VertexID)
#endif
{
#ifdef GLSL
    const uint InstanceIndex = gl_InstanceIndex;
    const uint VertexIndex = gl_VertexIndex;

    VS_INPUT input = {iPosition, iTexcoord, iNormals, iTangent, iBinormal};
#endif

    // Full Instance ID of Node (BY GEOMETRY INSTATNCE!!)
    //const uint primitiveID = uint(gl_PrimitiveIndex.x);
    const uint geometryInstanceID = uint(InstanceIndex.x);
    const uint nodeMeshID = drawInfo.data.x;
    const uint globalInstanceID = drawInfo.data.z;
    const uint idx = uint(VertexIndex.x);

    // By Geometry Data
    float3x4 matras = float3x4(float4(1.f,0.f.xxx),float4(0.f,1.f,0.f.xx),float4(0.f.xx,1.f,0.f));
    float3x4 matra4 = float3x4(rtxInstances[globalInstanceID].transform);
    if (hasTransform(meshInfo[nodeMeshID])) {
#ifdef GLSL
        matras = float3x4(instances[nodeMeshID].transform[geometryInstanceID]);
#else
        matras = float3x4(tmatrices[nodeMeshID][geometryInstanceID]);
#endif
    };

    // Native Normal Transform
    const float3x3 normalTransform = inverse(transpose(regen3(matras))); // Geometry ID (Mesh)
    const float3x3 normInTransform = inverse(transpose(regen3(matra4))); // Instance ID (Node)

    // Just Remap Into... 
    PS_INPUT output;
    output.fTexcoord = float4(input.iTexcoord.xy, 0.f.xx);
    output.fPosition = mul4(mul4(float4(input.iPosition.xyz, 1.f), matras), matra4); // CORRECT
    output.fBarycent = float4(0.f.xxx, 0.f);
    output.uData = uint4(InstanceIndex, 0u.xxx);
    output.Position = mul(getMT4x4(pushed.projection), float4(mul(getMT3x4(pushed.modelview), output.fPosition), 1.f));
    output.Position.y *= -1.f;
    
#ifdef GLSL
    {
        gl_Position = output.Position;
        fTexcoord = output.fTexcoord;
        fPosition = output.fPosition;
        fBarycent = output.fBarycent;
        uData = output.uData;
    };
#else
    return output;
#endif
};
