#ifdef GLSL
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

// 
#include "./driver.hlsli"
#include "./global.hlsli"

// 
#ifdef GLSL
struct PS_INPUT
{
    float4 FragCoord;
    float4 fPosition;
    float4 fTexcoord;     
    float4 fBarycent;
    float4 uData;
    float PointSize;
};

// 
struct PS_OUTPUT {
     float4 oMaterial;
     float4 oGeoIndice; 
     float4 oPosition;
     float4 oBarycent;
     float FragDepth;
};

// 
layout (location = 0) in float4 fPosition;
layout (location = 1) in float4 fTexcoord;
layout (location = 2) in float4 fBarycent;
layout (location = 3) flat in float4 uData;

// 
layout (location = RS_MATERIAL) out float4 oMaterial;
layout (location = RS_GEOMETRY) out float4 oGeoIndice;
layout (location = RS_POSITION) out float4 oPosition;
layout (location = RS_BARYCENT) out float4 oBarycent;

#else

// 
struct PS_INPUT
{
    float4 FragCoord             : SV_POSITION;
               float4 fPosition  : POSITION0;
               float4 fTexcoord  : TEXCOORD0;     
               float4 fBarycent  : TEXCOORD1;
    nointerpolation float4 uData : COLOR0;
    float PointSize              : PSIZE0;
};

// 
struct PS_OUTPUT {
     float4 oMaterial   : SV_TARGET0;
     float4 oGeoIndice  : SV_TARGET1; 
     float4 oPosition   : SV_TARGET2;
     float4 oBarycent   : SV_TARGET3;
     float FragDepth    : SV_Depth;
};

#endif

// 
//layout ( early_fragment_tests ) in; // Reduce Lag Rate! (but transparency may broken!)
// Прозрачность с новой прошивкой починим! @RED21

//layout (location = RS_DIFFUSED) out float4 oDiffused;
//layout (location = RS_BARYCENT) out float4 oBarycent;

// TODO: FIX Conservative Rasterization! (i.e. add layer or virtualization)
// TODO: Triangle Edge Testing!
// TODO: Anti-Aliasing Support!
#ifdef GLSL 
void main() 
#else
PS_OUTPUT main(in PS_INPUT inp, in uint PrimitiveID : SV_PrimitiveID, float3 BaryWeights : SV_Barycentrics)
#endif
{   // TODO: Re-Interpolate for Randomized Center
#ifdef GLSL
    const uint PrimitiveID = gl_PrimitiveID;
    const float4 FragCoord = gl_FragCoord;
    const float3 BaryWeights = BARYCOORD;

    PS_INPUT inp = { gl_FragCoord, fPosition, fTexcoord, fBarycent, uData, 0.f };
#endif

    const uint geometryInstanceID = floatBitsToUint(inp.uData.x);//uint(InstanceIndex.x);
    const uint nodeMeshID = drawInfo.data.x;
    const uint globalInstanceID = drawInfo.data.z;


#ifdef GLSL
#define MatID geomMTs[nonuniformEXT(nodeMeshID)].materialID[geometryInstanceID]
#else
#define MatID materialID[nodeMeshID][geometryInstanceID]
#endif

    const MaterialUnit unit = materials[MatID]; // NEW! 20.04.2020
    const float4 diffuseColor = toLinear(unit. diffuseTexture >= 0 ? textureSample(textures[nonuniformEXT(unit. diffuseTexture)], samplers[2u], inp.fTexcoord.xy) : unit.diffuse);

    // 
    PS_OUTPUT outp;
    outp.oPosition  = float4(0.f.xxxx);
    outp.oMaterial  = float4(0.f.xxxx);
    outp.oGeoIndice = float4(0.f.xxxx);
    outp.FragDepth  = 1.1f;

    // 
    XHIT processing;
    if (diffuseColor.w > 0.0001f) { // Only When Opaque!
        processing.gIndices = uint4(geometryInstanceID, globalInstanceID, PrimitiveID, 0u); // already nodeMeshID used by instance
        processing.origin   = float4(inp.fPosition.xyz, 1.f);

        // 
        outp.oPosition = processing.origin; // Save texcoord for Parallax Mapping with alpha channel
        outp.oMaterial = uintBitsToFloat(uint4(0u, 0u, 0u, floatBitsToUint(1.f)));
        outp.oGeoIndice = uintBitsToFloat(uint4(globalInstanceID, geometryInstanceID, PrimitiveID, floatBitsToUint(1.f)));
        outp.oBarycent = float4(max(BaryWeights, 0.0001f.xxx), 1.f);
        outp.FragDepth = inp.FragCoord.z;
    };

#ifdef GLSL
    {
        oPosition = outp.oPosition;
        oMaterial = outp.oMaterial;
        oGeoIndice = outp.oGeoIndice;
        oBarycent = outp.oBarycent;
        gl_FragDepth = outp.FragDepth;
    };
#else
    return outp;
#endif

};
