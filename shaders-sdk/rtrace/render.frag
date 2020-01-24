#version 460 core
#extension GL_GOOGLE_include_directive          : require
#include "./driver.glsl"

// 
layout ( location = 0 ) in vec2 vcoord;
layout ( location = 0 ) out vec4 uFragColor;

// 
vec4 getIndirect(in ivec2 map){
    const ivec2 size = imageSize(writeImages[DIFFUSED]);
    //vec4 samples = imageLoad(writeImages[DIFFUSED],ivec2(map.x,size.y-map.y-1)); samples = max(samples, 0.001f); samples.xyz /= samples.w;
    //return samples;
    return imageLoad(writeImages[DIFFUSED],ivec2(map.x,size.y-map.y-1));
};

vec4 getReflection(in ivec2 map){
    const ivec2 size = imageSize(writeImages[REFLECTS]);
    return imageLoad(writeImages[REFLECTS],ivec2(map.x,size.y-map.y-1));
};

vec4 getNormal(in ivec2 coord){
    vec4 normals = vec4(texelFetch(frameBuffers[NORMALED],ivec2(coord),0).xyz, 0.f);
    return normals;
};

vec4 getPosition(in ivec2 coord){
    vec4 position = vec4(texelFetch(frameBuffers[POSITION],ivec2(coord),0).xyz, 0.f);
    return position;
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

vec4 getDenoised(in ivec2 coord) {
    vec4 centerNormal = getNormal(coord);
    vec3 centerOrigin = world2screen(getPosition(coord).xyz);
    
    /*
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
    */

    vec4 sampled = 0.f.xxxx; int scount = 0;
    for (uint x=0;x<5u;x++) {
        for (uint y=0;y<5u;y++) {
            ivec2 map = coord+ivec2(x-2u,y-2u);
            vec4 nsample = getNormal(map);
            vec3 psample = world2screen(getPosition(map).xyz);

            if (dot(nsample.xyz,centerNormal.xyz) >= 0.5f && distance(psample.xyz,centerOrigin.xyz) < 0.01f && abs(centerOrigin.z-psample.z) < 0.005f || (x == 2u && y == 2u)) {
                sampled += getIndirect(map);
            };
        };
    };

    sampled = max(sampled, 0.001f);
    return sampled /= sampled.w;
};

// 
void main() {
    const ivec2 size = imageSize(writeImages[DIFFUSED]), samplep = ivec2(gl_FragCoord.x,float(size.y)-gl_FragCoord.y);
    const vec4 emission = texelFetch(frameBuffers[EMISSION],samplep,0);
    const vec4 diffused = texelFetch(frameBuffers[COLORING],samplep,0);
    const vec4 reflects = getReflection(samplep);

    uFragColor = vec4(mix(getDenoised(samplep).xyz*diffused.xyz+emission.xyz,reflects.xyz,reflects.w),1.f);
};
