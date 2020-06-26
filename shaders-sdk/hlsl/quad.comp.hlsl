#version 460 core // #
#extension GL_GOOGLE_include_directive : require
#define GEN_QUAD_INDEX
#define TRANSFORM_FEEDBACK
#include "./driver.glsl"

layout (local_size_x = 256u) in; 

// store 32 value as by 8 bytes
void lStore(in int loc, in uint32_t val) {
    u8float4 cor = unpack8(val);
    imageStore(buffers[drawInfo.data[2]], loc * 4 + 0, cor.xxxx);
    imageStore(buffers[drawInfo.data[2]], loc * 4 + 1, cor.yyyy);
    imageStore(buffers[drawInfo.data[2]], loc * 4 + 2, cor.zzzz);
    imageStore(buffers[drawInfo.data[2]], loc * 4 + 3, cor.wwww);
};

// THIS SHADER FOR Minecraft Compatibility
void main(){
    const uint4 quadIndices = gl_GlobalInvocationID.x*4u + uint4(0u,1u,2u,3u);

    {
        // CW triangle A
        lStore(int(gl_GlobalInvocationID.x*6+0), quadIndices.x);
        lStore(int(gl_GlobalInvocationID.x*6+1), quadIndices.y);
        lStore(int(gl_GlobalInvocationID.x*6+2), quadIndices.z);

        // CW triangle B
        lStore(int(gl_GlobalInvocationID.x*6+3), quadIndices.y);
        lStore(int(gl_GlobalInvocationID.x*6+4), quadIndices.w);
        lStore(int(gl_GlobalInvocationID.x*6+5), quadIndices.z);
    };
};
