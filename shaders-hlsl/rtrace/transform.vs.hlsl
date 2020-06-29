#define TRANSFORM_FEEDBACK
#include "./driver.hlsli"

struct VS_INPUT 
{
    float3 iPosition : LOCATION0;
    float2 iTexcoord : LOCATION1;
    float3 iNormals  : LOCATION2;
    float4 iTangent  : LOCATION3;
    float4 iBinormal : LOCATION4;
    float4 iColor    : LOCATION5;
};

// 
struct GS_INPUT {
    float4 gPosition : POSITION;
    float4 gTexcoord : TEXCOORD;
    float4 gNormal   : NORMAL;
    float4 gTangent  : TANGENT;
    float4 Position  : SV_POSITION;
    float PointSize  : PSIZE;
};

uint packUnorm4x8(in float4 color) {
    uint4 colors = uint4(color*255.f)&0xFFu.xxxx;
    return ((colors.x>>0)|(colors.y<<8)|(colors.z<<16)|(colors.w<<24));
};

// Should to able used by OpenGL
GS_INPUT main(in VS_INPUT input, in uint VertexIndex : SV_VERTEXID) { // Cross-Lake
    const int IdxType = int(drawInfo.data[1])-1;
    uint idx = VertexIndex; // Default Index of Vertice
    //if (IdxType == IndexU8 ) { idx = load_u8 (idx*1u, 0u, true); };
    //if (IdxType == IndexU16) { idx = load_u16(idx*2u, 0u, true); };
    //if (IdxType == IndexU32) { idx = load_u32(idx*4u, 0u, true); };

    // Use Apple-Like Attributes
    const float4 iPosition = get_float4(idx, 0u);
    const float4 iTexcoord = get_float4(idx, 1u);
    const float4 iNormals  = get_float4(idx, 2u);
    const float4 iTangent  = get_float4(idx, 3u);
    const float4 iBinormal = get_float4(idx, 4u);

    // 
    GS_INPUT output;
    output.gTexcoord.xy = iTexcoord.xy;
    output.gPosition = float4(iPosition.xyz,1.f);
    output.gNormal = float4(iNormals.xyz,asfloat(packUnorm4x8(input.iColor))); // Do NOT interpolate W for Fragment Shader, because needs `unpackUnorm4x8(floatBitsToUint())`
    output.gTangent = float4(iTangent.xyz,0.f);
    output.Position = float4(iPosition.xyz,1.f);
    return output;
};
