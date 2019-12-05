#version 460 core
#extension GL_GOOGLE_include_directive : require
#extension GL_EXT_scalar_block_layout : require

// Swap Buffers
#define DIFFUSE 0
#define REFLECT 1
#define SAMPLES 2 // Used for Position

#define COLORED 3
#define NORMALS 4
#define NORMMOD 5
#define PARAMET 6
#define DEPTHST 7

#define DENOISE 8
#define OUTPUTS 9

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
layout (binding = 2) uniform sampler2D frameBuffers[];
layout (binding = 3, scalar) buffer Attributes { vec4 data[]; } attris[];
layout (binding = 4, rgba32f) uniform image2D writeImages[];
layout (binding = 5) uniform texture2D textures[];
layout (binding = 6) uniform sampler samplers[];

// Using Transposed, DirectX styled Matrices
layout (binding = 0, scalar) uniform Matrices {
    mat4 prvproject;
    mat4 projection;
    mat4x3 prevmodel;
    mat4x3 modelview;
};

// 
void main() {
    colors = vec4(texture(sampler2D(textures[0],samplers[0]),gTexcoords.xy).xyz,1.f);
    normals = gNormals;
    txnormals = gNormals;
    samples = gPosition;
    parameters = intBitsToFloat(gIndexes);
};
