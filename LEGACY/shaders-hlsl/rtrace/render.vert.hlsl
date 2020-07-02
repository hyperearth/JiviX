#include "./driver.hlsli"
#include "./global.hlsli"

// 
static const float2 cpositions[4] = { float2(-1.f, 1.f), float2(1.f, 1.f), float2(-1.f, -1.f), float2(1.f, -1.f) };
static const float2 tcoords[4] = { float2(0.f, 0.f), float2(1.f, 0.f), float2(0.f, 1.f), float2(1.f, 1.f) };

// 
struct PSInput
{
    float4 position : SV_POSITION;
    float2 vcoord : COLOR;
};

// 
PSInput main(in uint InstanceIndex : SV_InstanceID, in uint VertexIndex : SV_VertexID) {
    PSInput output;
    output.position = float4(cpositions[VertexIndex].xy, 0.0f, 1.0f);
    output.vcoord = tcoords[VertexIndex].xy;
    return output;
};
