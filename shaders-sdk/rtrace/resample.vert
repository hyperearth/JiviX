#version 460 core
#extension GL_GOOGLE_include_directive  : require
#include "./driver.glsl"
// 
layout (location = 0) out vec4 gColor;
layout (location = 1) out vec4 gSample;
layout (location = 2) out vec4 wPosition;

// 
void main() {
    //const uint idx = gl_VertexIndex;
    const ivec2 size = imageSize(writeImages[0]  );
    const ivec2 f2fx = ivec2(gl_VertexIndex, gl_InstanceIndex);

    // 
    const vec4 positions = imageLoad(writeImages[SAMPLING],f2fx); // from previous frame 
    const vec4 diffcolor = imageLoad(writeImages[DIFFUSED],f2fx); // 

    // 
    gl_PointSize = 0; gColor = 0.f.xxxx; wPosition = 0.f.xxxx;
    if (diffcolor.w > 0.f && !all(equal(positions.xyz,0.f.xxx))) { // set into current 
        gl_Position = vec4(vec4(positions.xyz,1.f) * modelview, 1.f) * projection, gl_PointSize = 1, gl_Position.y *= -1.f;
        wPosition = positions;
        gColor = clamp(diffcolor, 0.001f, 10000000.f);
        gSample = vec4(gl_Position.xyz / gl_Position.w,1.f);
    };
};
