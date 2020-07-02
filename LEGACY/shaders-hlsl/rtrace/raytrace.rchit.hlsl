#include "./driver.hlsli"
#include "./global.hlsli"

typedef BuiltInTriangleIntersectionAttributes MyAttributes;

[shader("closesthit")]
void main(inout CHIT hit, in MyAttributes attr) {
    hit.gIndices = uint4(InstanceIndex(), GeometryIndex(), PrimitiveIndex(), 0u);
    hit.gBarycentric = float4(max(float3(1.f-attr.barycentrics.x-attr.barycentrics.y, attr.barycentrics.xy), 0.0001f.xxx), RayTCurrent());
};
