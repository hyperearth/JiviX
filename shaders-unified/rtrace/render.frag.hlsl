#ifdef GLSL
#version 460 core // #
#extension GL_GOOGLE_include_directive          : require
#endif

#include "./driver.hlsli"

//#define FXAA_PC 1
//#define FXAA_GLSL_130 1
//#define FXAA_QUALITY_PRESET 39
//#include "./fxaa3_11.h"

#ifdef GLSL
layout ( location = 0 ) out float4 uFragColor;
layout ( location = 0 ) in float2 vcoord;
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
PSOutput main(in PSInput input)
#endif
{ // TODO: explicit sampling 
#ifdef GLSL
    PSInput input;
    input.position = gl_FragCoord;
    input.vcoord = vcoord;
#endif
    const int2 size = int2(textureSize(frameBuffers[BW_RENDERED], 0)), samplep = int2(input.position.x, input.position.y);

    // Final Result Rendering
    PSOutput output;
    output.uFragColor = 0.f.xxxx;
    //output.uFragColor = FxaaPixelShader(vcoord, zero, renderBuffers[BW_RENDERED], renderBuffers[BW_RENDERED], renderBuffers[BW_RENDERED], size, output.uFragColor, output.uFragColor, output.uFragColor, 0.75, 0.166, 0.0833, 8.0, 0.125, 0.05, output.uFragColor);// = imageLoad(writeImages[BW_RENDERED], samplep);
    output.uFragColor = textureSample(frameBuffers[BW_RENDERED], samplers[1u], input.vcoord);

#ifdef GLSL
    uFragColor = output.uFragColor;
#else
    return output;
#endif
};
