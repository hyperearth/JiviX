#version 460 core
#extension GL_GOOGLE_include_directive : require
#extension GL_EXT_scalar_block_layout : require
#extension GL_EXT_shader_realtime_clock : require
precision highp float;
precision highp int;
#include "./index.glsl"

// 
layout ( location = 0 ) in vec2 vcoord;
layout ( location = 0 ) out vec4 uFragColor;

// 
void main() {
    //imageStore(outputImage, ivec2(vcoord*imageSize(outputImage)), vec4(1.f.xxx,1.f));
    const vec2 size = textureSize(frameBuffers[DIFFUSE],0);
    vec2 coord = gl_FragCoord.xy; //coord.y = size.y - coord.y;
    coord.y = size.y - coord.y;
    vec4 samples = max(texelFetch(frameBuffers[DIFFUSE],ivec2(coord),0),0.0001f.xxxx); samples.xyz /= samples.w;
    uFragColor = vec4(0.f.xxx,1.f);
    if (samples.w >= 0.001f) uFragColor = vec4(samples.xyz*texelFetch(frameBuffers[COLORED],ivec2(coord),0).xyz,1.f);
    //uFragColor = vec4(samples.xyz*texelFetch(frameBuffers[COLORED],ivec2(coord),0).xyz,1.f);
    //uFragColor = samples;
}
