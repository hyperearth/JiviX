#define BARYCOORD BaryWeights

#include "./driver.hlsli"
#include "./global.hlsli"

// 
//layout ( early_fragment_tests ) in; // Reduce Lag Rate! (but transparency may broken!)
// Прозрачность с новой прошивкой починим! @RED21

// 
struct PS_INPUT
{
    float PointSize              : PSIZE0;
    float4 Position              : SV_POSITION;
               float4 fPosition  : POSITION0;
               float4 fTexcoord  : TEXCOORD0;     
               float4 fBarycent  : TEXCOORD1;
    nointerpolation float4 uData : COLOR0;
};

// 
struct PS_OUTPUT {
     float4 oMaterial   : SV_TARGET0;
     float4 oGeoIndice  : SV_TARGET1; 
     float4 oPosition   : SV_TARGET2;
     float4 oBarycent   : SV_TARGET3;
     float FragDepth    : SV_Depth;
};

//layout (location = RS_DIFFUSED) out float4 oDiffused;
//layout (location = RS_BARYCENT) out float4 oBarycent;

// TODO: FIX Conservative Rasterization! (i.e. add layer or virtualization)
// TODO: Triangle Edge Testing!
// TODO: Anti-Aliasing Support!
PS_OUTPUT main(in PS_INPUT input, in uint gl_PrimitiveID : SV_PrimitiveID, in float4 FragCoord : SV_Position, float3 BaryWeights : SV_Barycentrics) { // TODO: Re-Interpolate for Randomized Center
    const float2 fragCoord = FragCoord.xy; // + SampleCenter;
    const uint primitiveID = uint(gl_PrimitiveID.x);
    const uint geometryInstanceID = input.uData.x;//uint(gl_InstanceIndex.x);
    const uint nodeMeshID = drawInfo.data.x;
    const uint globalInstanceID = drawInfo.data.z;

#define MatID materialID[nodeMeshID][geometryInstanceID]
    const MaterialUnit unit = materials[MatID]; // NEW! 20.04.2020
    const float4 diffuseColor = toLinear(unit. diffuseTexture >= 0 ? textures[unit.diffuseTexture].SampleLevel(samplers[2u], input.fTexcoord.xy, 0) : unit.diffuse);
    //const float4 diffuseColor = float4(1.f,0.f,0.f,1.f);

    //oDiffused = float4(0.f.xxxx);
    //oBarycent = float4(0.f.xxxx);
    PS_OUTPUT output;
    output.oPosition  = float4(0.f.xxxx);
    output.oMaterial  = float4(0.f.xxxx);
    output.oGeoIndice = float4(0.f.xxxx);
    output.FragDepth = 1.1f;

    XHIT processing;
    if (diffuseColor.w > 0.0001f) { // Only When Opaque!
        processing.gIndices = uint4(geometryInstanceID, globalInstanceID, primitiveID, 0u); // already nodeMeshID used by instance
        processing.origin   = float4(input.fPosition.xyz, 1.f);

        // 
        output.oPosition = processing.origin; // Save texcoord for Parallax Mapping with alpha channel
        output.oMaterial = asfloat(uint4(0u, 0u, 0u, asuint(1.f)));
        output.oGeoIndice = asfloat(uint4(globalInstanceID, geometryInstanceID, primitiveID, asuint(1.f)));
        output.oBarycent = float4(max(BARYCOORD, 0.0001f.xxx), 1.f);
        output.FragDepth = FragCoord.z;
    };
    return output;
};
