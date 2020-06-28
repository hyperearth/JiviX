#include "./driver.hlsli"
#include "./global.hlsli"

// 
const float2 cpositions[4] = { float2(-1.f, 1.f), float2(1.f, 1.f), float2(-1.f, -1.f), float2(1.f, -1.f) };
const float2 tcoords[4] = { float2(0.f, 0.f), float2(1.f, 0.f), float2(0.f, 1.f), float2(1.f, 1.f) };

// 
struct PSInput
{
    float4 position : SV_POSITION;
    float2 vcoord : COLOR;
};

// 
PSInput main() {
    PSInput output;
    output.position = float4(cpositions[gl_VertexIndex].xy, 0.0f, 1.0f);
    output.vcoord = tcoords[gl_VertexIndex].xy;
    return output;
};
