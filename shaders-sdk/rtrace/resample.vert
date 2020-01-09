#version 460 core
#extension GL_GOOGLE_include_directive : require
#extension GL_EXT_scalar_block_layout : require
#extension GL_EXT_shader_realtime_clock : require
precision highp float;
precision highp int;
#include "./index.glsl"

// 
layout (location = 0) out vec4 gColor;

const vec2 cpositions[4] = { vec2(-1.f, 1.f), vec2(1.f, 1.f), vec2(-1.f, -1.f), vec2(1.f, -1.f) };
const vec2 tcoords[4] = { vec2(0.f), vec2(1.f, 0.f), vec2(0.f, 1.f), vec2(1.f) };

// 
void main() {
    const uint idx = gl_VertexIndex;
    const ivec2 size = textureSize(frameBuffers[0],0);
    const ivec2 f2fx = ivec2(idx%size.x,idx/size.x);

    // 
    const vec4 positions = texelFetch(frameBuffers[SAMPLING],f2fx,0);
    const vec4 diffcolor = texelFetch(frameBuffers[DIFFUSED],f2fx,0);

    gl_PointSize = 0; gColor = 0.f.xxxx;
    if (diffcolor.w > 0.f) {
        gl_Position = vec4(vec4(positions.xyz,1.f) * modelview, 1.f) * projection;
        gl_PointSize = 1;
        gColor = clamp(diffcolor, 0.f, 100000.f);
        //imageAtomicExchange(writeImages[DIFFUSE], f2fx, gColor);
        //imageStore(writeImages[DIFFUSE], f2fx, diffcolor);
    };

    //gl_PointSize = 1;
    //gl_Position = vec4(vec4(positions.xyz,1.f) * modelview, 1.f) * projection;
    //gl_Position = vec4(vec2(f2fx)/vec2(size)*2.f-1.f, 0.f, 1.f);

    //gl_Position = vec4(cpositions[gl_VertexIndex].xy, 0.0f, 1.0f);
    //gColor = vec4(1.f,0.f,1.f,1.f);//diffcolor;
};
