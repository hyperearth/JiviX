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

#ifdef GLSL
void main() 
#else
[shader("closesthit")]
void main(inout CHIT hit, in MyAttributes attr) 
#endif
{
    hit.gIndices = uint4(InstanceIndex(), GeometryIndex(), PrimitiveIndex(), 0u);
    hit.gBarycentric = float4(max(float3(1.f-baryCoord.x-baryCoord.y, baryCoord.xy), 0.0001f.xxx), RayTCurrent());
};
