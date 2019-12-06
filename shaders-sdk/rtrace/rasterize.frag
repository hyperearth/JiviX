#version 460 core
#extension GL_GOOGLE_include_directive : require
#extension GL_EXT_scalar_block_layout : require
#include "./index.glsl"

// 
layout (location = 0) in vec4 gPosition;
layout (location = 1) in vec4 gNormals;
layout (location = 2) in vec4 gTexcoords;
layout (location = 3) in vec4 gTangents;
layout (location = 4) flat in ivec4 gIndexes;

// 
layout (location = 0) out vec4 colors;
layout (location = 1) out vec4 normals;
layout (location = 2) out vec4 txnormals;
layout (location = 3) out vec4 parameters;
layout (location = 4) out vec4 samples;

// 
void main() {
    colors = vec4(texture(sampler2D(textures[0],samplers[0]),gTexcoords.xy).xyz,1.f);
    normals = gNormals;
    txnormals = gNormals;
    samples = gPosition;
    parameters = intBitsToFloat(gIndexes);
};
