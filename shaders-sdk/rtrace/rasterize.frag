#version 460 core
#extension GL_GOOGLE_include_directive  : require
#include "./driver.glsl"

// 
layout (location = 0) in vec4 gPosition;
layout (location = 1) in vec4 gTexcoords;
layout (location = 2) in vec4 gNormals;
layout (location = 3) in vec4 gTangents;
//layout (location = 4) flat in ivec4 gIndexes;

// 
layout (location = COLORING) out vec4 colored;
layout (location = POSITION) out vec4 samples;
layout (location = NORMALED) out vec4 normals;
layout (location = TANGENTS) out vec4 tangent;

// 
void main() {
    colored = vec4(DIFFUSE_COLOR,1.f);
    normals = vec4(gNormals.xyz,1.f);
    samples = gPosition;
};
