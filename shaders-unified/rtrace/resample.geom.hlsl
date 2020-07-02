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
    float4 vColor;
    float4 vSample;
    float4 vNormal;
    float4 vPosition;
    float4 vSpecular;
    float4 vRescolor;
    float4 vSmooth;
    float4 Position;
    float PointSize;
};

// 
struct PS_INPUT
{
    float4 gColor;
    float4 gSample;
    float4 gNormal;
    float4 wPosition;
    float4 gSpecular;
    float4 gRescolor;
    float4 gSmooth;
    float4 Position;
    float PointSize;
};

#else

// 
struct GS_INPUT
{
    float4 vColor    : COLOR0;
    float4 vSample   : COLOR1;
    float4 vNormal   : COLOR2;
    float4 vPosition : COLOR3;
    float4 vSpecular : COLOR4;
    float4 vRescolor : COLOR5;
    float4 vSmooth   : COLOR6;
    float4 Position  : SV_POSITION;
    float PointSize  : PSIZE0;
};

// 
struct PS_INPUT
{
    float4 gColor    : COLOR0;
    float4 gSample   : COLOR1;
    float4 gNormal   : COLOR2;
    float4 wPosition : COLOR3;
    float4 gSpecular : COLOR4;
    float4 gRescolor : COLOR5;
    float4 gSmooth   : COLOR6;
    float4 Position  : SV_POSITION;
    float PointSize  : PSIZE0;
};

#endif

// 
#ifdef GLSL
layout (points) in;
layout (points, max_vertices = 1) out;
void main()
#else
[maxvertexcount(1)]
void main(point in GS_INPUT inp[1], inout TriangleStream<PS_INPUT> OutputStream)
#endif
{
#ifdef GLSL
    GS_INPUT inp[1];
    inp[0].Position = gl_in[0].gl_Position;
    inp[0].vColor = vColor[0];
    inp[0].vSample = vSample[0];
    inp[0].vNormal = vNormal[0];
    inp[0].vPosition = vPosition[0];
    inp[0].vSpecular = vSpecular[0];
    inp[0].vRescolor = vRescolor[0];
    inp[0].vSmooth = vSmooth[0];
    inp[0].PointSize = gl_in[0].gl_PointSize;
#endif

    PS_INPUT outp;
    outp.Position = inp[0].Position;
    outp.gColor = inp[0].vColor;
    outp.gSample = inp[0].vSample;
    outp.gNormal = inp[0].vNormal;
    outp.wPosition = inp[0].vPosition;
    outp.gSpecular = inp[0].vSpecular;
    outp.gRescolor = inp[0].vRescolor;
    outp.gSmooth = inp[0].vSmooth;
    outp.PointSize = inp[0].PointSize;

#ifdef GLSL
    gl_Position = outp.Position;
    gColor = outp.gColor;
    gSample = outp.gSample;
    gNormal = outp.gNormal;
    wPosition = outp.wPosition;
    gSpecular = outp.gSpecular;
    gRescolor = outp.gRescolor;
    gSmooth = outp.gSmooth;
    gl_PointSize = outp.PointSize;
    EmitVertex(), EndPrimitive();
#else
    OutputStream.Append(outp);
    OutputStream.RestartStrip();
#endif
};
