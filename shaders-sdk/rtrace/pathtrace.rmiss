#version 460 core
#extension GL_NV_ray_tracing : require
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


struct RayPayload {
    vec2 UV;
    float depth;
    uint index;
    uvec4 mdata;
};

layout ( location = 0 ) rayPayloadInNV RayPayload PrimaryRay;


void main() {
    PrimaryRay.UV = vec2(0,0);
    PrimaryRay.depth = 100.f;
    PrimaryRay.index = 0xFFFFFFFF;
}
