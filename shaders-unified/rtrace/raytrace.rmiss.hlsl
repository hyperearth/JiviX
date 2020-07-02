#ifdef GLSL
#extension GL_EXT_ray_tracing : require
#endif

#include "./driver.hlsli"
#include "./global.hlsli"

#ifdef GLSL
layout (location = 0) rayPayloadInEXT CHIT hit;
#endif

#ifdef GLSL
void main() 
#else
[shader("miss")]
void main(inout CHIT hit)
#endif
{
    hit.gIndices = uint4(0u.xxx, 0u);
    hit.gBarycentric = float4(float3(0.f.xxx), 10000.f);
};
