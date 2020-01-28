#version 460 core // #
#extension GL_GOOGLE_include_directive  : require
#include "./driver.glsl"
// 
layout (location = 0) in vec4 gColor;
layout (location = 1) in vec4 gSample;
layout (location = 2) in vec4 gNormal;
layout (location = 3) in vec4 wPosition;
layout (location = DIFFUSED) out vec4 oDiffused;
layout (location = SAMPLING) out vec4 oSampling;


// 
void main() { // Currently NO possible to compare
    const ivec2 f2fx = ivec2(gl_FragCoord.xy);
    const ivec2 size = ivec2(textureSize(frameBuffers[POSITION], 0));
    const ivec2 i2fx = ivec2(f2fx.x,size.y-f2fx.y-1);
    const vec2 i2fxm = vec2(gl_FragCoord.x,float(size.y)-gl_FragCoord.y);

    // world space
    vec4 positions = vec4(gSample.xyz,1.f); // SSP from previous frame got...
    vec4 almostpos = vec4(texture(frameBuffers[POSITION],i2fxm).xyz,1.f), worldspos = almostpos; // get world space from pixel
    vec4 normaling = vec4(texture(frameBuffers[NORMALED],i2fxm).xyz,1.f);
    almostpos = vec4(world2screen(almostpos.xyz),1.f);//, almostpos.y *= -1.f, almostpos.xyz /= almostpos.w; // make-screen-space from world space

    // 
    const bool isBackground = all(fequal(texelFetch(frameBuffers[POSITION],i2fx.xy,0).xyz,0.f.xxx));
    if (abs(almostpos.z-positions.z) < 0.0001f && dot(gNormal.xyz,normaling.xyz)>=0.5f && distance(wPosition.xyz,worldspos.xyz) < 0.05f && !isBackground) {
        oDiffused = gColor;
    } else {
        discard;
    };
};
