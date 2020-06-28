#include "./driver.glsl"
#include "./global.glsl"

[shader("miss")]
void main(inout CHIT hit) {
    hit.gIndices = uint4(0u.xxx, 0u);
    hit.gBarycentric = float4(float3(0.f.xxx), 10000.f);
};
