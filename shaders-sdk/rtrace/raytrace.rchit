#version 460 core // #
#extension GL_GOOGLE_include_directive : require
#extension GL_EXT_ray_tracing          : require

#include "./driver.glsl"
#include "./global.glsl"

layout (location = 0) rayPayloadInEXT RCData rcdata;

hitAttributeEXT vec2 baryCoord;

void main() {
    rcdata.fdata.z = gl_HitTEXT.x;
    rcdata.fdata.xy = baryCoord;
    rcdata.fdata.w = 1.f; // i.e. closest
    rcdata.udata.x = gl_InstanceID.x;
    rcdata.udata.y = gl_InstanceCustomIndexEXT.x;
    rcdata.udata.z = gl_GeometryIndexEXT.x;
    rcdata.udata.w = gl_PrimitiveID.x;
};
