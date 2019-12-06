#version 460 core
#extension GL_GOOGLE_include_directive : require
#extension GL_EXT_scalar_block_layout : require
#extension GL_EXT_shader_realtime_clock : require
#include "./index.glsl"

// 
layout (location = 0) in vec4 gColor;
layout (location = 0) out vec4 oDiffuse;

// 
void main() {
    oDiffuse = gColor;
};
