#ifdef GLSL
#version 460 core // #
#extension GL_GOOGLE_include_directive  : require
#endif

#define TRANSFORM_FEEDBACK
#include "./driver.hlsli"
#include "./tf.hlsli"

#ifdef GLSL
// Left Oriented
layout (location = 0) in float3 iPosition;
layout (location = 1) in float2 iTexcoord;
layout (location = 2) in float3 iNormals;
layout (location = 3) in float4 iTangent;
layout (location = 4) in float4 iColor; // Will used by Minecraft
//layout (location = 4) in float4 fBinormal;

// Right Oriented
layout (location = 0) out float4 gPosition;
layout (location = 1) out float4 gTexcoord;
layout (location = 2) out float4 gNormal;
layout (location = 3) out float4 gTangent;

out gl_PerVertex {   // some subset of these members will be used
    float4 gl_Position;
    float gl_PointSize;
};
#else

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

#endif

// Should to able used by OpenGL
#ifdef GLSL
void main() 
#else
GS_INPUT main(in VS_INPUT input, in uint VertexIndex : SV_VERTEXID) 
#endif
{ // Cross-Lake
#ifdef GLSL
    const uint VertexIndex = gl_VertexIndex;
#endif

    const int IdxType = int(drawInfo.data.y)-1;
    const uint idx = VertexIndex; // Default Index of Vertice
    //if (IdxType == IndexU8 ) { idx = load_u8 (idx*1u, 0u, true); };
    //if (IdxType == IndexU16) { idx = load_u16(idx*2u, 0u, true); };
    //if (IdxType == IndexU32) { idx = load_u32(idx*4u, 0u, true); };

    // Use Apple-Like Attributes
    const float4 iPosition = get_float4(idx, 0u);
    const float4 iTexcoord = get_float4(idx, 1u);
    const float4 iNormals  = get_float4(idx, 2u);
    const float4 iTangent  = get_float4(idx, 3u);
    const float4 iBinormal = get_float4(idx, 4u);

    // HuLuSuL traditional (needs correct support for GLTF)
    //const float4 iPosition = float4(input.iPosition,1.f);//get_float4(idx, 0u);
    //const float4 iTexcoord = float4(input.iTexcoord,0.f.xx);//get_float4(idx, 1u);
    //const float4 iNormals  = float4(input.iNormals,0.f);//get_float4(idx, 2u);
    //const float4 iTangent  = input.iTangent;//get_float4(idx, 3u);
    //const float4 iBinormal = input.iBinormal;//get_float4(idx, 4u);

    // 
    GS_INPUT output;
    output.gTexcoord.xy = iTexcoord.xy;
    output.gPosition = float4(iPosition.xyz,1.f);
    output.gNormal = float4(iNormals.xyz,asfloat(packUnorm4x8(input.iColor))); // Do NOT interpolate W for Fragment Shader, because needs `unpackUnorm4x8(floatBitsToUint())`
    output.gTangent = float4(iTangent.xyz,0.f);
    output.Position = float4(iPosition.xyz,1.f);

    // 
#ifdef GLSL
    gPosition = output.gPosition;
    gTexcoord = output.gTexcoord;
    gNormal = output.gNormal;
    gTangent = output.gTangent;
    gl_Position = output.Position;
#else
    return output;
#endif

};
