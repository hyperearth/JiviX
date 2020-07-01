#version 460 core // #
#extension GL_GOOGLE_include_directive  : require
#include "./driver.hlsli"

// 
layout (triangles) in;
layout (triangle_strip, max_vertices = 3) out;

// Right Oriented
layout (location = 0) in float4 iPosition[];
layout (location = 1) in float4 iTexcoord[];
layout (location = 2) in float4 iBarycent[];
layout (location = 3) flat in uint4 inData[];

// Right Oriented
layout (location = 0) out float4 fPosition;
layout (location = 1) out float4 fTexcoord;
layout (location = 2) out float4 fBarycent;
layout (location = 3) flat out uint4 uData;

out gl_PerVertex {   // some subset of these members will be used
    float4 gl_Position;
    float gl_PointSize;
};

// 
const float3 bary[3] = { float3(1.f,0.f,0.f), float3(0.f,1.f,0.f), float3(0.f,0.f,1.f) };
void main() { // Just Remap Into... 
    [[unroll]] for (uint i=0u;i<3u;i++) {
        fTexcoord = iTexcoord[i];
        fPosition = iPosition[i]; // CORRECT
        fBarycent = iBarycent[i];
        uData = inData[i];

        // 
        gl_Position = float4(iPosition[i] * modelview, 1.f) * projection;
        EmitVertex();
    };
    EndPrimitive();
};
