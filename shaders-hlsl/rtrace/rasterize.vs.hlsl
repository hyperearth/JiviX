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

    // Use Apple-Like Attributes
    //const float4 iPosition = get_float4(idx, 0u, nodeMeshID);
    //const float4 iTexcoord = get_float4(idx, 1u, nodeMeshID);
    //const float4 iNormals  = get_float4(idx, 2u, nodeMeshID);
    //const float4 iTangent  = get_float4(idx, 3u, nodeMeshID);
    //const float4 iBinormal = get_float4(idx, 4u, nodeMeshID);

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
    PS_INPUT output;
    output.fTexcoord = float4(input.iTexcoord.xy, 0.f.xx);
    output.fPosition = mul4(mul4(float4(input.iPosition.xyz, 1.f), matras), matra4); // CORRECT
    output.fBarycent = float4(0.f.xxx, 0.f);
    output.uData = uint4(InstanceIndex, 0u.xxx);
    output.Position = mul(pushed.projection, float4(mul(pushed.modelview, output.fPosition), 1.f));
    output.Position.y *= -1.f;
    return output;
};
