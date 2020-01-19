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
    //colored = vec4(gNormals.xyz * 0.5f + 0.5f,1.f);//vec4(texture(textures[0],gTexcoords.xy).xyz,1.f);
    colored = vec4(vec3(0.9f,0.8f,0.8f)*0.75f * (gNormals.xyz*0.5f+0.5f),1.f);
    //colored = 1.f.xxxx;
    normals = vec4(gNormals.xyz,1.f);
    samples = gPosition;
};
