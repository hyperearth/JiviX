#version 460 core
#extension GL_GOOGLE_include_directive  : require
#include "./driver.glsl"
// 
layout (location = 0) in vec4 gColor;
layout (location = 1) in vec4 gSample;
layout (location = 2) in vec4 wPosition;
layout (location = DIFFUSED) out vec4 oDiffused;
layout (location = SAMPLING) out vec4 oSampling;


// 
void main() { // Currently NO possible to compare
    const ivec2 f2fx = ivec2(gl_FragCoord.xy);
    const ivec2 size = ivec2(textureSize(frameBuffers[POSITION], 0));
    const ivec2 i2fx = ivec2(f2fx.x,size.y-f2fx.y);

    // world space
    vec4 positions = vec4(gSample.xyz,1.f); // SSP from previous frame got...
    vec4 almostpos = vec4(texture(frameBuffers[POSITION],gl_FragCoord.xy,0).xyz,1.f), worldspos = almostpos; // get world space from pixel
    almostpos = vec4(vec4(almostpos.xyz,1.f) * modelview, 1.f) * projection, almostpos.y *= -1.f, almostpos.xyz /= almostpos.w; // make-screen-space from world space

    // 
    const bool isBackground = all(fequal(texelFetch(frameBuffers[POSITION],i2fx,0).xyz,0.f.xxx)); // don't place into background
    //if (fequal(almostpos.z,positions.z) && distance(almostpos.xyz,positions.xyz) < 0.005f && all(lessThan(abs(almostpos.xy-positions.xy)*vec2(size.xy),1.f.xx)) && !isBackground) {
    if (fequal(almostpos.z,positions.z) && !isBackground) {
        oDiffused = gColor;
    } else {
        discard;
    };
};
