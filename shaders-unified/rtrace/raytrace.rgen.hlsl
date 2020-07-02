#ifdef GLSL
#extension GL_EXT_ray_tracing          : require
#endif

#ifndef ENABLE_AS
#define ENABLE_AS
#endif

#include "./driver.hlsli"
#include "./global.hlsli"

// 
#define RAY_TRACE
#define FAST_BW_TRANSPARENT false

//#define FAST_BW_TRANSPARENT // Can be denoised, but with WRONG results!
//#define TOP_LAYERED // Has reflection problems

// 
//layout (local_size_x = 32u, local_size_y = 24u) in;
#ifdef GLSL
layout ( location = 0u ) rayPayloadEXT CHIT hit;
#else
STATIC CHIT hit; // Yep, HLSL!
#endif

// For Cache
STATIC XHIT RES;

// 
#ifndef GLSL
void traceRayEXT(in uint flags, in uint mask, in uint a, in uint stride, in uint b, in float3 origin, in float minT, in float3 direct, in float maxT) {
    RayDesc desc;
    desc.Origin = origin;
    desc.TMin = minT;
    desc.Direction = direct;
    desc.TMax = maxT;

    // 
    TraceRay(Scene, flags, mask, a, stride, b, desc, hit);
};
#else
void traceRayEXT(in uint flags, in lowp uint mask, in uint a, in uint stride, in uint b, in float3 origin, in float minT, in float3 direct, in float maxT) {
    traceRayEXT(Scene, flags, mask, a, stride, b, origin, minT, direct, maxT, 0);
};

#define RAY_FLAG_FORCE_OPAQUE gl_RayFlagsOpaqueEXT
#define RAY_FLAG_CULL_BACK_FACING_TRIANGLES gl_RayFlagsCullBackFacingTrianglesEXT

#endif

// Ray Query Broken In Latest Driver... 
XHIT traceRays(in float3 origin, in float3 raydir, in float3 normal, float maxT, bool scatterTransparency, float threshold) {
    uint I = 0, R = 0; float lastMax = maxT, lastMin = 0.001f, fullLength = 0.f; float3 forigin = origin + faceforward(normal.xyz, -raydir.xyz, normal.xyz) * lastMin + raydir.xyz * lastMin, sorigin = forigin;

    // 
    XHIT processing, confirmed;
    processing.origin = float4(origin.xyz, 1.f);
    processing.direct = float4(raydir.xyz, 0.f);
    processing.gIndices = uint4(0u.xxxx);
    processing.gBarycentric = float4(0.f.xxx, maxT);
    confirmed = processing;

    // 
    bool restart = true, opaque = false;
    while((R++) < 16 && restart) { restart = false; // restart needs for transparency (after every resolve)
        float lastMax = (maxT - fullLength); float3 lastOrigin = forigin;//raydir * fullLength + sorigin; 
        traceRayEXT(RAY_FLAG_FORCE_OPAQUE|RAY_FLAG_CULL_BACK_FACING_TRIANGLES, 0xFFu, 0u, 1u, 0u, lastOrigin, 0.001f, raydir, lastMax);

        // 
        const float3 baryCoord = hit.gBarycentric.xyz;
        const bool isSkybox = dot(baryCoord.yz,1.f.xx)<=0.f; //uintBitsToFloat(datapass.z) <= 0.99f;
        const uint primitiveID = hit.gIndices.z;
        const uint geometryInstanceID = hit.gIndices.y;
        const uint globalInstanceID = hit.gIndices.x;
        const uint nodeMeshID = getMeshID(rtxInstances[globalInstanceID]);
        const float tHit = hit.gBarycentric.w;

        // 
        if (tHit < lastMax && !isSkybox) { lastOrigin = raydir*(lastMax=tHit) + forigin;
            //processing = hit;
            processing.gIndices = hit.gIndices;
            processing.gBarycentric = hit.gBarycentric;
            processing.origin.xyz = (forigin = (raydir*(processing.gBarycentric.w = (fullLength += tHit)) + sorigin));

            // Interpolate In Ray-Tracing
            XGEO geometry = interpolate(processing);
            XPOL material = materialize(processing, geometry);

            // 
            forigin += faceforward(geometry.gNormal.xyz, -raydir.xyz, geometry.gNormal.xyz) * lastMin + raydir.xyz * lastMin;

            // confirm that hit 
            if (material.diffuseColor.w > (scatterTransparency ? random(seed) : threshold)) { opaque = true; };
        };

        // 
        if (fullLength <= (maxT-1.f) && !opaque) { restart = true; };
        if (!restart) { break; };
    };

    // 
    if (fullLength <= (maxT-1.f) && opaque) { confirmed = processing; };
    return confirmed;
};

// 
#define RAY_TRACE_DEFINED
#ifdef GLSL
#define LAUNCH_ID gl_LaunchIDEXT
#else
#define LAUNCH_ID DispatchRaysIndex()
#endif
#include "./stuff.hlsli"

// 
void main() {
    const Box box = { -1.f.xxx, 1.f.xxx }; // TODO: Change Coordinate
    const float4 sphere = float4(float3(16.f,128.f,16.f), 8.f);
    const float3 lightc = 32.f*4096.f.xxx/(sphere.w*sphere.w);

    const uint2 lanQ = LAUNCH_ID.xy;//gl_LaunchIDEXT.xy;//gl_GlobalInvocationID.xy;
    launchSize = imageSize(writeImages[IW_POSITION]);

    // 
    const int2 curPixel = int2(lanQ), invPixel = int2(curPixel.x,curPixel.y);
    const int2 sizPixel = int2(launchSize);

    // WARNING! Quality may critically drop when move! 
    const bool checker = bool(((curPixel.x ^ curPixel.y) ^ (pushed.rdata.x^1u))&1u);

    {
        //
        packed = packUint2x16(curPixel), seed = uint2(packed, pushed.rdata.x);
        const float2 shift = random2(seed),   pixel = float2(invPixel)+(shift*2.f-1.f)*0.25f+0.5f;
        //const float2 shift = 0.5f.xx,       pixel = float2(invPixel)+(shift*2.f-1.f)*0.25f+0.5f;

        // 
        float3 origin = screen2world(float3((float2(pixel)/float2(sizPixel))*2.f-1.f,0.001f));
        float3 target = screen2world(float3((float2(pixel)/float2(sizPixel))*2.f-1.f,0.999f));
        float3 raydir = normalize(target - origin);
        float3 normal = float3(0.f.xxx);
        float3 geonrm = float3(0.f.xxx);

        // Replacement for rasterization
        //XHIT RPM = traceRays(    origin.xyz,           (raydir), normal, 10000.f, FAST_BW_TRANSPARENT, 0.001f);
          XHIT RPM = rasterize(    origin.xyz,           (raydir), normal, 10000.f, FAST_BW_TRANSPARENT, 0.001f);
        if (checker) { RES = RPM; };

        // TODO: Optimize Fetching and Interpolation 
        XGEO GEO = interpolate(RPM);
        XPOL MAT = materialize(RPM, GEO);

        // 
        float4 gposition = float4(RPM.origin.xyz, RPM.gBarycentric.w);

        //XHIT RPM = SUF.txcmid.z >=0.99f && SUF.diffuseColor.w <= 0.99f ? traceRays(SUF.origin.xyz, refractive(raydir), normal, 10000.f, false, 0.99f) : SUF; // Ground Deep
        imageStore(writeBuffer[nonuniformEXT(BW_GROUNDPS)], int2(lanQ), float4(RPM.origin.xyz, RPM.gBarycentric.w)); // Prefer From TOP layer (like as in Minecraft)
        imageStore(writeImages[nonuniformEXT(IW_INDIRECT)], int2(lanQ), (RPM.gBarycentric.w < 9999.f && checker) ? float4(1.f.xxx, 1.f) : float4(0.f.xxx, 0.f));

        // By Geometry Data
        const uint globalInstanceID = RPM.gIndices.y, geometryInstanceID = RPM.gIndices.x;
        const uint nodeMeshID = getMeshID(rtxInstances[globalInstanceID]);
            
        // By Geometry Data
        float3x4 matras = float3x4(float4(1.f,0.f.xxx),float4(0.f,1.f,0.f.xx),float4(0.f.xx,1.f,0.f));
        float3x4 matra4 = rtxInstances[globalInstanceID].transform;
        if (hasTransform(meshInfo[nodeMeshID])) {
#ifdef GLSL
            matras = float3x4(instances[nodeMeshID].transform[geometryInstanceID]);
#else
            matras = float3x4(tmatrices[nodeMeshID][geometryInstanceID]);
#endif
        };

        // Initial Position
        //float4 instanceRel = inverse(matras) * inverse(rtxInstances[globalInstanceID].transform) * float4(RPM.origin.xyz,1.f);
        float4 instanceRel = mul(inverse(getMT4x4(rtxInstances[globalInstanceID].transform)), mul(inverse(matras), float4(RPM.origin.xyz,1.f)));

        // Problem: NOT enough slots for writables
        // Solution: DON'T use for rasterization after 7th slot, maximize up to 12u slots... 
        //imageStore(writeImages[nonuniformEXT(IW_POSITION)], int2(lanQ), float4(RPM.origin .xyz, RPM.gBarycentric.w));
        imageStore(writeImages[nonuniformEXT(IW_POSITION)], int2(lanQ), float4(instanceRel.xyz, RPM.gBarycentric.w));
        imageStore(writeImages[nonuniformEXT(IW_GEOMETRY)], int2(lanQ), uintBitsToFloat(uint4(RPM.gIndices.xy, RPM.gIndices.xy)));
        imageStore(writeImages[nonuniformEXT(IW_INDIRECT)], int2(lanQ), float4(0.f.xxx, 1.f));

        // Will Resampled Itself (anchors)
        imageStore(writeImages[nonuniformEXT(IW_MATERIAL)], int2(lanQ), float4(MAT.txcmid  ));
        imageStore(writeImages[nonuniformEXT(IW_TRANSPAR)], int2(lanQ), float4(0.f.xxx, RPM.gBarycentric.w < 9999.f ? 0.f : 1.f));
        imageStore(writeImages[nonuniformEXT(IW_REFLECLR)], int2(lanQ), float4(0.f.xxx, 1.f)); // Py-Clone

        // 
        imageStore(writeImages[nonuniformEXT(IW_GEONORML)], int2(lanQ), float4(GEO.gNormal.xyz, 1.f));
        imageStore(writeImages[nonuniformEXT(IW_MAPNORML)], int2(lanQ), float4(MAT.mapNormal.xyz, RPM.gBarycentric.w < 9999.f ? 1.f : 0.f));


        // Make Visible Color as Anti-Aliased!
        float4 diffused = float4(MAT.diffuseColor.xyz, 1.f);
        float4 emission = float4(MAT.emissionColor.xyz, 1.f);
        diffused.xyz = max(diffused.xyz - emission.xyz, 0.f.xxx);
        if (RPM.gBarycentric.w >= 9999.f) {
            diffused.xyz = 0.f.xxx, emission.xyz = gSkyShader(raydir.xyz, origin.xyz).xyz;
            diffused.xyz += emission.xyz;
        };
        imageStore(writeImages[nonuniformEXT(IW_SMOOTHED)], int2(lanQ), float4(diffused));
    };

    //
#ifdef RAY_TRACE
    
    float4 adaptiveData = 10000.f.xxxx;
    XGEO GEO = interpolate(RES);
    XPOL MAT = materialize(RES, GEO);
    if ( checker && (MAT.diffuseColor.w > 0.001f && RES.gBarycentric.w < 9999.f) ) { // 
              float4 origin = float4(RES.origin.xyz-RES.gBarycentric.w*RES.direct.xyz, 1.f);
        const float4 bspher = float4(origin.xyz,10000.f);
        const float inIOR = 1.f, outIOR = 1.6666f;

        // 
        const uint MAX_ITERATION = 3u;
        for (uint I=0;I<MAX_ITERATION;I++) {
            if (!(MAT.diffuseColor.w > 0.001f && RES.gBarycentric.w < 9999.f)) { continue; }; // useless tracing mode
            if (  MAT.diffuseColor.w > 0.99f  && I == 2 ) { break; }; // still needs shading, except surface transparency

            // 
            float3x3 TBN = float3x3(GEO.gTangent.xyz, GEO.gBinormal.xyz, GEO.gNormal.xyz);
            float3 forigin = (origin.xyz = RES.origin.xyz), fraydir = RES.direct.xyz, fnormal = MAT.mapNormal.xyz;

            // 
            fnormal = normalize(faceforward(fnormal, fraydir.xyz, TBN[2]));
            TBN[2]  = normalize(faceforward(TBN[2] , fraydir.xyz, TBN[2]));

            // 
            float3 raydir = I == 2 ? refractive(fraydir.xyz) : (I == 0 ? randomHemisphereCosine(seed, TBN) : reflectGlossy(seed, fraydir.xyz, TBN, MAT.specularColor.y));
            float3 origin = forigin, normal = normalize(faceforward(fnormal.xyz, -raydir.xyz, TBN[2]));
            float4 gEnergy = float4(1.f.xxxx), gSignal = float4(0.f.xxx,1.f);

            // 
            if ( I == 1 ) {  }; // TODO: metallic reflection
            if ( I == 0 ) { directLight(RES, sphere, origin, fnormal, seed, gSignal, gEnergy); };

            // 
            bool hasSkybox = false;
            for (uint i=0;i<2;i++) { // fast trace
                XHIT hit = traceRays(origin, raydir, normal, 10000.f, true, 0.001f);
                XGEO result = interpolate(hit);
                XPOL material = materialize(hit, result);

                if (i == 0 && I == 2) { hasSkybox = hit.gBarycentric.w > 9999.f; }; 

                // 
                normal.xyz = material.mapNormal.xyz = normalize(faceforward(material.mapNormal.xyz, raydir.xyz, result.gNormal.xyz));

                // 
                float sdepth = raySphereIntersect(origin.xyz,raydir.xyz,sphere.xyz,sphere.w); sdepth = sdepth <= 0.f ? 10000.f : sdepth;
                float mvalue = min(hit.gBarycentric.w, 10000.f);

                // power of reflection
                float reflectionPower = mix(clamp(pow(1.0f + dot(raydir.xyz, material.mapNormal.xyz), outIOR/inIOR), 0.f, 1.f) * 0.3333f, 1.f, material.specularColor.z);
                bool couldReflection = random(seed) <= reflectionPower;

                // 
                if ( hit.gBarycentric.w >= 9999.f ) {
                    const float sdepth = raySphereIntersect(origin.xyz, raydir.xyz,bspher.xyz,bspher.w); mvalue = (sdepth <= 0.f ? 10000.f : sdepth);
                    gSignal.xyz = max(fma(gEnergy.xyz, material.emissionColor.xyz, gSignal.xyz), 0.f.xxx), gEnergy *= 0.f;
                } else 
                if ( material.diffuseColor.w > 0.001f ) {
                    if (couldReflection) {
                        gEnergy *= float4(mix(1.f.xxx, material.diffuseColor.xyz, material.specularColor.zzz), 1.f);
                    } else {
                        gSignal.xyz += gEnergy.xyz * material.emissionColor.xyz * material.emissionColor.w;
                        gEnergy *= float4(max(material.diffuseColor.xyz - clamp(material.emissionColor.xyz*material.emissionColor.w,0.f.xxx,1.f.xxx), 0.f.xxx), 1.f);
                    };
                } else { // VOID!
                    gEnergy *= float4(0.f.xxxx);
                }

                // 
                const float3x3 TBN = float3x3(result.gTangent.xyz,result.gBinormal.xyz,material.mapNormal.xyz);
                raydir.xyz = couldReflection ? 
                    reflectGlossy(seed, raydir.xyz, TBN, material.specularColor.y) : 
                    randomHemisphereCosine(seed, TBN);

                // 
                normal.xyz = normalize(faceforward(material.mapNormal.xyz, -raydir.xyz, result.gNormal.xyz));
                origin.xyz = hit.origin.xyz;

                // 
                if ( i == 0 ) { adaptiveData[I] = hit.gBarycentric.w; }; // length of ray for adaptive denoise
                if ((dot(gEnergy.xyz,1.f.xxx)/3.f) <= 0.001f || hit.gBarycentric.w >= 9999.f) { break; }; //

                // 
                directLight(hit, sphere, origin, normal, seed, gSignal, gEnergy);
            };

            // 
            { gSignal.xyz = clamp(gSignal.xyz,0.f.xxx,16.f.xxx); };
            if (I == 0) { imageStore(writeImages[nonuniformEXT(IW_INDIRECT)], int2(lanQ), float4(gSignal.xyz, 1.f)); };
            if (I == 1) { imageStore(writeImages[nonuniformEXT(IW_REFLECLR)], int2(lanQ), float4(clamp(gSignal.xyz, 0.f.xxx, 2.f.xxx), 1.f)); };
            if (I == 2) { imageStore(writeImages[nonuniformEXT(IW_TRANSPAR)], int2(lanQ), float4(clamp(gSignal.xyz, 0.f.xxx, 2.f.xxx), (hasSkybox?1.f:2.f))); }; // alpha channel reserved, zero always opaque type
        
        };
        imageStore(writeImages[nonuniformEXT(IW_ADAPTIVE)], int2(lanQ), adaptiveData); // For Adaptive Denoise
    };
#endif

    {   // 
        float4 gposition = float4(RES.origin.xyz, RES.gBarycentric.w);

        // Used By Reprojection (comparsion)
        imageStore(writeBuffer[nonuniformEXT(BW_INDIRECT)], int2(lanQ), imageLoad(writeImages[nonuniformEXT(IW_INDIRECT)], int2(lanQ)));
        imageStore(writeBuffer[nonuniformEXT(BW_POSITION)], int2(lanQ), gposition); // Stay The Same...
        imageStore(writeBuffer[nonuniformEXT(BW_GEONORML)], int2(lanQ), imageLoad(writeImages[nonuniformEXT(IW_GEONORML)], int2(lanQ)));
        imageStore(writeBuffer[nonuniformEXT(BW_SMOOTHED)], int2(lanQ), imageLoad(writeImages[nonuniformEXT(IW_SMOOTHED)], int2(lanQ)));
        imageStore(writeBuffer[nonuniformEXT(BW_MATERIAL)], int2(lanQ), imageLoad(writeImages[nonuniformEXT(IW_MATERIAL)], int2(lanQ)));
        imageStore(writeBuffer[nonuniformEXT(BW_REFLECLR)], int2(lanQ), imageLoad(writeImages[nonuniformEXT(IW_REFLECLR)], int2(lanQ)));
        imageStore(writeBuffer[nonuniformEXT(BW_TRANSPAR)], int2(lanQ), imageLoad(writeImages[nonuniformEXT(IW_TRANSPAR)], int2(lanQ)));
        imageStore(writeBuffer[nonuniformEXT(BW_ADAPTIVE)], int2(lanQ), imageLoad(writeImages[nonuniformEXT(IW_ADAPTIVE)], int2(lanQ)));
    };
};

