
#include "./driver.hlsli"
#include "./index.hlsli"

// 
#define FETCH_FX(NAME, BUFFER) \
    const uint2 size = uint2(0u,0u); BUFFER[NAME].GetDimensions(size.x, size.y); return BUFFER[NAME].Load(uint3(map.x,map.y,0));

#define WRITE_FX(NAME, BUFFER) \
    const uint2 size = uint2(0u,0u); BUFFER[NAME].GetDimensions(size.x, size.y); writeBuffer[BW_INDIRECT][int2(map.x,map.y)] = color;

// Not Reprojected by Previous Frame
float4 getPosition   (in int2 map) { FETCH_FX(BW_POSITION, writeBuffer); };
float4 getNormal     (in int2 map) { FETCH_FX(IW_MAPNORML, writeImages); };
float4 getData       (in int2 map) { FETCH_FX(IW_MATERIAL, writeImages); };

// For current frame only
float4 getReflection (in int2 map) { FETCH_FX(BW_REFLECLR, writeBuffer); };
float4 getTransparent(in int2 map) { FETCH_FX(BW_TRANSPAR, writeBuffer); };
float4 getColor      (in int2 map) { FETCH_FX(BW_INDIRECT, writeBuffer); };

// With Resampling
float4 getSmoothed   (in int2 map) { FETCH_FX(IW_SMOOTHED, writeImages); };
float4 getIndirect   (in int2 map) { FETCH_FX(IW_INDIRECT, writeImages); };
float4 getPReflection(in int2 map) { FETCH_FX(IW_REFLECLR, writeImages); };

// 
void setColor      (in int2 map, in float4 color) { WRITE_FX(BW_INDIRECT, writeBuffer); };
void setReflection (in int2 map, in float4 color) { WRITE_FX(BW_REFLECLR, writeBuffer); };

// 
int2 mapc(in int2 map) {
    return int2(map.x, map.y);
}

bool skyboxPixel(in int2 samplep) {
    const  float4 dataflat = getData(samplep);
    const uint4 datapass = asuint(dataflat);
    const float2 texcoord = unpackUnorm2x16(datapass.x);
    return asfloat(datapass.z) <= 0.f;
}

// 
float4 getDenoised(in int2 coord, in int type, in uint maxc) {
    float4 centerNormal = getNormal(coord);
    float3 centerOrigin = world2screen(getPosition(coord).xyz);

    float4 sampled = 0.f.xxxx; int scount = 0;
    float4 centerc = getIndirect(coord);

    for (uint x=0;x<maxc;x++) {
        for (uint y=0;y<maxc;y++) {
            int2 map = coord+int2(x-(maxc>>1),y-(maxc>>1));
            float4 nsample = getNormal(map), psample = float4(world2screen(getPosition(map).xyz), 1.f);

            if (dot(nsample.xyz,centerNormal.xyz) >= 0.5f && distance(psample.xyz,centerOrigin.xyz) < 0.01f && abs(centerOrigin.z-psample.z) < 0.005f || (x == (maxc>>1) && y == (maxc>>1)) || centerc.w <= 0.0001f && sampled.w <= 0.0001f) {
                float4 samp = 0.f.xxxx; float simp = 1.f;
                if (type == 0) { samp = getIndirect   (map); simp = samp.w; };
                if (type == 1) { samp = getPReflection(map); simp = samp.w; };
                if (type == 2) { samp = getTransparent(map); simp = samp.w; };

                // 
                samp.xyz = clamp(samp.xyz/max(samp.w,0.5f), 0.f.xxx, type == 2 ? 2.f.xxx : 16.f.xxx)*samp.w; samp.w = simp;
                sampled += samp;
            };
        };
    };

    //if (reflection) {
    //    sampled.xyz /= sampled.w;
    //    sampled.w = getPReflection(coord).w;
    //};

    sampled.w = max(sampled.w, 1.f);
    return sampled;
};

// Checker Board BUG Fix
float fixedTranparency(in int2 samplep) {
    float center = getTransparent(samplep).w;
    float fixedt = center;
    float xt = getTransparent(samplep + int2(1, 0)).w;
    float yt = getTransparent(samplep + int2(0, 1)).w;

    if (fixedt < 0.0001f) { 
        if (xt > 0.0001f && yt > 0.0001f) { fixedt = .5f*(xt+ yt); } else 
                                          { fixedt =  max(xt, yt); };
    };
    return fixedt;
};


// TODO: Use Texcoord and Material ID's instead of Color, PBR-Map, Emission,  (due, needs only two or one buffers)
[numthreads(32, 24, 1)]
void main(uint3 DTid : SV_DispatchThreadID) { // TODO: explicit sampling 
    const uint2 size = int2(0,0); writeImages[IW_INDIRECT].GetDimensions(size.x, size.y);
    const uint2 samplep = uint2(DTid.xy);
    const  float4 dataflat = getData(samplep);
    const uint4 datapass = asuint(dataflat);
    const  float4 position = getPosition(samplep);

    // 
    const float2 texcoord = unpackUnorm2x16(datapass.x);
    const bool isSkybox = asfloat(datapass.z) <= 0.f;

    // 
    const MaterialUnit unit = materials[0u][datapass.y];
          float4 diffused = toLinear(unit. diffuseTexture >= 0 ? textures[unit. diffuseTexture].Sample(samplers[2u],texcoord.xy) : unit.diffuse);
          float4 emission = toLinear(unit.emissionTexture >= 0 ? textures[unit.emissionTexture].Sample(samplers[2u],texcoord.xy) : unit.emission);
          float4 normaled = unit. normalsTexture >= 0 ? textures[unit. normalsTexture].Sample(samplers[2u],texcoord.xy) : unit.normals;
          float4 specular = unit.specularTexture >= 0 ? textures[unit.specularTexture].Sample(samplers[2u],texcoord.xy) : unit.specular;
          float4 dtexdata = diffused;

    // experimental (unused for alpha transparency)
    //if (diffused.w < 0.99f && !isSkybox) { diffused.xyz = 1.f.xxx, diffused.w = 1.f; } // doesnt't working
    //if (diffused.w < 0.99f) { diffused.xyz = 1.f.xxx; };

    // 
    const float3 camera = mul(pushed.modelview, float4(position.xyz,1.f));
    const float3 raydir = mul(normalize(camera.xyz), pushed.modelview).xyz;
    const float3 origin = getPosition(samplep).xyz;
    const float3 normal = getNormal(samplep).xyz;

    // Updated for 12.05.2020
    diffused.xyz = max(diffused.xyz - emission.xyz, 0.f.xxx);
    //diffused = getSmoothed(samplep), diffused /= max(diffused.w, 1.f); // BROKEN!
    diffused.w = dtexdata.w; // Replace Into Diffuse Alpha Channel

    if (isSkybox) { diffused.xyz = 0.f.xxx, emission.xyz = gSkyShader(raydir.xyz, origin.xyz).xyz; };
    
    // 
#ifndef LATE_STAGE
    float4 reflects = float4(0.f.xxxx);
    float4 coloring = getDenoised(samplep, 0, 9);
#else // 
    int denDepth = 3;
    if (specular.y > 0.3333f) denDepth = 5;
    if (specular.y > 0.6666f) denDepth = 7;
    if (specular.y > 0.9999f) denDepth = 9;
    float4 reflects = getDenoised(samplep, 1, denDepth);
    float4 coloring = float4(0.f.xxxx);
#endif

    // 
    if (reflects.w <= 0.f) { reflects = float4(0.f.xxx,0.f); };
    if (coloring.w <= 0.f) { coloring = float4(0.f.xxx,1.f); };
    coloring = max(coloring, 0.f.xxxx);
    reflects = max(reflects, 0.f.xxxx);

    // 
    const float inIOR = 1.f, outIOR = 1.6666f;
    const float frefl = lerp(clamp(pow(1.0f + dot(raydir.xyz, normal.xyz), outIOR/inIOR), 0.f, 1.f) * 0.3333f, 1.f, specular.z) * (isSkybox ? 0.f : 1.f);

    // Currently NOT denoised! (impolated with previous frame)
    float4 currentReflection = getReflection(samplep), previousReflection = getPReflection(samplep);
    previousReflection = (previousReflection/max(previousReflection.w,1.f));
     currentReflection = ( currentReflection/max( currentReflection.w,1.f));

    // 
    float4 transpar = getDenoised(samplep, 2, 3);
    transpar.xyz /= max(transpar.w, 0.5f);
    transpar.w = diffused.w <= 0.9999f ? 1.f : 0.f;
    float alpha = clamp((transpar.w * (1.f-diffused.w)) * (isSkybox ? 0.f : 1.f), 0.f, 1.f);

#ifndef LATE_STAGE
    writeImages[IW_REFLECLR][mapc(samplep)] = (reflects = float4(clamp(lerp(previousReflection.xyz/max(previousReflection.w,1.f), currentReflection.xyz/max(currentReflection.w,1.f), (1.f-specular.y*0.5f)), 0.f.xxx, 1.f.xxx), 1.f));
    writeBuffer[BW_RENDERED][     samplep ] = float4((coloring.xyz/max(coloring.w,1.f))*(diffused.xyz)+max(emission.xyz,0.f.xxx),1.f);

    // Copy Into Back-Side
    [unroll] for (uint I=0;I<12u;I++) {
        writeImagesBack[I][mapc(samplep)] = writeImages[I][mapc(samplep)];
    };

    // Will very actual after adaptive denoise... 
    if (!isSkybox && getTransparent(samplep).w > 0.f) {
        float scount = max(writeImages[IW_INDIRECT][mapc(samplep)].w, 1.f);
        writeImagesBack[IW_INDIRECT][mapc(samplep)] = float4(diffused.xyz*scount, scount);
    };
#else
    writeBuffer[BW_RENDERED][samplep] = float4(clamp(mix(writeBuffer[BW_RENDERED][samplep].xyz, transpar.xyz/max(transpar.w,0.5f), alpha), 0.f.xxx, 1.f.xxx), 1.f);
    writeBuffer[BW_RENDERED][samplep] = float4(clamp(mix(writeBuffer[BW_RENDERED][samplep].xyz, reflects.xyz/max(reflects.w,0.5f), frefl), 0.f.xxx, 1.f.xxx), 1.f);

    //imageStore(writeBuffer[BW_RENDERED],   (samplep), float4(mix((coloring.xyz/max(coloring.w,1.f))*(diffused.xyz/max(diffused.w,1.f))+max(emission.xyz,0.f.xxx),(reflects.xyz/max(reflects.w,1.f)),frefl),1.f));
#endif

};
