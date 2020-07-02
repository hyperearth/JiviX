#version 460 core // #
#extension GL_GOOGLE_include_directive  : require
#include "./driver.hlsli"

// 
const float2 cpositions[4] = { float2(-1.f, 1.f), float2(1.f, 1.f), float2(-1.f, -1.f), float2(1.f, -1.f) };
const float2 tcoords[4] = { float2(0.f, 0.f), float2(1.f, 0.f), float2(0.f, 1.f), float2(1.f, 1.f) };
//const float2 tcoords[4] = { float2(0.f, 1.f), float2(1.f, 1.f), float2(0.f, 0.f), float2(1.f, 0.f) };
layout ( location = 0 ) out float2 vcoord;

// 
void main() {
    gl_Position = float4(cpositions[gl_VertexIndex].xy, 0.0f, 1.0f);
    vcoord = tcoords[gl_VertexIndex].xy;
};
