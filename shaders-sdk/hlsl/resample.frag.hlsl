#version 460 core // #
#extension GL_GOOGLE_include_directive  : require
#extension GL_EXT_ray_tracing           : require
#extension GL_EXT_ray_query             : require
#include "./driver.glsl"

// 
layout (location = 0) in float4 gColor;
layout (location = 1) in float4 gSample;
layout (location = 2) in float4 gNormal;
layout (location = 3) in float4 wPosition;
layout (location = 4) in float4 gSpecular;
layout (location = 5) in float4 gRescolor;
layout (location = 6) in float4 gSmooth;

// 
layout (location = IW_INDIRECT) out float4 oDiffused;
layout (location = IW_REFLECLR) out float4 oSpecular;
layout (location = IW_SMOOTHED) out float4 oSmoothed;

// 
const float2 shift[9] = {
    float2(-1.f,-1.f),float2(0.f,-1.f),float2(1.f,-1.f),
    float2(-1.f, 0.f),float2(0.f, 0.f),float2(1.f, 0.f),
    float2(-1.f, 1.f),float2(0.f, 1.f),float2(1.f, 1.f)
};

// 
bool checkCorrect(in float4 screenSample, in float2 i2fxm) {
    for (int i=0;i<9;i++) {
        const float2 offt = shift[i];

        float4 worldspos = float4(texture(frameBuffers[BW_POSITION], float2(i2fxm+offt), 0).xyz,1.f);
        float4 almostpos = float4(world2screen(worldspos.xyz),1.f);
        //almostpos.y *= -1.f;

        if (
            //abs(screenSample.z-almostpos.z) < 0.0001f && 
            (screenSample.z-almostpos.z) < 0.0001f && // Reserved for FOG 
            length(almostpos.xy-screenSample.xy) < 4.f && 
            dot(gNormal.xyz,    texture(frameBuffers[BW_GEONORML],  float2(i2fxm+offt), 0).xyz) >=0.5f && 
                             texelFetch(frameBuffers[BW_MATERIAL], int2(i2fxm+offt), 0).z > 0.f &&
            distance(wPosition.xyz,worldspos.xyz) < 0.05f || 
            false//(i == 4 && texelFetch(frameBuffers[BW_INDIRECT], int2(i2fxm+offt), 0).w <= 0.01f) // Prefer use center texel for filling
        ) { return true; };
    };
    return false;
};

// WE NEEDS: 
// - GL_NV_shader_atomic_float
// - GL_NV_shader_atomic_fp16_vector
// - GL_NV_shader_atomic_float64
// FOR COMPUTE SHADERS! 

// 
void main() { // Currently NO possible to compare
    const int2 f2fx  = int2(gl_FragCoord.xy);
    const int2 size  = int2(textureSize(frameBuffers[BW_POSITION], 0));
    const int2 i2fx  = int2(f2fx.x,size.y-f2fx.y-1);
    const  float2 i2fxm = gl_FragCoord.xy; //float2(gl_FragCoord.x,float(size.y)-gl_FragCoord.y);

    // 
    if (checkCorrect(float4(gSample.xyz,1.f), i2fxm)) {
        oDiffused = gColor;
        oSpecular = float4(gSpecular.xyz,gSpecular.w*0.5f); // TODO: Make New Reflection Sampling
        oSmoothed = gSmooth;
    } else { discard; };
};
