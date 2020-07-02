#define GEN_QUAD_INDEX
#define TRANSFORM_FEEDBACK

#include "./driver.hlsli"
#include "./tf.hlsli"

// store 32 value as by 8 bytes
#ifdef GLSL
void lStore(in int loc, in uint val) {
    const u8vec4 cor = unpack8(val);
    buffers[drawInfo.data[2]].data[loc * 4 + 0] = cor.x;
    buffers[drawInfo.data[2]].data[loc * 4 + 1] = cor.y;
    buffers[drawInfo.data[2]].data[loc * 4 + 2] = cor.z;
    buffers[drawInfo.data[2]].data[loc * 4 + 3] = cor.w;
};
#else
void lStore(in int loc, in uint val) {
    buffers[drawInfo.data[2]].Store(loc, val);
};
#endif

// THIS SHADER FOR Minecraft Compatibility
#ifdef GLSL
layout (local_size_x = 256u) in;
void main()
#else
[numthreads(256, 1, 1)]
void main(in uint GlobalInvocationID : SV_DISPATCHTHREADID)
#endif
{
#ifdef GLSL
    const uint3 GlobalInvocationID = gl_GlobalInvocationID;
#endif
    const uint4 quadIndices = GlobalInvocationID.x*4u + uint4(0u,1u,2u,3u);
    {
        // CW triangle A
        lStore(int(GlobalInvocationID.x*6+0), quadIndices.x);
        lStore(int(GlobalInvocationID.x*6+1), quadIndices.y);
        lStore(int(GlobalInvocationID.x*6+2), quadIndices.z);

        // CW triangle B
        lStore(int(GlobalInvocationID.x*6+3), quadIndices.y);
        lStore(int(GlobalInvocationID.x*6+4), quadIndices.w);
        lStore(int(GlobalInvocationID.x*6+5), quadIndices.z);
    };
};
