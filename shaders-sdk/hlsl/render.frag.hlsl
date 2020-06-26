#version 460 core // #
#extension GL_GOOGLE_include_directive          : require
#include "./driver.glsl"

#define FXAA_PC 1
#define FXAA_GLSL_130 1
#define FXAA_QUALITY_PRESET 39
#include "./fxaa3_11.h"

layout ( location = 0 ) out float4 uFragColor;
layout ( location = 0 ) in float2 vcoord;

// 
void main() { // TODO: explicit sampling 
    const int2 size = textureSize(renderBuffers[BW_RENDERED], 0), samplep = int2(gl_FragCoord.x,gl_FragCoord.y);
    
    // Final Result Rendering
    float4 zero = 0.f.xxxx;
    //uFragColor = FxaaPixelShader(vcoord, zero, renderBuffers[BW_RENDERED], renderBuffers[BW_RENDERED], renderBuffers[BW_RENDERED], size, zero, zero, zero, 0.75, 0.166, 0.0833, 8.0, 0.125, 0.05, zero);// = imageLoad(writeImages[BW_RENDERED], samplep);
    uFragColor = texture(renderBuffers[BW_RENDERED], vcoord);
};
