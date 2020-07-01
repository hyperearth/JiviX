#version 460 core // #
#extension GL_GOOGLE_include_directive  : require
#define TRANSFORM_FEEDBACK
#include "./driver.hlsli"

// Left Oriented
layout (location = 0) in float3 iPosition;
layout (location = 1) in float2 iTexcoord;
layout (location = 2) in float3 iNormals;
layout (location = 3) in float4 iTangent;
layout (location = 4) in float4 iColor; // Will used by Minecraft
//layout (location = 4) in float4 fBinormal;

// Right Oriented
layout (location = 0) out float4 gPosition;
layout (location = 1) out float4 gTexcoord;
layout (location = 2) out float4 gNormal;
layout (location = 3) out float4 gTangent;

out gl_PerVertex {   // some subset of these members will be used
    float4 gl_Position;
    float gl_PointSize;
};

// Should to able used by OpenGL
void main() { // Cross-Lake
    const int IdxType = int(drawInfo.data[1])-1;
    uint idx = uint(gl_VertexIndex.x); // Default Index of Vertice
    //if (IdxType == IndexU8 ) { idx = load_u8 (idx*1u, 0u, true); };
    //if (IdxType == IndexU16) { idx = load_u16(idx*2u, 0u, true); };
    //if (IdxType == IndexU32) { idx = load_u32(idx*4u, 0u, true); };

    // Use Apple-Like Attributes
    const float4 iPosition = get_float4(idx, 0u);
    const float4 iTexcoord = get_float4(idx, 1u);
    const float4 iNormals  = get_float4(idx, 2u);
    const float4 iTangent  = get_float4(idx, 3u);
    const float4 iBinormal = get_float4(idx, 4u);

    // 
    gTexcoord.xy = iTexcoord.xy;
    gPosition = float4(iPosition.xyz,1.f);
    gNormal = float4(iNormals.xyz,uintBitsToFloat(packUnorm4x8(iColor))); // Do NOT interpolate W for Fragment Shader, because needs `unpackUnorm4x8(floatBitsToUint())`
    gTangent = float4(iTangent.xyz,0.f);
    gl_Position = float4(iPosition.xyz,1.f);
};
