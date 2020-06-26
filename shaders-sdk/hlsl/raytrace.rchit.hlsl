#include "./driver.glsl"
#include "./global.glsl"

layout (location = 0) rayPayloadInEXT CHIT hit;

hitAttributeEXT float2 baryCoord;

void main() {
    hit.gIndices = uint4(gl_InstanceID, gl_GeometryIndexEXT, gl_PrimitiveID, 0u);
    hit.gBarycentric = float4(max(float3(1.f-baryCoord.x-baryCoord.y, baryCoord.xy), 0.0001f.xxx), gl_HitTEXT );
};
