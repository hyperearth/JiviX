#include "./driver.hlsli"
#include "./global.hlsli"

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
     float FragDepth    : SV_Depth;
};


PS_OUTPUT main(in PS_INPUT input, in uint gl_PrimitiveID : SV_PrimitiveID, in float4 FragCoord : SV_Position) { // TODO: Re-Interpolate for Randomized Center
    const float2 fragCoord = FragCoord.xy; // + SampleCenter;
    const uint primitiveID = uint(gl_PrimitiveID.x);
    const uint geometryInstanceID = input.uData.x;//uint(gl_InstanceIndex.x);
    const uint nodeMeshID = drawInfo.data.x;
    const uint globalInstanceID = drawInfo.data.z;

#define MatID materialID[nodeMeshID][geometryInstanceID]
    const MaterialUnit unit = materials[MatID]; // NEW! 20.04.2020
    const float4 diffuseColor = toLinear(unit. diffuseTexture >= 0 ? textures[unit.diffuseTexture].SampleLevel(samplers[3u], input.fTexcoord.xy, 0) : unit.diffuse);

    PS_OUTPUT output;
    output.oPosition  = float4(0.f.xxxx);
    output.oMaterial  = float4(0.f.xxxx);
    output.oGeoIndice = float4(0.f.xxxx);
    output.FragDepth = 1.f;


    return output;
};
