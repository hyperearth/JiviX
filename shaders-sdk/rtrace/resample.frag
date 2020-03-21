#version 460 core // #
#extension GL_GOOGLE_include_directive  : require
#include "./driver.glsl"
// 
layout (location = 0) in vec4 gColor;
layout (location = 1) in vec4 gSample;
layout (location = 2) in vec4 gNormal;
layout (location = 3) in vec4 wPosition;
layout (location = 4) in vec4 gSpecular;
layout (location = 5) in vec4 gRescolor;

//gRescolor
layout (location = DIFFUSED) out vec4 oDiffused;
layout (location = SAMPLING) out vec4 oSampling;
layout (location = REFLECTS) out vec4 oSpecular;
layout (location = RESCOLOR) out vec4 oRescolor;

const vec2 shift[9] = {
    vec2(-1.f,-1.f),vec2(0.f,-1.f),vec2(1.f,-1.f),
    vec2(-1.f, 0.f),vec2(0.f, 0.f),vec2(1.f, 0.f),
    vec2(-1.f, 1.f),vec2(0.f, 1.f),vec2(1.f, 1.f)
};

bool checkCorrect(in vec4 positions, in vec2 i2fxm) {
    for (int i=0;i<9;i++) {
        const vec2 offt = shift[i];

        vec4 almostpos = vec4(texture(frameBuffers[POSITION],i2fxm+offt).xyz,1.f), worldspos = almostpos; // get world space from pixel
        vec4 normaling = vec4(texture(frameBuffers[GEONORML],i2fxm+offt).xyz,1.f);
        almostpos = vec4(world2screen(almostpos.xyz),1.f);//, almostpos.y *= -1.f, almostpos.xyz /= almostpos.w; // make-screen-space from world space

        if (abs(almostpos.z-positions.z) < 0.0001f && dot(gNormal.xyz,normaling.xyz)>=0.5f && distance(wPosition.xyz,worldspos.xyz) < 0.05f) {
            return true;
        };
    };
    return false;
}

// WE NEEDS: 
// - GL_NV_shader_atomic_float
// - GL_NV_shader_atomic_fp16_vector
// - GL_NV_shader_atomic_float64
// FOR COMPUTE SHADERS! 

// 
void main() { // Currently NO possible to compare
    const ivec2 f2fx  = ivec2(gl_FragCoord.xy);
    const ivec2 size  = ivec2(textureSize(frameBuffers[POSITION], 0));
    const ivec2 i2fx  = ivec2(f2fx.x,size.y-f2fx.y-1);
    const  vec2 i2fxm =  vec2(gl_FragCoord.x,float(size.y)-gl_FragCoord.y);

    // 
    if (checkCorrect(vec4(gSample.xyz,1.f), i2fxm)) {
        oDiffused = gColor;
        oSpecular = gSpecular;
        oRescolor = gRescolor;
    } else { discard; };
};
