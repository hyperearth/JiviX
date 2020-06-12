#version 460 core // #
#extension GL_GOOGLE_include_directive : require
#extension GL_EXT_ray_tracing          : require
#extension GL_EXT_ray_query            : require
#extension GL_ARB_post_depth_coverage  : require

#ifdef AMD_SOLUTION
#extension GL_AMD_shader_explicit_vertex_parameter : require
#define BARYCOORD gl_BaryCoordSmoothAMD
#else
#extension GL_NV_fragment_shader_barycentric : require
#define BARYCOORD gl_BaryCoordNV.yz
#endif

#include "./driver.glsl"
#include "./global.glsl"

// 
layout ( binding = 2, set = 1 ) uniform accelerationStructureEXT Scene;
//layout ( early_fragment_tests ) in; // Reduce Lag Rate! (but transparency may broken!)
// Прозрачность с новой прошивкой починим! @RED21

// 
layout (location = 0) in vec4 fPosition;
layout (location = 1) in vec4 fTexcoord;
layout (location = 2) in vec4 fBarycent;
layout (location = 3) flat in uvec4 uData;

// 
layout (location = RS_MATERIAL) out vec4 oMaterial;
layout (location = RS_GEOMETRY) out vec4 oGeoIndice;
layout (location = RS_POSITION) out vec4 oPosition;
//layout (location = RS_DIFFUSED) out vec4 oDiffused;
//layout (location = RS_BARYCENT) out vec4 oBarycent;

// TODO: FIX Conservative Rasterization! (i.e. add layer or virtualization)
// TODO: Triangle Edge Testing!
// TODO: Anti-Aliasing Support!
void main() { // TODO: Re-Interpolate for Randomized Center
    const vec2 fragCoord = gl_FragCoord.xy; // + SampleCenter;
    const uint primitiveID = uint(gl_PrimitiveID.x);
    const uint geometryInstanceID = uData.x;//uint(gl_InstanceIndex.x);
    const uint nodeMeshID = drawInfo.data.x;
    const uint globalInstanceID = drawInfo.data.z;

#define MatID geomMTs[nonuniformEXT(nodeMeshID)].materialID[geometryInstanceID]
    const MaterialUnit unit = materials[0u].data[MatID]; // NEW! 20.04.2020
    const vec4 diffuseColor = toLinear(unit. diffuseTexture >= 0 ? texture(textures[nonuniformEXT(unit. diffuseTexture)],fTexcoord.xy) : unit.diffuse);
    //const vec4 diffuseColor = vec4(1.f,0.f,0.f,1.f);

    //oDiffused = vec4(0.f.xxxx);
    //oBarycent = vec4(0.f.xxxx);
    oPosition  = vec4(0.f.xxxx);
    oMaterial  = vec4(0.f.xxxx);
    oGeoIndice = vec4(0.f.xxxx);
    gl_FragDepth = 1.f;

    XHIT processing;
    if (diffuseColor.w > 0.f) { // Only When Opaque!
        processing.gIndices = uvec4(geometryInstanceID, globalInstanceID, primitiveID, 0u); // already nodeMeshID used by instance
        processing.origin   = vec4(fPosition.xyz, 1.f);

        // 
        oPosition = processing.origin; // Save texcoord for Parallax Mapping with alpha channel
        oMaterial = uintBitsToFloat(uvec4(floatBitsToUint(max(BARYCOORD,0.00001f.xx)), packUnorm2x16(fTexcoord.xy), floatBitsToUint(1.f)));
        oGeoIndice = uintBitsToFloat(uvec4(globalInstanceID, geometryInstanceID, primitiveID, floatBitsToUint(1.f)));
        gl_FragDepth = gl_FragCoord.z;
        
    };
};
