#version 460 core // #
#extension GL_GOOGLE_include_directive  : require
#include "./driver.glsl"

// Left Oriented
layout (location = 0) in vec3 iPosition;
layout (location = 1) in vec2 iTexcoord;
layout (location = 2) in vec3 iNormals;
layout (location = 3) in vec4 iTangent;
//layout (location = 4) in vec4 fBinormal;

// Right Oriented
layout (location = 0) out vec4 gPosition;
layout (location = 1) out vec4 gTexcoord;
layout (location = 2) out vec4 gNormal;
layout (location = 3) out vec4 gTangent;

// 
void main() { // Cross-Lake
    gTexcoord.xy = iTexcoord;
    gPosition = vec4(iPosition.xyz,1.f);
    gNormal = vec4(iNormals.xyz,0.f);
    gTangent = vec4(iTangent.xyz,0.f);
    gl_Position = vec4(iPosition.xyz, 1.f);
};
