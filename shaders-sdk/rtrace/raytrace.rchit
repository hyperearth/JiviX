#version 460 core // #
#extension GL_GOOGLE_include_directive : require
#extension GL_EXT_ray_tracing          : require

#include "./driver.glsl"
#include "./global.glsl"

layout (location = 0) rayPayloadInEXT XHIT hit;

hitAttributeEXT vec2 baryCoord;

void main() {
    hit.direct = vec4(gl_WorldRayDirectionEXT, 0.f);
    hit.origin = vec4(gl_WorldRayOriginEXT, 1.f);
    hit.gIndices = uvec4(gl_InstanceID, gl_GeometryIndexEXT, gl_PrimitiveID, 0u);
    hit.gBarycentric = vec4(max(vec3(1.f-baryCoord.x-baryCoord.y, baryCoord.xy), 0.0001f.xxx), gl_HitTEXT );
};
