#version 460 core
#extension GL_NV_ray_tracing : require
#extension GL_GOOGLE_include_directive : require
#extension GL_EXT_scalar_block_layout : require
#extension GL_EXT_shader_realtime_clock : require
#include "./index.glsl"

layout ( location = 0 ) rayPayloadInNV RayPayloadData PrimaryRay;
                        hitAttributeNV vec2 HitAttribs;

// Test Material 
void main() {
    PrimaryRay.udata.x = 0u;
    PrimaryRay.fdata.xyz = vec3(0.8f, 0.8f, 0.8f);
};
