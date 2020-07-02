#include "./driver.hlsli"

//#define FXAA_PC 1
//#define FXAA_GLSL_130 1
//#define FXAA_QUALITY_PRESET 39
//#include "./fxaa3_11.h"

#ifdef GLSL
layout ( location = 0 ) out float4 uFragColor;
layout ( location = 0 ) in float2 vcoord;

struct PSInput
{
    float4 position;
    float2 vcoord;
};

//
struct PSOutput 
{
    float4 uFragColor;
};

#else
// 
struct PSInput
{
    float4 position : SV_POSITION;
    float2 vcoord : COLOR;
};

//
struct PSOutput 
{
    float4 uFragColor : SV_TARGET0;
};
#endif

// 
#ifdef GLSL
void main() 
#else
PSOutput main(in PSInput inp)
#endif
{ // TODO: explicit sampling 
#ifdef GLSL
    PSInput inp;
    inp.position = gl_FragCoord;
    inp.vcoord = vcoord;
#endif
    const int2 size = int2(textureSize(frameBuffers[BW_RENDERED], 0)), samplep = int2(inp.position.x, inp.position.y);

    // Final Result Rendering
    PSOutput outp;
    outp.uFragColor = 0.f.xxxx;
    //outp.uFragColor = FxaaPixelShader(vcoord, zero, renderBuffers[BW_RENDERED], renderBuffers[BW_RENDERED], renderBuffers[BW_RENDERED], size, outp.uFragColor, outp.uFragColor, outp.uFragColor, 0.75, 0.166, 0.0833, 8.0, 0.125, 0.05, outp.uFragColor);// = imageLoad(writeImages[BW_RENDERED], samplep);
    outp.uFragColor = textureSample(frameBuffers[BW_RENDERED], samplers[1u], inp.vcoord);

#ifdef GLSL
    uFragColor = outp.uFragColor;
#else
    return outp;
#endif
};
