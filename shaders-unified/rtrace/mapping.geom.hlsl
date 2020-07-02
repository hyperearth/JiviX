#ifdef GLSL
#version 460 core // #
#extension GL_GOOGLE_include_directive  : require
#endif

#include "./driver.hlsli"

// Right Oriented
#ifdef GLSL
layout (location = 0) in float4 iPosition[];
layout (location = 1) in float4 iTexcoord[];
layout (location = 2) in float4 iBarycent[];
layout (location = 3) flat in uint4 inData[];

// Right Oriented
layout (location = 0) out float4 fPosition;
layout (location = 1) out float4 fTexcoord;
layout (location = 2) out float4 fBarycent;
layout (location = 3) flat out uint4 uData;

// 
out gl_PerVertex {   // some subset of these members will be used
    float4 gl_Position;
    float gl_PointSize;
};

// 
struct GS_INPUT
{
    float4 Position;
    float4 iPosition;
    float4 iTexcoord;
    float4 iBarycent;
    uint4 inData;
};

// 
struct PS_INPUT
{
    float PointSize;
    float4 Position;
    float4 fPosition;
    float4 fTexcoord;
    float4 fBarycent;
    uint4 uData;
};

#else 

// 
struct GS_INPUT
{
    float4 Position              : SV_POSITION;
    float4 iPosition             : POSITION0;
    float4 iTexcoord             : TEXCOORD0;
    float4 iBarycent             : TEXCOORD1;
    nointerpolation uint4 inData : COLOR0;
};

// 
struct PS_INPUT
{
    float PointSize              : PSIZE0;
    float4 Position              : SV_POSITION;
               float4 fPosition  : POSITION0;
               float4 fTexcoord  : TEXCOORD0;
               float4 fBarycent  : TEXCOORD1;
    nointerpolation uint4 uData  : COLOR0;
};

#endif

// 
STATIC const float3 bary[3] = { float3(1.f,0.f,0.f), float3(0.f,1.f,0.f), float3(0.f,0.f,1.f) };

// 
#ifdef GLSL
layout (triangles) in;
layout (triangle_strip, max_vertices = 3) out;
void main()  // Just Remap Into... 
#else
[maxvertexcount(3)]
void main(triangle GS_INPUT input[3], inout TriangleStream<PS_INPUT> OutputStream)  // Just Remap Into... 
#endif
{

#ifdef GLSL
    GL_INPUT input[3];
    [[unroll]] for (uint i=0u;i<3u;i++) {
        input[i].iTexcoord = iTexcoord[i];
        input[i].iPosition = iPosition[i];
        input[i].iBarycent = iBarycent[i];
        input[i].inData = inData[i];
        input[i].Position = gl_in[i].gl_Position;
        input[i].PointSize = 0.f;
    };
#endif

    PS_INPUT output;
    [unroll] for (uint i=0u;i<3u;i++) {
        output.Position = mul(getMT4x4(pushed.projection), float4(mul(getMT3x4(pushed.modelview), input[i].iPosition), 1.f));
        //output.PointSize = 1;
        output.fTexcoord = input[i].iTexcoord;
        output.fPosition = input[i].iPosition; // CORRECT
        output.fBarycent = input[i].iBarycent;
        output.uData = input[i].inData;

#ifdef GLSL
        {   // HARD Operation...
            fTexcoord = output.fTexcoord;
            fPosition = output.fPosition; // CORRECT
            fBarycent = output.fBarycent;
            uData = output.uData;
            gl_Position = output.Position;
            EmitVertex();
        };
#else
        OutputStream.Append(output);
#endif
    };


#ifdef GLSL
    EndPrimitive();
#else
    OutputStream.RestartStrip();
#endif
};
