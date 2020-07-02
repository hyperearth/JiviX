#define GEN_QUAD_INDEX
#define TRANSFORM_FEEDBACK
#include "./driver.hlsli"


// store 32 value as by 8 bytes
void lStore(in int loc, in uint val) {
    buffers[drawInfo.data[2]].Store(loc, val);
};

// THIS SHADER FOR Minecraft Compatibility
[numthreads(256, 1, 1)]
void main(in uint GlobalInvocationID : SV_DISPATCHTHREADID){
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
