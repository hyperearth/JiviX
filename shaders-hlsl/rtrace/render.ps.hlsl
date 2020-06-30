#include "./driver.hlsli"
#include "./global.hlsli"

// 
//#define FXAA_PC 1
//#define FXAA_GLSL_130 1
//#define FXAA_QUALITY_PRESET 39
//#include "./fxaa3_11.h"

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

// 
PSOutput main(in PSInput input) { // TODO: explicit sampling 
    uint2 size = uint2(0,0), samplep = int2(input.position.x,input.position.y);
    frameBuffers[BW_RENDERED].GetDimensions(size.x, size.y);

    // Final Result Rendering
    PSOutput output;
    output.uFragColor = 0.f.xxxx;
    //output.uFragColor = FxaaPixelShader(vcoord, output.uFragColor, frameBuffers[BW_RENDERED], frameBuffers[BW_RENDERED], frameBuffers[BW_RENDERED], size, zero, zero, zero, 0.75, 0.166, 0.0833, 8.0, 0.125, 0.05, zero);
    output.uFragColor = frameBuffers[BW_RENDERED].SampleLevel(samplers[1u], input.vcoord, 0);
    return output;
};
