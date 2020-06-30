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
struct PS_INPUT
{
    float PointSize              : PSIZE0;
    float4 Position              : SV_POSITION;
               float4 fPosition  : POSITION0;
               float4 fTexcoord  : TEXCOORD0;     
               float4 fBarycent  : TEXCOORD1;
    nointerpolation float4 uData : COLOR0;
};

// 
PS_INPUT main(in VS_INPUT input, in uint InstanceIndex : SV_InstanceID, in uint VertexIndex : SV_VertexID) {
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
//#ifdef SUPPORT_LOCAL_MATRIX
        matras = float3x4(tmatrices[nodeMeshID][geometryInstanceID]);
        //matras = asfloat(uint3x4( // UNSUPPORTED BY DXC SPIR-V
        //    tmatrices[nodeMeshID].Load4((geometryInstanceID*3u+0u)*16u), 
        //    tmatrices[nodeMeshID].Load4((geometryInstanceID*3u+1u)*16u), 
        //    tmatrices[nodeMeshID].Load4((geometryInstanceID*3u+2u)*16u)
        //));
//#endif
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
    return output;
};
