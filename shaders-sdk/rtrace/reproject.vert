#version 460 core
#extension GL_GOOGLE_include_directive : require
#extension GL_EXT_scalar_block_layout : require
#include "./index.glsl"

// 
layout (location = 0) out vec4 gColor;

// 
void main() {
    const uint idx = gl_VertexIndex;
    const ivec2 size = textureSize(frameBuffers[0],0);
    const ivec2 f2fx = ivec2(idx%size.x,idx/size.x);

    // 
    const vec4 positions = texelFetch(frameBuffers[SAMPLES],f2fx,0);
    const vec4 diffcolor = texelFetch(frameBuffers[DIFFUSE],f2fx,0);

    gl_PointSize = 0; gColor = 0.f.xxxx;
    if (diffcolor.w > 0.f) {
        gl_Position = (positions.xyz * modelview) * projection;
        gl_PointSize = 1;
        gColor = diffcolor;
    };
};
