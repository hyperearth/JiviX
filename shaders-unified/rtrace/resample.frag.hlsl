#ifdef GLSL
#extension GL_EXT_ray_tracing           : require
#extension GL_EXT_ray_query             : require
#endif

#include "./driver.hlsli"

// 
#ifdef GLSL
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
struct PS_INPUT
{
    float4 gColor;
    float4 gSample;
    float4 gNormal;
    float4 wPosition;
    float4 gSpecular;
    float4 gRescolor;
    float4 gSmooth;
    float4 FragCoord;
    float PointSize;
};

// 
struct PS_OUTPUT
{
    float4 oDiffused;
    float4 oSmoothed;
    float4 oSpecular;
};

#else
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
    float4 FragCoord : SV_Position;
    float PointSize  : PSIZE0;
};

// 
struct PS_OUTPUT
{
    float4 oDiffused : SV_TARGET0;
    float4 oSmoothed : SV_TARGET1;
    float4 oSpecular : SV_TARGET2;
};
#endif

// 
STATIC const float2 shift[1] = { 
    float2(0.f, 0.f)
    //float2(-1.f,-1.f),float2(0.f,-1.f),float2(1.f,-1.f),
    //float2(-1.f, 0.f),float2(0.f, 0.f),float2(1.f, 0.f),
    //float2(-1.f, 1.f),float2(0.f, 1.f),float2(1.f, 1.f)
};

// 
bool checkCorrect(in float4 gNormal, in float4 wPosition, in float4 screenSample, in float2 i2fxm) {
    //for (int i=0;i<9;i++) {
    for (int i=0;i<1;i++) {
        const float2 offt = shift[i];

        // 
        float4 worldspos = float4(textureLodSample(frameBuffers[BW_POSITION], samplers[0u], float2(i2fxm+offt), 0).xyz, 1.f);
        float3 screenpos = world2screen(worldspos.xyz);
        float3 screensmp = world2screen(wPosition.xyz);

        // 
        if (
            abs(screenpos.z - screensmp.z) < 0.0001f && 
            dot(gNormal.xyz, textureLodSample(frameBuffers[BW_MAPNORML], samplers[0u], float2(i2fxm+offt), 0).xyz) > 0.5f && 
            texelFetch(frameBuffers[BW_MATERIAL], int2(i2fxm+offt), 0).z > 0.f &&
            distance(wPosition.xyz,worldspos.xyz) < 0.1f &&
            true
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
#ifdef GLSL
void main() // Currently NO possible to compare
#else
PS_OUTPUT main(in PS_INPUT inp)
#endif
{
#ifdef GLSL
    PS_INPUT inp;
    inp.gColor = gColor;
    inp.gSample = gSample;
    inp.gNormal = gNormal;
    inp.wPosition = wPosition;
    inp.gSpecular = gSpecular;
    inp.gRescolor = gRescolor;
    inp.gSmooth = gSmooth;
    inp.FragCoord = gl_FragCoord;
#endif

    // 
    const int2 f2fx  = int2(inp.FragCoord.xy);
    const int2 size  = int2(textureSize(frameBuffers[BW_POSITION], 0));
    const int2 i2fx  = int2(f2fx.x,f2fx.y);
    const float2 i2fxm = inp.FragCoord.xy; //float2(gl_FragCoord.x,float(size.y)-gl_FragCoord.y);

    // 
    PS_OUTPUT outp;
    if (checkCorrect(inp.gNormal, inp.wPosition, float4(inp.gSample.xyz,1.f), i2fxm)) {
        outp.oDiffused = inp.gColor;
        outp.oSpecular = float4(inp.gSpecular.xyz,1.f); // TODO: Make New Reflection Sampling
        outp.oSmoothed = inp.gSmooth;
    } else { discard; };

    // 
#ifdef GLSL
    oDiffused = outp.oDiffused;
    oSpecular = outp.oSpecular;
    oSmoothed = outp.oSmoothed;
#else
    return outp;
#endif
};
