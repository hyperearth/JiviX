#version 460 core // #
#extension GL_GOOGLE_include_directive          : require
#include "./driver.glsl"

// 
layout ( location = 0 ) in vec2 vcoord;
layout ( location = 1 ) in vec4 position;

layout (location = COLORING) out vec4 colored;
layout (location = POSITION) out vec4 samples;
layout (location = NORMALED) out vec4 normals;
layout (location = TANGENTS) out vec4 tangent;

// 
void main() {
    const vec2 size = imageSize(writeImages[DIFFUSED]);
    vec2 coord = gl_FragCoord.xy; coord.y = size.y - coord.y;
    vec4 samples = max(imageLoad(writeImages[DIFFUSED],ivec2(coord)),0.0001f.xxxx); samples.xyz /= samples.w;
    //samples = vec4(vec4(normalize(position.xyz)*10000.f,1.f)*modelviewInv,1.f);
    samples = vec4(vec3(0.f,0.f,0.f),0.f);
    //colored = vec4(0.f.xxx,1.f);
    colored = vec4(gSkyColor,1.f);
    normals = vec4((modelview * normalize(vec3(0.f, 0.f, 1.f))).xyz, 0.f);

    gl_FragDepth = 1.f; 
    imageStore(writeImages[REFLECTS], ivec2(gl_FragCoord.x,size.y-gl_FragCoord.y), vec4(0.f,0.f,0.f,0.f));
    imageStore(writeImages[DIFFUSED], ivec2(gl_FragCoord.x,size.y-gl_FragCoord.y), vec4(0.f,0.f,0.f,0.f));
    imageStore(writeImages[NORMALED], ivec2(gl_FragCoord.x,size.y-gl_FragCoord.y), vec4(0.f,0.f,0.f,0.f));
    imageStore(writeImages[SAMPLING], ivec2(gl_FragCoord.x,size.y-gl_FragCoord.y), vec4(samples.xyz,0.f));
};
