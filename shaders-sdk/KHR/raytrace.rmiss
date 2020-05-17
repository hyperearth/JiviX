#version 460 core // #
#extension GL_GOOGLE_include_directive : require
#extension GL_EXT_ray_tracing          : require
#include "./driver.glsl"

// 
struct RCData {
    uvec4 udata;
    vec4 fdata;
};

layout (location = 0) rayPayloadInEXT RCData rcdata;

void main() {
    rcdata.fdata.z = 10000.f;
    rcdata.fdata.xy = vec2(0.f);
    rcdata.fdata.w = 0.f; // i.e. miss
    rcdata.udata.x = 0u;
    rcdata.udata.y = 0u;
    rcdata.udata.z = 0u;
    rcdata.udata.w = 0u;
};
