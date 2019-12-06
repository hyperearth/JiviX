#version 460 core
#extension GL_GOOGLE_include_directive : require
#extension GL_EXT_scalar_block_layout : require
#include "./index.glsl"

// 
layout ( location = 0 ) in vec2 vcoord;
layout ( location = 0 ) out vec4 uFragColor;

// 
void main() {
    //imageStore(outputImage, ivec2(vcoord*imageSize(outputImage)), vec4(1.f.xxx,1.f));
    const vec2 size = imageSize(writeImages[OUTPUTS]);
    vec2 coord = gl_FragCoord.xy; //coord.y = size.y - coord.y;
    uFragColor = vec4(imageLoad(writeImages[OUTPUTS],ivec2(coord)).xyz,1.f);
}
