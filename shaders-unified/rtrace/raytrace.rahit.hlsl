#ifdef GLSL
#extension GL_EXT_ray_tracing          : require
#endif

#include "./driver.hlsli"
#include "./global.hlsli"

#ifdef GLSL
layout (location = 0) rayPayloadInEXT CHIT hit;
hitAttributeEXT float2 baryCoord;
#define InstanceIndex() gl_InstanceID
#define GeometryIndex() gl_GeometryIndexEXT
#define PrimitiveIndex() gl_PrimitiveID
#define RayTCurrent() gl_HitTEXT
#else 
typedef BuiltInTriangleIntersectionAttributes MyAttributes;
#define baryCoord attr.barycentrics
#endif

// TODO: Add HLSL Support!!!
#ifdef GLSL
void main() 
#else
[shader("anyhit")]
void main(inout CHIT hit, in MyAttributes attr) 
#endif
{
    //hit.gIndices = uint4(InstanceIndex(), GeometryIndex(), PrimitiveIndex(), 0u);
    //hit.gBarycentric = float4(max(float3(1.f-baryCoord.x-baryCoord.y, baryCoord.xy), 0.0001f.xxx), RayTCurrent());

#ifndef OPAQUE
    XHIT xhit;
    xhit.gIndices = uint4(InstanceIndex(), GeometryIndex(), PrimitiveIndex(), 0u);
    xhit.gBarycentric = float4(max(float3(1.f-baryCoord.x-baryCoord.y, baryCoord.xy), 0.0001f.xxx), RayTCurrent());
#ifdef GLSL
    xhit.origin = float4(gl_WorldRayOriginEXT.xyz, 1.f);
    xhit.direct = float4(gl_WorldRayDirectionEXT.xyz, 0.f);
#endif

    uvec2 seed = uvec2(pushed.rdata.x, packUnorm4x8(fract(vec4(xhit.origin.xyz*0.1f, 1.f))));

    // Interpolate In Ray-Tracing
    XGEO geometry = interpolate(xhit);
    XPOL material = materialize(xhit, geometry);

    // confirm that hit 
    if (material.diffuseColor.w <= random(seed)) { // Only When Opaque!
        ignoreIntersectionEXT();
    }; //else {
        //hit.gIndices = xhit.gIndices;
        //hit.gBarycentric = xhit.gBarycentric;
    //};
#endif
};
