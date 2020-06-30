#include "./driver.hlsli"
#include "./global.hlsli"

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

// 
static const float3 bary[3] = { float3(1.f,0.f,0.f), float3(0.f,1.f,0.f), float3(0.f,0.f,1.f) };

[maxvertexcount(3)]
void main(triangle GS_INPUT input[3], inout TriangleStream<PS_INPUT> OutputStream) { // Just Remap Into... 
    PS_INPUT output;
    [unroll] for (uint i=0u;i<3u;i++) {
        output.Position = mul(getMT4x4(pushed.projection), float4(mul(getMT3x4(pushed.modelview), input[i].iPosition), 1.f));
        //output.PointSize = 1;
        output.fTexcoord = input[i].iTexcoord;
        output.fPosition = input[i].iPosition; // CORRECT
        output.fBarycent = input[i].iBarycent;
        output.uData = input[i].inData;
        OutputStream.Append(output);
    };
};
