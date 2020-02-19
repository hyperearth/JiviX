#version 460 core // #
#extension GL_GOOGLE_include_directive  : require
#include "./driver.glsl"
// 
layout (location = 0) out vec4 gColor;
layout (location = 1) out vec4 gSample;
layout (location = 2) out vec4 gNormal;
layout (location = 3) out vec4 wPosition;
layout (location = 4) out vec4 gSpecular;


out gl_PerVertex {   // some subset of these members will be used
    vec4 gl_Position;
    float gl_PointSize;
};

// 
void main() {
    //const uint idx = gl_VertexIndex;
    const ivec2 size = imageSize(writeImages[0]  );
    const ivec2 f2fx = ivec2(gl_VertexIndex, gl_InstanceIndex);
    const ivec2 i2fx = ivec2(size.x,size.y-f2fx.y-1);

    // 
    const vec4 positions = imageLoad(writeImages[SAMPLING],f2fx); // from previous frame 
    const vec4 diffcolor = imageLoad(writeImages[DIFFUSED],f2fx); // 
    const vec4 normaling = imageLoad(writeImages[NORMALED],f2fx);
    const vec4 speccolor = imageLoad(writeImages[REFLECTS],f2fx);
    
    // 
    gl_PointSize = 0, gColor = 0.f.xxxx, gNormal.xxxx, wPosition = 0.f.xxxx;
    if (diffcolor.w > 0.f && !all(fequal(positions.xyz,0.f.xxx))) { // set into current 
        gl_Position = vec4(world2screen(positions.xyz),1.f), gl_Position.y *= -1.f, gl_PointSize = 1.f;
        gColor = clamp(diffcolor, 0.001f, 10000000.f);
        gSpecular = vec4(speccolor.xyz,1.f);
        gSample = vec4(vec3(gl_Position.x,gl_Position.y*-1.f,gl_Position.z),1.f);
        gNormal = vec4(normaling.xyz,1.f);
        wPosition = positions;
    };
};
