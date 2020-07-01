#version 460 core // #
#extension GL_GOOGLE_include_directive : require
#extension GL_EXT_ray_tracing          : require
#include "./driver.hlsli"
#include "./global.hlsli"

layout (location = 0) rayPayloadInEXT CHIT hit;

void main() {
    hit.gIndices = uint4(0u.xxx, 0u);
    hit.gBarycentric = float4(float3(0.f.xxx), 10000.f);
};
