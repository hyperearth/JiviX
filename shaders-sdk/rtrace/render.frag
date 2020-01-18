#version 460 core
#extension GL_GOOGLE_include_directive          : require
#include "./driver.glsl"

// 
layout ( location = 0 ) in vec2 vcoord;
layout ( location = 0 ) out vec4 uFragColor;

// 
vec4 getIndirect(in ivec2 swapc){
    vec4 samples = imageLoad(writeImages[DIFFUSED],ivec2(swapc)); samples = max(samples, 0.001f); samples.xyz /= samples.w;
    return samples;
};

vec4 getNormal(in ivec2 coord){
    vec4 normals = vec4(texelFetch(frameBuffers[NORMALED],ivec2(coord),0).xyz, 0.f);
    return normals;
};

// bubble sort horror
void sort(inout vec3 arr[9u], int d)
{
    vec3 temp;
    for(int i=0;i<d*d-1;++i)
    {
        for(int j=i+1;j<d*d;++j)
        {
            // my super multicomponent branchless arithmetic swap
            // based on: a <- a+b, b <- a - b, a <- a - b
            vec3 g = vec3(greaterThan(arr[i],arr[j]));
            arr[i] += arr[j];
            arr[j] = g*arr[i] - (2.0*g-vec3(1.0))*arr[j];
            arr[i] -= arr[j];
        }
    }
}

vec4 getDenoised(in ivec2 coord, in ivec2 swapc){
    vec4 centerColor = getIndirect(swapc);
    vec4 centerNormal = getNormal(coord);
    //vec4 sampleBlur = 0.f.xxxx;
    
    vec3 samples[9u]; int scount = 0;
    for (uint x=0;x<3u;x++) {
        for (uint y=0;y<3u;y++) {
            vec4 csample = getIndirect(swapc+ivec2(x-1u,y-1u));
            vec4 nsample = getNormal(coord+ivec2(x-1u,y-1u));

            if (dot(nsample.xyz,centerNormal.xyz) > 0.5f || (x == 4u && y == 4u)) {
                //sampleBlur += vec4(csample.xyz,1.f);
                samples[scount++].xyz = csample.xyz;
            };
        };
    };

    if (scount > 0) sort(samples,int(sqrt(float(scount))));
    return vec4(samples[scount>>1u],1.f);
};

// 
void main() {
    const ivec2 size = imageSize(writeImages[DIFFUSED]);
    vec2 coord = gl_FragCoord.xy, swapc = gl_FragCoord.xy; coord.y = float(size.y) - coord.y;
    //vec4 samples = imageLoad(writeImages[DIFFUSED],ivec2(swapc)); samples = max(samples, 0.001f); samples.xyz /= samples.w;
    vec4 samples = getDenoised(ivec2(coord),ivec2(swapc));
    //if (samples.w >= 0.001f) uFragColor = vec4(samples.xyz*vec3(1.f,1.f,1.f),1.f);
    if (samples.w >= 0.001f) uFragColor = vec4(samples.xyz*texelFetch(frameBuffers[COLORING],ivec2(coord),0).xyz,1.f);
    //if (samples.w >= 0.001f) uFragColor = vec4(samples.xyz.xyz,1.f);
};
