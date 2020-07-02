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
#ifdef GLSL
    const uint VertexIndex = gl_VertexIndex;
#endif
    PSInput outp;
    outp.position = float4(cpositions[VertexIndex].xy, 0.0f, 1.0f);
    outp.vcoord = tcoords[VertexIndex].xy;
#ifdef GLSL
    gl_Position = outp.position;
    vcoord = outp.vcoord;
#else
    return outp;
#endif
};
