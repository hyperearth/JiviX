#include "./driver.glsl"

// RESERVED for adaptive denoise and RTX-like reflections... 
layout (points) in;
layout (points, max_vertices = 1) out;

// 
layout (location = 0) in float4 vColor[];
layout (location = 1) in float4 vSample[];
layout (location = 2) in float4 vNormal[];
layout (location = 3) in float4 vPosition[];
layout (location = 4) in float4 vSpecular[];
layout (location = 5) in float4 vRescolor[];
layout (location = 6) in float4 vSmooth[];

// 
layout (location = 0) out float4 gColor;
layout (location = 1) out float4 gSample;
layout (location = 2) out float4 gNormal;
layout (location = 3) out float4 wPosition;
layout (location = 4) out float4 gSpecular;
layout (location = 5) out float4 gRescolor;
layout (location = 6) out float4 gSmooth;

out gl_PerVertex {   // some subset of these members will be used
    float4 gl_Position;
    float gl_PointSize;
};

// 
void main() {
    gl_Position = gl_in[0].gl_Position;
    gl_PointSize = 1.f;

    gColor = vColor[0];
    gSample = vSample[0];
    gNormal = vNormal[0];
    wPosition = vPosition[0];
    gSpecular = vSpecular[0];
    gRescolor = vRescolor[0];
    gSmooth = vSmooth[0];

    EmitVertex(), EndPrimitive();
};
