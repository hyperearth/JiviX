#ifdef GLSL
#version 460 core // #
#extension GL_GOOGLE_include_directive          : require
#endif

#include "./driver.hlsli"

// 
STATIC const float2 cpositions[4] = { float2(-1.f, 1.f), float2(1.f, 1.f), float2(-1.f, -1.f), float2(1.f, -1.f) };
STATIC const float2 tcoords[4] = { float2(0.f, 0.f), float2(1.f, 0.f), float2(0.f, 1.f), float2(1.f, 1.f) };
//const float2 tcoords[4] = { float2(0.f, 1.f), float2(1.f, 1.f), float2(0.f, 0.f), float2(1.f, 0.f) };

#ifdef GLSL
layout ( location = 0 ) out float2 vcoord;
struct PSInput
{
    float4 position;
    float2 vcoord;
};
#else
struct PSInput
{
    float4 position : SV_POSITION;
    float2 vcoord : COLOR;
};
#endif

// 
#ifdef GLSL
void main() 
#else
PSInput main(in uint InstanceIndex : SV_InstanceID, in uint VertexIndex : SV_VertexID)
#endif
{
    PSInput output;
    output.position = float4(cpositions[VertexIndex].xy, 0.0f, 1.0f);
    output.vcoord = tcoords[VertexIndex].xy;
#ifdef GLSL
    gl_Position = output.position;
    vcoord = output.vcoord;
#else
    return output;
#endif
};
