#version 460 core // #
#extension GL_GOOGLE_include_directive  : require
#extension GL_EXT_ray_tracing           : require
#extension GL_EXT_ray_query             : require
#include "./driver.glsl"

// 
layout (location = 0) in vec4 gColor;
layout (location = 1) in vec4 gSample;
layout (location = 2) in vec4 gNormal;
layout (location = 3) in vec4 wPosition;
layout (location = 4) in vec4 gSpecular;
layout (location = 5) in vec4 gRescolor;

// 
layout (location = DIFFUSED) out vec4 oDiffused;
layout (location = REFLECTP) out vec4 oSpecular;

// 
const vec2 shift[9] = {
    vec2(-1.f,-1.f),vec2(0.f,-1.f),vec2(1.f,-1.f),
    vec2(-1.f, 0.f),vec2(0.f, 0.f),vec2(1.f, 0.f),
    vec2(-1.f, 1.f),vec2(0.f, 1.f),vec2(1.f, 1.f)
};

// 
bool checkCorrect(in vec4 screenSample, in vec2 i2fxm) {
    for (int i=0;i<9;i++) {
        const vec2 offt = shift[i];

        vec4 worldspos = vec4(texture(frameBuffers[SAMPLEPT], vec2(i2fxm+offt), 0).xyz,1.f);
        vec4 almostpos = vec4(world2screen(worldspos.xyz),1.f);
        //almostpos.y *= -1.f;

        if (
            abs(almostpos.z-screenSample.z) < 0.0001f && 
            length(almostpos.xy-screenSample.xy) < 4.f && 
            dot(gNormal.xyz,    texture(frameBuffers[NORMALGM],  vec2(i2fxm+offt), 0).xyz) >=0.5f && 
                             texelFetch(frameBuffers[MASKDATA], ivec2(i2fxm+offt), 0).z > 0.f &&
            distance(wPosition.xyz,worldspos.xyz) < 0.05f || 
            (i == 4 && texelFetch(frameBuffers[COLORING], ivec2(i2fxm+offt), 0).w <= 0.01f) // Prefer use center texel for filling
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
    const ivec2 f2fx  = ivec2(gl_FragCoord.xy);
    const ivec2 size  = ivec2(textureSize(frameBuffers[SAMPLEPT], 0));
    const ivec2 i2fx  = ivec2(f2fx.x,size.y-f2fx.y-1);
    const  vec2 i2fxm = gl_FragCoord.xy; //vec2(gl_FragCoord.x,float(size.y)-gl_FragCoord.y);

    // 
    if (checkCorrect(vec4(gSample.xyz,1.f), i2fxm)) {
        oDiffused = gColor;
        oSpecular = vec4(gSpecular.xyz,gSpecular.w*0.5f); // TODO: Make New Reflection Sampling
    } else { discard; };
};
