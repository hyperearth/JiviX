#include "./driver.hlsli"
#include "./global.hlsli"

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
    float PointSize  : PSIZE0;
    float4 FragCoord : SV_Position;
};

// 
struct PS_OUTPUT
{
    float4 oDiffused : SV_TARGET0;
    float4 oSmoothed : SV_TARGET1;
    float4 oSpecular : SV_TARGET2;
};

// 
const float2 shift[9] = {
    float2(-1.f,-1.f),float2(0.f,-1.f),float2(1.f,-1.f),
    float2(-1.f, 0.f),float2(0.f, 0.f),float2(1.f, 0.f),
    float2(-1.f, 1.f),float2(0.f, 1.f),float2(1.f, 1.f)
};

// 
bool checkCorrect(in float4 gNormal, in float4 wPosition, in float4 screenSample, in float2 i2fxm) {
    for (int i=0;i<9;i++) {
        const float2 offt = shift[i];

        float4 worldspos = float4(frameBuffers[BW_POSITION].SampleLevel(samplers[0u], float2(i2fxm+offt), 0).xyz,1.f);
        float4 almostpos = float4(world2screen(worldspos.xyz),1.f);
        //almostpos.y *= -1.f;

        if (
            //abs(screenSample.z-almostpos.z) < 0.0001f && 
            (screenSample.z-almostpos.z) < 0.0001f && // Reserved for FOG 
            length(almostpos.xy-screenSample.xy) < 4.f && 
            dot(gNormal.xyz, frameBuffers[BW_GEONORML].SampleLevel(samplers[0u], float2(i2fxm+offt), 0).xyz) >=0.5f && 
                             frameBuffers[BW_MATERIAL][i2fxm+offt].z > 0.f &&
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
PS_OUTPUT main(in PS_INPUT input) { // Currently NO possible to compare
    const int2 f2fx  = int2(input.FragCoord.xy);
          int2 size  = int2(0, 0); frameBuffers[BW_POSITION].GetDimensions(size.x,size.y);
    const int2 i2fx  = int2(f2fx.x,size.y-f2fx.y-1);
    const float2 i2fxm = input.FragCoord.xy; //float2(gl_FragCoord.x,float(size.y)-gl_FragCoord.y);

    // 
    PS_OUTPUT output;
    if (checkCorrect(input.gNormal, input.wPosition, float4(input.gSample.xyz,1.f), i2fxm)) {
        output.oDiffused = input.gColor;
        output.oSpecular = float4(input.gSpecular.xyz, input.gSpecular.w*0.5f); // TODO: Make New Reflection Sampling
        output.oSmoothed = input.gSmooth;
    } else { discard; };
    return output;
};
