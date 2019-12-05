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
layout (binding = 2) uniform sampler2D frameBuffers[];
layout (location = 3) out vec4 gColor;

// Using Transposed, DirectX styled Matrices
layout (binding = 0, scalar) uniform Matrices {
    mat4 prvproject;
    mat4 projection;
    mat4x3 prevmodel;
    mat4x3 modelview;
};

// 
void main() {
    const uint idx = gl_VertexIndex;
    const ivec2 size = textureSize(frameBuffers[0],0);
    const ivec2 f2fx = ivec2(idx%size.x,idx/size.x);

    // 
    const vec4 positions = texelFetch(frameBuffers[SAMPLES],f2fx,0);
    const vec4 diffcolor = texelFetch(frameBuffers[DIFFUSE],f2fx,0);

    gl_PointSize = 0; gColor = 0.f.xxxx;
    if (diffcolor.w > 0.f) {
        gl_Position = (positions.xyz * modelview) * projection;
        gl_PointSize = 1;
        gColor = diffcolor;
    };
};
