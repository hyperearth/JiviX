#version 460 core
#extension GL_GOOGLE_include_directive : require
#extension GL_EXT_scalar_block_layout : require
#extension GL_EXT_shader_realtime_clock : require
precision highp float;
precision highp int;
#include "./index.glsl"

// 
//layout (location = 0) in vec4 iPosition;
layout (location = 0) in vec3 iPosition;
layout (location = 1) in vec4 iNormals;
layout (location = 2) in vec4 iTexcoords;
layout (location = 3) in vec4 iTangents;

// 
layout (location = 0) out vec4 gPosition;
layout (location = 1) out vec4 gNormals;
layout (location = 2) out vec4 gTexcoords;
layout (location = 3) out vec4 gTangents;
layout (location = 4) flat out ivec4 gIndexes;

// 
void main() {
    gTexcoords = iTexcoords;
    gPosition = vec4(iPosition,1.f);
    gTangents = iTangents;
    gNormals = iNormals;

    // Final Output 
    gl_Position = vec4(vec4(iPosition,1.f) * modelview, 1.f) * projection;
};
