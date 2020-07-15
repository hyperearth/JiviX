#include "./driver.hlsli"

// 
//layout ( location = 0 ) in float2 vcoord;
//layout ( location = 0 ) out float4 uFragColor;

// Not Reprojected by Previous Frame
float4 getPosition   (in int2 map) { const int2 size = imageSize(writeBuffer[BW_POSITION]); return imageLoad(writeBuffer[BW_POSITION],int2(map.x,map.y)); };
float4 getNormal     (in int2 map) { const int2 size = imageSize(writeImages[IW_MAPNORML]); return imageLoad(writeImages[IW_MAPNORML],int2(map.x,map.y)); };
float4 getData       (in int2 map) { const int2 size = imageSize(writeImages[IW_MATERIAL]); return imageLoad(writeImages[IW_MATERIAL],int2(map.x,map.y)); };

// For current frame only
float4 getReflection (in int2 map) { const int2 size = imageSize(writeBuffer[BW_REFLECLR]); return imageLoad(writeBuffer[BW_REFLECLR],int2(map.x,map.y)); };
//float4 getReflection (in int2 map) { const int2 size = imageSize(writeImages[IW_REFLECLR]); return imageLoad(writeImages[IW_REFLECLR],int2(map.x,map.y)); };
float4 getTransparent(in int2 map) { const int2 size = imageSize(writeBuffer[BW_TRANSPAR]); return imageLoad(writeBuffer[BW_TRANSPAR],int2(map.x,map.y)); };
float4 getColor      (in int2 map) { const int2 size = imageSize(writeBuffer[BW_INDIRECT]); return imageLoad(writeBuffer[BW_INDIRECT],int2(map.x,map.y)); };

// With Resampling
float4 getSmoothed   (in int2 map) { const int2 size = imageSize(writeImages[IW_SMOOTHED]); return imageLoad(writeImages[IW_SMOOTHED],int2(map.x,map.y)); };
float4 getPReflection(in int2 map) { const int2 size = imageSize(writeImages[IW_REFLECLR]); return imageLoad(writeImages[IW_REFLECLR],int2(map.x,map.y)); };
float4 getFIndirect  (in int2 map) { const int2 size = imageSize(writeImages[IW_INDIRECT]); return imageLoad(writeImages[IW_INDIRECT],int2(map.x,map.y)); };

//float4 getIndirect   (in int2 map) { const int2 size = imageSize(writeBuffer[BW_INDIRECT]); return imageLoad(writeBuffer[BW_INDIRECT],int2(map.x,map.y)); }; // Player of Latest Sample
float4 getIndirect  (in int2 map) { const int2 size = imageSize(writeImages[IW_INDIRECT]); return imageLoad(writeImages[IW_INDIRECT],int2(map.x,map.y)); };

// 
void setColor      (in int2 map, in float4 color) { const int2 size = imageSize(writeBuffer[BW_INDIRECT]); imageStore(writeBuffer[BW_INDIRECT],int2(map.x,map.y),color); };
void setReflection (in int2 map, in float4 color) { const int2 size = imageSize(writeBuffer[BW_REFLECLR]); imageStore(writeBuffer[BW_REFLECLR],int2(map.x,map.y),color); };

// 
int2 mapc(in int2 map) {
    const int2 size = imageSize(writeImages[IW_REFLECLR]);
    return int2(map.x, map.y);
};

bool skyboxPixel(in int2 samplep) {
    const  float4 dataflat = getData(samplep);
    const uint4 datapass = floatBitsToUint(dataflat);
    const float2 texcoord = unpackUnorm2x16(datapass.x);
    return uintBitsToFloat(datapass.z) <= 0.f;
};

// 
float4 getDenoised(in int2 coord, in int type, in uint maxc) {
    float4 centerNormal = getNormal(coord);
    float3 centerOrigin = world2screen(getPosition(coord).xyz);

    float4 sampled = 0.f.xxxx; int scount = 0;
    float4 centerc = getFIndirect(coord); // Get Full Sampled and Previous Frame
    float4 samplep = max(centerc - getIndirect(coord) - 0.0001f, 0.f.xxxx);

    for (uint x=0;x<2;x++) {
        for (uint y=0;y<2;y++) {
            int2 map = coord+int2(x,y);
            float4 nsample = getNormal(map), psample = float4(world2screen(getPosition(map).xyz), 1.f);

            float4 samp = 0.f.xxxx; float simp = 1.f;
            if (type == 0) { samp = getIndirect   (map); simp = samp.w; };
            if (type == 1) { samp = getPReflection(map); simp = samp.w; };
            if (type == 2) { samp = getTransparent(map); simp = samp.w; };
            if (type == 3) { samp = getReflection (map); simp = samp.w; };
            if ((dot(nsample.xyz,centerNormal.xyz) >= 0.5f && distance(psample.xyz,centerOrigin.xyz) < 0.01f && abs(centerOrigin.z-psample.z) < 0.005f) || (x == 0 && y == 0 && samp.w > 0.f) || (centerc.w <= 0.0001f && scount <= 0)) {
                samp.xyz = clamp(samp.xyz/max(samp.w,0.5f), 0.f.xxx, type == 2 ? 2.f.xxx : 16.f.xxx)*samp.w; samp.w = simp;
                sampled += samp; if (simp > 0.f) { scount++; };
                break;
            };
        };
    };

    // 
    sampled /= max(float(scount), 1.f);
    if (type == 0) { sampled += samplep; };
    sampled.w = max(sampled.w, 1.f);
    return sampled;
};

// Checker Board BUG Fix
float fixedTranparency(in int2 samplep) {
    float center = getTransparent(samplep).w;
    float fixedt = center;
    float xt = getTransparent(samplep + int2(1, 0)).w;
    float yt = getTransparent(samplep + int2(0, 1)).w;

#ifndef HIGH_QUALITY_TRANSPARENCY
    if (fixedt < 0.0001f) { 
        if (xt > 0.0001f && yt > 0.0001f) { fixedt = .5f*(xt+ yt); } else 
                                          { fixedt =  max(xt, yt); };
    };
#endif
    return fixedt;
};


// TODO: Use Texcoord and Material ID's instead of Color, PBR-Map, Emission,  (due, needs only two or one buffers)
#ifdef GLSL
layout (local_size_x = 32u, local_size_y = 24u) in; 
void main()  // TODO: explicit sampling 
#else
[numthreads(32, 24, 1)]
void main(uint3 DTid : SV_DispatchThreadID) // TODO: explicit sampling 
#endif
{

#ifdef GLSL
const uint3 DTid = gl_GlobalInvocationID.xyz;
const uint3 GlobalInvocationID = gl_GlobalInvocationID.xyz;
#else
const uint3 GlobalInvocationID = DTid;
#endif

    // 
    const int2 size = imageSize(writeImages[IW_INDIRECT]), samplep = int2(GlobalInvocationID.xy);
    const  float4 dataflat = getData(samplep);
    const uint4 datapass = floatBitsToUint(dataflat);
    const  float4 position = getPosition(samplep);

    // 
    const float2 texcoord = unpackUnorm2x16(datapass.x);
    const bool isSkybox = uintBitsToFloat(datapass.z) <= 0.f;

    // 
    const MaterialUnit unit = materials[datapass.y];
          float4 diffused = toLinear(unit. diffuseTexture >= 0 ? textureSample(textures[nonuniformEXT(unit. diffuseTexture)],samplers[2u],texcoord.xy) : unit.diffuse);
          float4 emission = toLinear(unit.emissionTexture >= 0 ? textureSample(textures[nonuniformEXT(unit.emissionTexture)],samplers[2u],texcoord.xy) : unit.emission);
          float4 normaled = unit. normalsTexture >= 0 ? textureSample(textures[nonuniformEXT(unit. normalsTexture)],samplers[2u],texcoord.xy) : unit.normals;
          float4 specular = unit.specularTexture >= 0 ? textureSample(textures[nonuniformEXT(unit.specularTexture)],samplers[2u],texcoord.xy) : unit.specular;
          float4 dtexdata = diffused;

    // experimental (unused for alpha transparency)
    //if (diffused.w < 0.99f && !isSkybox) { diffused.xyz = 1.f.xxx, diffused.w = 1.f; } // doesnt't working
    //if (diffused.w < 0.99f) { diffused.xyz = 1.f.xxx; };

    // 
    const float3 camera = mul(getMT3x4(pushed.modelview), float4(position.xyz,1.f));
    const float3 raydir = mul(normalize(camera.xyz), getMT3x4(pushed.modelview)).xyz;
    const float3 origin = getPosition(samplep).xyz;
    const float3 normal = getNormal(samplep).xyz;

    // Updated for 12.05.2020
    diffused.xyz = max(diffused.xyz - emission.xyz, 0.f.xxx);
    //diffused = getSmoothed(samplep), diffused /= max(diffused.w, 1.f); // BROKEN!
    diffused.w = dtexdata.w; // Replace Into Diffuse Alpha Channel

    if (isSkybox) { diffused.xyz = 0.f.xxx, emission.xyz = gSkyShader(raydir.xyz, origin.xyz).xyz; };
    
    // 
    int denDepth = 3;
#ifndef LATE_STAGE
    float4 reflects = float4(0.f.xxxx);
    float4 coloring = getDenoised(samplep, 0, 9);
#else // 
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
    reflects /= max(reflects.w, 1.f);
    

    // 
    const float inIOR = 1.f, outIOR = 1.6666f;
    const float frefl = mix(clamp(pow(1.0f + dot(raydir.xyz, normal.xyz), outIOR/inIOR), 0.f, 1.f) * 0.3333f, 1.f, specular.z) * (isSkybox ? 0.f : 1.f);

    // Currently NOT denoised! (impolated with previous frame)
    float4 currentReflection = getDenoised(samplep, 3, denDepth), previousReflection = getDenoised(samplep, 1, denDepth);
    previousReflection /= max(previousReflection.w,1.f);
     currentReflection /= max( currentReflection.w,1.f);

    // 
    float4 transpar = getDenoised(samplep, 2, 3);
    transpar.xyz /= max(transpar.w, 0.5f);
    transpar.w = diffused.w <= 0.9999f ? 1.f : 0.f;
    float alpha = clamp((transpar.w * (1.f-diffused.w)) * (isSkybox ? 0.f : 1.f), 0.f, 1.f);

#ifndef LATE_STAGE
      // Reflection Still Required
      imageStore(writeImages[IW_REFLECLR], (samplep), reflects = float4(clamp(mix(previousReflection.xyz/max(previousReflection.w,1.f), currentReflection.xyz/max(currentReflection.w,1.f), (0.5f-specular.y*0.5f)), 0.f.xxx, 1.f.xxx), 1.f)); // Smoothed Reflections
      imageStore(writeBuffer[BW_RENDERED], (samplep), float4((coloring.xyz/max(coloring.w,1.f))*(diffused.xyz)+max(emission.xyz,0.f.xxx),1.f));
    //imageStore(writeBuffer[BW_RENDERED], (samplep), float4(mix((coloring.xyz/max(coloring.w,1.f))*(diffused.xyz/max(diffused.w,1.f))+max(emission.xyz,0.f.xxx),(reflects.xyz/max(reflects.w,1.f)),frefl),1.f));

    // Copy Into Back-Side
    [[unroll]] for (uint I=0;I<12u;I++) {
        imageStore(writeImagesBack[I], (samplep), imageLoad(writeImages[I], mapc(samplep)));
    };

    // Will very actual after adaptive denoise... 
    if (!isSkybox && getTransparent(samplep).w > 0.f) {
        float scount = max(imageLoad(writeImages[IW_INDIRECT], mapc(samplep)).w, 1.f);
        imageStore(writeImagesBack[IW_INDIRECT], mapc(samplep), float4(diffused.xyz*scount, scount));
    };
#else
      imageStore(writeBuffer[BW_RENDERED], (samplep), float4(clamp(mix(imageLoad(writeBuffer[BW_RENDERED], samplep).xyz, transpar.xyz/max(transpar.w,0.5f), alpha), 0.f.xxx, 1.f.xxx), 1.f));
      imageStore(writeBuffer[BW_RENDERED], (samplep), float4(clamp(mix(imageLoad(writeBuffer[BW_RENDERED], samplep).xyz, reflects.xyz/max(reflects.w,0.5f), frefl), 0.f.xxx, 1.f.xxx), 1.f));
    //imageStore(writeBuffer[BW_RENDERED], (samplep), float4(mix((coloring.xyz/max(coloring.w,1.f))*(diffused.xyz/max(diffused.w,1.f))+max(emission.xyz,0.f.xxx),(reflects.xyz/max(reflects.w,1.f)),frefl),1.f));
#endif

};
