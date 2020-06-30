#include "./driver.hlsli"
#include "./global.hlsli"

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

// 
GS_INPUT main(in VS_INPUT input, in uint InstanceIndex : SV_InstanceID, in uint VertexIndex : SV_VertexID) {
    // Full Instance ID of Node (BY GEOMETRY INSTATNCE!!)
    //const uint primitiveID = uint(gl_PrimitiveIndex.x);
    const uint geometryInstanceID = uint(InstanceIndex.x);
    const uint nodeMeshID = drawInfo.data.x;
    const uint globalInstanceID = drawInfo.data.z;
    const uint idx = uint(VertexIndex.x);

    // By Geometry Data
    float3x4 matras = transforms[nodeMeshID][geometryInstanceID];
    if (!hasTransform(meshInfo[nodeMeshID])) {
        matras = float3x4(float4(1.f,0.f.xxx),float4(0.f,1.f,0.f.xx),float4(0.f.xx,1.f,0.f));
    };

    // By Instance Data
    const float3x4 matra4 = rtxInstances[globalInstanceID].transform;

    // Native Normal Transform
    const float3x3 normalTransform = inverse(transpose(regen3(matras))); // Geometry ID (Mesh)
    const float3x3 normInTransform = inverse(transpose(regen3(matra4))); // Instance ID (Node)

    // Just Remap Into... 
    GS_INPUT output;
    output.fTexcoord = float4(input.iTexcoord.xy, 0.f.xx);
    output.fPosition = float4(mul(matra4, float4(mul(matras, float4(input.iPosition.xyz, 1.f)), 1.f)), 1.f); // CORRECT
    output.fBarycent = float4(0.f.xxxx);
    output.uData = uint4(InstanceIndex, 0u.xxx);
    output.Position = mul(float4(mul(pushed.modelview, output.fPosition), 1.f), pushed.projection);
    return output;
};
