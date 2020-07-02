#include "./driver.hlsli"
#include "./global.hlsli"

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

// 
[maxvertexcount(1)]
void main(point in GS_INPUT input[1], inout TriangleStream<PS_INPUT> OutputStream) {
    PS_INPUT output;
    output.Position = input[0].Position;
    output.PointSize = 1.f;
    output.gColor = input[0].vColor;
    output.gSample = input[0].vSample;
    output.gNormal = input[0].vNormal;
    output.wPosition = input[0].vPosition;
    output.gSpecular = input[0].vSpecular;
    output.gRescolor = input[0].vRescolor;
    output.gSmooth = input[0].vSmooth;
    OutputStream.Append(output);
    OutputStream.RestartStrip();
};
