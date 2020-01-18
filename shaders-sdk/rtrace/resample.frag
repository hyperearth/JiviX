#version 460 core
#extension GL_GOOGLE_include_directive  : require
#include "./driver.glsl"
// 
layout (location = 0) in vec4 gColor;
layout (location = 1) in vec4 gSample;
layout (location = DIFFUSED) out vec4 oDiffused;
layout (location = SAMPLING) out vec4 oSampling;

// 
void main() { // Currently NO possible to compare
    const ivec2 f2fx = ivec2(gl_FragCoord.xy);
    const ivec2 size = ivec2(textureSize(frameBuffers[POSITION], 0));
    const ivec2 i2fx = ivec2(f2fx.x,size.y-f2fx.y);

    // world space
    vec4 positions = vec4(gSample.xyz,1.f); // from previous frame got...
    vec4 almostpos = vec4(texelFetch(frameBuffers[POSITION],i2fx,0).xyz,1.f); // get current position of pixel
    almostpos = vec4(vec4(almostpos.xyz,1.f) * modelview, 1.f) * projection, almostpos.y *= -1.f, almostpos.xyz /= almostpos.w; // make-world space
    //positions = vec4(vec4(divW(vec4(positions.xyz,1.f) * projectionInv), 1.f)*modelviewInv,1.f), positions.z = gl_FragCoord.y; 

    // 
    if (distance(almostpos.xyz,positions.xyz) < 0.005f) { // TODO: Enable When Will Full Polygons
        oDiffused = gColor;
        oSampling = vec4(0.f);
    } else {
        oDiffused = vec4(0.f);
        oSampling = vec4(0.f);
    };

    //oDiffused = vec4((almostpos.xyz-positions.xyz)*0.5f+0.5f,1.f);
};
