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
//layout (location = 0) in vec4 iPosition;
layout (location = 0) in vec3 iPosition;
layout (location = 1) in vec4 iNormals;
layout (location = 2) in vec4 iTexcoords;
layout (location = 3) in vec4 iTangents;

// 
layout (location = 0) out vec4 gPosition;
layout (location = 1) out vec4 gNormals;
layout (location = 2) out vec4 gTexcoords;
layout (location = 3) out vec4 gTangents;
layout (location = 4) flat out ivec4 gIndexes;

// Using Transposed, DirectX styled Matrices
layout (binding = 0, scalar) uniform Matrices {
    mat4 prvproject;
    mat4 projection;
    mat4x3 prevmodel;
    mat4x3 modelview;
};

// 
void main() {
    gTexcoords = iTexcoords;
    gPosition = vec4(iPosition,1.f);
    gTangents = iTangents;
    gNormals = iNormals;

    // Final Output 
    gl_Position = (iPosition.xyz * modelview) * projection;
};
