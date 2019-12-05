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
const vec2 cpositions[4] = { vec2(-1.f, 1.f), vec2(1.f, 1.f), vec2(-1.f, -1.f), vec2(1.f, -1.f) };
const vec2 tcoords[4] = { vec2(0.f), vec2(1.f, 0.f), vec2(0.f, 1.f), vec2(1.f) };
layout ( location = 0 ) out vec2 vcoord;

// 
void main() {
    gl_Position = vec4(cpositions[gl_VertexIndex].xy, 0.0f, 1.0f);
    vcoord = tcoords[gl_VertexIndex].xy;
}
