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
layout (location = 3) flat out float4 uData;

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
STATIC const float3 bary[3] = { float3(1.f,0.f,0.f), float3(0.f,1.f,0.f), float3(0.f,0.f,1.f) };
#ifdef GLSL
void main() 
#else
PS_INPUT main(in VS_INPUT inp, in uint InstanceIndex : SV_InstanceID, in uint VertexIndex : SV_VertexID)
#endif
{
#ifdef GLSL
    const uint InstanceIndex = gl_InstanceIndex;
    const uint VertexIndex = gl_VertexIndex;

    VS_INPUT inp = {iPosition, iTexcoord, iNormals, iTangent, iBinormal};
#endif

    // Full Instance ID of Node (BY GEOMETRY INSTATNCE!!)
    //const uint primitiveID = uint(gl_PrimitiveIndex.x);
    const uint geometryInstanceID = uint(InstanceIndex.x);
    const uint nodeMeshID = drawInfo.data.x;
    const uint globalInstanceID = drawInfo.data.z;

    GeometryNode node;
#ifdef GLSL
    node = geometryNodes[nonuniformEXT(nodeMeshID)].data[geometryInstanceID];
#else
    node = geometryNodes[nonuniformEXT(nodeMeshID)][geometryInstanceID];
#endif

    // By Geometry Data
    float3x4 matras = float3x4(float4(1.f,0.f.xxx),float4(0.f,1.f,0.f.xx),float4(0.f.xx,1.f,0.f));
    float3x4 matra4 = float3x4(rtxInstances[globalInstanceID].transform);
    if (hasTransform(meshInfo[nodeMeshID])) {
        matras = node.transform;
    };

    // Native Normal Transform
    const float3x3 normalTransform = inverse(transpose(regen3(matras))); // Geometry ID (Mesh)
    const float3x3 normInTransform = inverse(transpose(regen3(matra4))); // Instance ID (Node)

    // Just Remap Into... 
    PS_INPUT outp;
    outp.fTexcoord = float4(inp.iTexcoord.xy, 0.f.xx);
    outp.fPosition = mul4(mul4(float4(inp.iPosition.xyz, 1.f), matras), matra4); // CORRECT
    outp.fBarycent = float4(0.f.xxx, 0.f);
    outp.uData = uintBitsToFloat(uint4(InstanceIndex, 0u.xxx));
    outp.Position = mul(getMT4x4(pushed.projection), float4(mul(getMT3x4(pushed.modelview), outp.fPosition), 1.f));
    outp.Position.y *= -1.f;
    
#ifdef GLSL
    {
        gl_Position = outp.Position;
        fTexcoord = outp.fTexcoord;
        fPosition = outp.fPosition;
        fBarycent = outp.fBarycent;
        uData = outp.uData;
    };
#else
    return outp;
#endif
};
