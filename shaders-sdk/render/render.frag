#version 460 core
#extension GL_GOOGLE_include_directive : require
#extension GL_EXT_scalar_block_layout : require

// Swap Buffers
#define DIFFUSE 0
#define REFLECT 1
#define SAMPLES 2 // Used for Position

#define COLORED 3
#define NORMALS 4
#define NORMMOD 5
#define PARAMET 6
#define DEPTHST 7

#define DENOISE 8
#define OUTPUTS 9

// 
layout ( location = 0 ) in vec2 vcoord;
layout ( location = 0 ) out vec4 uFragColor;
layout ( binding = 4, rgba32f ) uniform image2D writeImages[];

// 
void main() {
    //imageStore(outputImage, ivec2(vcoord*imageSize(outputImage)), vec4(1.f.xxx,1.f));
    const vec2 size = imageSize(writeImages[OUTPUTS]);
    vec2 coord = gl_FragCoord.xy; //coord.y = size.y - coord.y;
    uFragColor = vec4(imageLoad(writeImages[OUTPUTS],ivec2(coord)).xyz,1.f);
}
