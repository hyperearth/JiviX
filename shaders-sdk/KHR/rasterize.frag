#version 460 core // #
#extension GL_GOOGLE_include_directive : require
#extension GL_EXT_ray_tracing          : require
#extension GL_EXT_ray_query            : require
#extension GL_ARB_post_depth_coverage  : require
#include "./driver.glsl"

layout ( binding = 2, set = 1 ) uniform accelerationStructureEXT Scene;
layout ( early_fragment_tests ) in; // Reduce Lag Rate! (but transparency may broken!)
// Прозрачность с новой прошивкой починим! @RED21


// 
layout (location = 0) in vec4 fPosition;
layout (location = 1) in vec4 fTexcoord;
layout (location = 2) in vec4 fNormal;
layout (location = 3) in vec4 fTangent;
layout (location = 4) in vec4 fBinormal;
layout (location = 5) flat in uvec4 uData;

// 
void main() {
    
};
