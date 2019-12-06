#version 460 core
#extension GL_NV_ray_tracing : require
#extension GL_GOOGLE_include_directive : require
#extension GL_EXT_scalar_block_layout : require
#include "./index.glsl"


struct RayPayload {
     vec4 fdata;
    uvec4 udata;
};

layout ( location = 0 ) rayPayloadInNV RayPayload PrimaryRay;
                        hitAttributeNV vec2 HitAttribs;

void main() {
    // PrimaryRay.fdata = vec4();
};
