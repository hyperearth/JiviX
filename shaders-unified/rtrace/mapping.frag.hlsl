#ifdef GLSL
#version 460 core // #
#extension GL_GOOGLE_include_directive : require
#extension GL_EXT_ray_tracing          : require
#extension GL_EXT_ray_query            : require
#extension GL_ARB_post_depth_coverage  : require

#ifdef AMD_SOLUTION
#extension GL_AMD_shader_explicit_vertex_parameter : require
#define BARYCOORD float3(1.f-gl_BaryCoordSmoothAMD.x-gl_BaryCoordSmoothAMD.y,gl_BaryCoordSmoothAMD)
#else
#extension GL_NV_fragment_shader_barycentric : require
#define BARYCOORD gl_BaryCoordNV
#endif
#endif

#include "./driver.hlsli"
#include "./global.hlsli"

// 
//layout ( early_fragment_tests ) in; // Reduce Lag Rate! (but transparency may broken!)
// Прозрачность с новой прошивкой починим! @RED21



// 
#ifdef GLSL
layout (location = 0) in float4 fPosition;
layout (location = 1) in float4 fTexcoord;
layout (location = 2) in float4 fBarycent;
layout (location = 3) flat in uint4 uData;

// 
layout (location = RS_MATERIAL) out float4 oMaterial;
layout (location = RS_GEOMETRY) out float4 oGeoIndice;
layout (location = RS_POSITION) out float4 oPosition;

// 
struct PS_INPUT
{
    float PointSize;
    float4 Position;
    float4 fPosition;
    float4 fTexcoord;     
    float4 fBarycent;
    float4 uData;
};

// 
struct PS_OUTPUT {
    float4 oMaterial;
    float4 oGeoIndice; 
    float4 oPosition;
    float FragDepth;
};

#else

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

#endif

//layout (location = RS_DIFFUSED) out float4 oDiffused;
//layout (location = RS_BARYCENT) out float4 oBarycent;

// TODO: FIX Conservative Rasterization! (i.e. add layer or virtualization)
// TODO: Triangle Edge Testing!
// TODO: Anti-Aliasing Support!


// Also...
// That Shader Can Be Used for: 
// - Shadow Mapping 1024x
// - Ray-Tracing Shadows 
// - Emission Generation 
// - Voxelization (like MC)

#ifdef GLSL
void main() { // TODO: Re-Interpolate for Randomized Center
#else
PS_OUTPUT main(in PS_INPUT input, in uint gl_PrimitiveID : SV_PrimitiveID, in float4 FragCoord : SV_Position) { // TODO: Re-Interpolate for Randomized Center
#endif
    const float2 fragCoord = gl_FragCoord.xy; // + SampleCenter;
    const uint primitiveID = uint(gl_PrimitiveID.x);
    const uint geometryInstanceID = uData.x;//uint(gl_InstanceIndex.x);
    const uint nodeMeshID = drawInfo.data.x;
    const uint globalInstanceID = drawInfo.data.z;

#ifdef GLSL
    PS_INPUT input = {gl_PointSize.x, gl_Position, fPosition, fTexcoord, fBarycent, uData};
#endif

    const float2 fragCoord = FragCoord.xy; // + SampleCenter;
    const uint primitiveID = uint(gl_PrimitiveID.x);
    const uint geometryInstanceID = input.uData.x;//uint(gl_InstanceIndex.x);
    const uint nodeMeshID = drawInfo.data.x;
    const uint globalInstanceID = drawInfo.data.z;

#ifdef GLSL
#define MatID geomMTs[nonuniformEXT(nodeMeshID)].materialID[geometryInstanceID]
#else
#define MatID materialID[nodeMeshID][geometryInstanceID]
#endif

    const MaterialUnit unit = materials[MatID]; // NEW! 20.04.2020
    const float4 diffuseColor = toLinear(unit. diffuseTexture >= 0 ? textures[unit.diffuseTexture].SampleLevel(samplers[3u], input.fTexcoord.xy, 0) : unit.diffuse);

    PS_OUTPUT output;
    output.oPosition  = float4(0.f.xxxx);
    output.oMaterial  = float4(0.f.xxxx);
    output.oGeoIndice = float4(0.f.xxxx);
    output.FragDepth = 1.f;

#if defined(HLSL) || !defined(GLSL)
    return output;
#else
    oPosition = output.oPosition;
    oMaterial = output.oMaterial;
    oGeoIndice = output.oGeoIndice;
    gl_FragDepth = output.FragDepth;
#endif

};
