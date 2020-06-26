#include "./driver.glsl"
#include "./global.glsl"

layout (location = 0) rayPayloadInEXT CHIT hit;

void main() {
    hit.gIndices = uint4(0u.xxx, 0u);
    hit.gBarycentric = float4(float3(0.f.xxx), 10000.f);
};
