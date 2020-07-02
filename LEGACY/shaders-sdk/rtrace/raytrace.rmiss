#version 460 core // #
#extension GL_GOOGLE_include_directive : require
#extension GL_EXT_ray_tracing          : require
#include "./driver.glsl"
#include "./global.glsl"

layout (location = 0) rayPayloadInEXT CHIT hit;

void main() {
    hit.gIndices = uvec4(0u.xxx, 0u);
    hit.gBarycentric = vec4(vec3(0.f.xxx), 10000.f);
};
