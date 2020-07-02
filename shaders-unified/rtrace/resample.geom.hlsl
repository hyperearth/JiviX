#ifdef GLSL
#version 460 core // #
#extension GL_GOOGLE_include_directive  : require
#endif

// 
#include "./driver.hlsli"

// 
#ifdef GLSL
layout (location = 0) in float4 vColor[];
layout (location = 1) in float4 vSample[];
layout (location = 2) in float4 vNormal[];
layout (location = 3) in float4 vPosition[];
layout (location = 4) in float4 vSpecular[];
layout (location = 5) in float4 vRescolor[];
layout (location = 6) in float4 vSmooth[];

// 
layout (location = 0) out float4 gColor;
layout (location = 1) out float4 gSample;
layout (location = 2) out float4 gNormal;
layout (location = 3) out float4 wPosition;
layout (location = 4) out float4 gSpecular;
layout (location = 5) out float4 gRescolor;
layout (location = 6) out float4 gSmooth;

// some subset of these members will be used
out gl_PerVertex {
    float4 gl_Position;
    float gl_PointSize;
};

// 
struct GS_INPUT
{
    float PointSize;
    float4 Position;
    float4 vColor;
    float4 vSample;
    float4 vNormal;
    float4 vPosition;
    float4 vSpecular;
    float4 vRescolor;
    float4 vSmooth;
};

// 
struct PS_INPUT
{
    float PointSize;
    float4 Position;
    float4 gColor;
    float4 gSample;
    float4 gNormal;
    float4 wPosition;
    float4 gSpecular;
    float4 gRescolor;
    float4 gSmooth;
};

#else

// 
struct GS_INPUT
{
    float PointSize  : PSIZE0;
    float4 Position  : SV_POSITION;
    float4 vColor    : COLOR0;
    float4 vSample   : COLOR1;
    float4 vNormal   : COLOR2;
    float4 vPosition : COLOR3;
    float4 vSpecular : COLOR4;
    float4 vRescolor : COLOR5;
    float4 vSmooth   : COLOR6;
};

// 
struct PS_INPUT
{
    float PointSize  : PSIZE0;
    float4 Position  : SV_POSITION;
    float4 gColor    : COLOR0;
    float4 gSample   : COLOR1;
    float4 gNormal   : COLOR2;
    float4 wPosition : COLOR3;
    float4 gSpecular : COLOR4;
    float4 gRescolor : COLOR5;
    float4 gSmooth   : COLOR6;
};

#endif

// 
#ifdef GLSL
layout (points) in;
layout (points, max_vertices = 1) out;
void main()
#else
[maxvertexcount(1)]
void main(point in GS_INPUT input[1], inout TriangleStream<PS_INPUT> OutputStream)
#endif
{
#ifdef GLSL
    GS_INPUT input[1];
    input.vColor = vColor[0];
    input.vSample = vSample[0];
    input.vNormal = vNormal[0];
    input.vPosition = vPosition[0];
    input.vSpecular = vSpecular[0];
    input.vRescolor = vRescolor[0];
    input.vSmooth = vSmooth[0];
    input.Position = gl_in[0].gl_Position;
    input.PointSize = gl_in[0].gl_PointSize;
#endif

    PS_INPUT output;
    output.Position = input[0].Position;
    output.PointSize = input[0].PointSize;
    output.gColor = input[0].vColor;
    output.gSample = input[0].vSample;
    output.gNormal = input[0].vNormal;
    output.wPosition = input[0].vPosition;
    output.gSpecular = input[0].vSpecular;
    output.gRescolor = input[0].vRescolor;
    output.gSmooth = input[0].vSmooth;

#ifdef GLSL
    gl_Position = output.Position;
    gl_PointSize = output.PointSize;
    gColor = output.gColor;
    gSample = output.gSample;
    gNormal = output.gNormal;
    wPosition = output.wPosition;
    gSpecular = output.gSpecular;
    gRescolor = output.gRescolor;
    gSmooth = output.gSmooth;
    EmitVertex(), EndPrimitive();
#else
    OutputStream.Append(output);
    OutputStream.RestartStrip();
#endif
};
