#version 460 core // #
#extension GL_GOOGLE_include_directive  : require
#include "./driver.glsl"

// RESERVED for adaptive denoise and RTX-like reflections... 
layout (points) in;
layout (points, max_vertices = 1) out;

// 
layout (location = 0) in vec4 vColor[];
layout (location = 1) in vec4 vSample[];
layout (location = 2) in vec4 vNormal[];
layout (location = 3) in vec4 vPosition[];
layout (location = 4) in vec4 vSpecular[];
layout (location = 5) in vec4 vRescolor[];

// 
layout (location = 0) out vec4 gColor;
layout (location = 1) out vec4 gSample;
layout (location = 2) out vec4 gNormal;
layout (location = 3) out vec4 wPosition;
layout (location = 4) out vec4 gSpecular;
layout (location = 5) out vec4 gRescolor;

out gl_PerVertex {   // some subset of these members will be used
    vec4 gl_Position;
    float gl_PointSize;
};

// 
void main() {
    gl_Position = gl_in[0].gl_Position;

    gColor = vColor[0];
    gSample = vSample[0];
    gNormal = vNormal[0];
    wPosition = vPosition[0];
    gSpecular = vSpecular[0];
    gRescolor = vRescolor[0];

    EmitVertex(), EndPrimitive();
};
