#ifdef GLSL
#extension GL_EXT_ray_tracing          : require
#extension GL_EXT_ray_query            : require
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

// TODO: X-Based Optimization
//const uint workX = 64u, workY = 12u; // Optimal Work Size for RTX 2070
#define workX 32
#define workY 24

// 
SHARED XHIT hits[workX*workY];

// Needs 1000$ for fix BROKEN ray query...
STATIC const uint MAX_ITERATION = 16u;


#ifndef GLSL
#define rayQueryGetIntersectionInstanceCustomIndexEXT(q, B) (B?q.CommittedInstanceID():q.CandidateInstanceID())
#define rayQueryGetIntersectionGeometryIndexEXT(q, B) (B?q.CommittedGeometryIndex():q.CandidateGeometryIndex())
#define rayQueryGetIntersectionInstanceIdEXT(q, B) (B?q.CommittedInstanceIndex():q.CandidateInstanceIndex())
#define rayQueryGetIntersectionBarycentricsEXT(q, B) (B?q.CommittedTriangleBarycentrics():q.CandidateTriangleBarycentrics())
#define rayQueryGetIntersectionPrimitiveIndexEXT(q, B) (B?q.CommittedPrimitiveIndex():q.CandidatePrimitiveIndex())
#define rayQueryGetIntersectionTEXT(q, B) (B?q.CommittedRayT():q.CandidateTriangleRayT())
#define rayQueryProceedEXT(q) q.Proceed()
#define rayQueryGetIntersectionTypeEXT(q, B) (B?q.CommittedStatus():COMMITTED_NOTHING)
#define rayQueryConfirmIntersectionEXT(q) q.CommitNonOpaqueTriangleHit()
#define rayQueryTerminateEXT(q) q.Abort()

// 
void rayQueryInitializeEXT(in RayQuery<RAY_FLAG_CULL_BACK_FACING_TRIANGLES> rayQuery, in uint flags, in uint mask, in float3 origin, in float minT, in float3 direct, in float maxT) {
    RayDesc desc;
    desc.Origin = origin;
    desc.TMin = minT;
    desc.Direction = direct;
    desc.TMax = maxT;

    // 
    rayQuery.TraceRayInline(Scene, flags, mask, desc);
};
#else
#define RAY_FLAG_FORCE_OPAQUE gl_RayFlagsOpaqueEXT
#define RAY_FLAG_CULL_BACK_FACING_TRIANGLES gl_RayFlagsCullBackFacingTrianglesEXT
#define COMMITTED_NOTHING gl_RayQueryCommittedIntersectionNoneEXT

void rayQueryInitializeEXT(in rayQueryEXT rayQuery, in uint flags, in lowp uint mask, in float3 origin, in float minT, in float3 direct, in float maxT) {
    rayQueryInitializeEXT(rayQuery, Scene, flags, mask, origin, minT, direct, maxT);
};
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
    while((R++) < 4u && restart) { restart = false; // restart needs for transparency (after every resolve)
        float lastMax = (maxT - fullLength); float3 lastOrigin = forigin;//raydir * fullLength + sorigin;

#ifdef GLSL
        rayQueryEXT rayQuery;
#else
        RayQuery<RAY_FLAG_CULL_BACK_FACING_TRIANGLES> rayQuery;
#endif

        rayQueryInitializeEXT(rayQuery, RAY_FLAG_CULL_BACK_FACING_TRIANGLES, 0xFF, lastOrigin, lastMin, raydir, lastMax);

        // BROKEN `rayQueryProceedEXT`
        bool proceed = false;
        I = 0; while((I++) < MAX_ITERATION && (proceed = rayQueryProceedEXT(rayQuery))) { // 
            uint nodeMeshID = rayQueryGetIntersectionInstanceCustomIndexEXT(rayQuery, false); // Mesh ID from Node Mesh List (because indexing)
            uint geometryInstanceID = rayQueryGetIntersectionGeometryIndexEXT(rayQuery, false); // TODO: Using In Ray Tracing (and Query) shaders!
            uint globalInstanceID = rayQueryGetIntersectionInstanceIdEXT(rayQuery, false);
            float2 baryCoord = rayQueryGetIntersectionBarycentricsEXT(rayQuery, false);
            uint primitiveID = rayQueryGetIntersectionPrimitiveIndexEXT(rayQuery, false); 
            //float3 origin = rayQueryGetIntersectionObjectRayOriginEXT(rayQuery, false);
            float tHit = rayQueryGetIntersectionTEXT(rayQuery, false);

            // 
            {   //lastOrigin = raydir*(lastMax = tHit) + forigin;
                XHIT hit = processing;
                hit.gIndices = uint4(globalInstanceID, geometryInstanceID, primitiveID, 0u);
                hit.gBarycentric.xyz = float3(1.f-baryCoord.x-baryCoord.y,baryCoord);
                hit.origin = float4(raydir*(hit.gBarycentric.w = (fullLength + tHit)) + sorigin, 1.f);

                // Interpolate In Ray-Tracing
                XGEO geometry = interpolate(hit);
                XPOL material = materialize(hit, geometry);

                // confirm that hit 
                if (material.diffuseColor.w > (scatterTransparency ? random(seed) : threshold)) { // Only When Opaque!
                    rayQueryConfirmIntersectionEXT(rayQuery); // override hit hit
                };
            };
        };

        // 
        processing = confirmed; lastMax = (maxT - fullLength); lastOrigin = raydir*maxT + sorigin; opaque = false;
        if (!proceed) { // Attemp to fix Broken Ray Query
            if (rayQueryGetIntersectionTypeEXT(rayQuery, true) != COMMITTED_NOTHING) 
            {
                uint nodeMeshID = rayQueryGetIntersectionInstanceCustomIndexEXT(rayQuery, true); // Mesh ID from Node Mesh List (because indexing)
                uint geometryInstanceID = rayQueryGetIntersectionGeometryIndexEXT(rayQuery, true); // TODO: Using In Ray Tracing (and Query) shaders!
                uint globalInstanceID = rayQueryGetIntersectionInstanceIdEXT(rayQuery, true);
                float2 baryCoord = rayQueryGetIntersectionBarycentricsEXT(rayQuery, true);
                uint primitiveID = rayQueryGetIntersectionPrimitiveIndexEXT(rayQuery, true); 
                //float3 origin = rayQueryGetIntersectionObjectRayOriginEXT(rayQuery, true);
                float tHit = rayQueryGetIntersectionTEXT(rayQuery, true);

                // 
                if (tHit < lastMax) { lastOrigin = raydir*(lastMax=tHit) + forigin;
                    processing.gIndices = uint4(globalInstanceID, geometryInstanceID, primitiveID, 0u);
                    processing.gBarycentric.xyz = float3(1.f-baryCoord.x-baryCoord.y,baryCoord);
                    processing.origin.xyz = (forigin = (raydir*(processing.gBarycentric.w = (fullLength += tHit)) + sorigin));

                    // Interpolate In Ray-Tracing
                    XGEO geometry = interpolate(processing);
                    XPOL material = materialize(processing, geometry);

                    // 
                    forigin += faceforward(geometry.gNormal.xyz, -raydir.xyz, geometry.gNormal.xyz) * lastMin + raydir.xyz * lastMin;

                    // confirm that hit 
                    //if (material.diffuseColor.w > (scatterTransparency ? random(seed) : threshold)) { opaque = true; };
                    if (processing.gBarycentric.w > 9999.f || material.diffuseColor.w > 0.001f) { opaque = true; };
                };
            } else { fullLength = maxT; };
        };
        rayQueryTerminateEXT(rayQuery);

        // 
        if (fullLength <= (maxT-1.f) && !opaque) { restart = true; };
        if (!restart) { break; };
    };

    // 
    if (fullLength <= (maxT-1.f)) { confirmed = processing; };
    return confirmed;
};

// 
#define LAUNCH_ID GlobalInvocationID.xy
#define RAY_TRACE_DEFINED
#include "./stuff.hlsli"
#define RES hits[LocalInvocationIndex]

// 14.06.2020
// Fully Refresh Ray Cast Shaders
#ifdef GLSL
layout ( local_size_x = workX, local_size_y = workY ) in; 
void main()
#else
[numthreads(32, 24, 1)]
void main(uint LocalInvocationIndex : SV_GroupIndex, uint3 GlobalInvocationID : SV_DispatchThreadID, uint3 LocalInvocationID : SV_GroupThreadID, uint3 WorkGroupID : SV_GroupID) // TODO: explicit sampling 
#endif
{
#ifdef GLSL
    const uint3 GlobalInvocationID = gl_GlobalInvocationID;
    const uint3 LocalInvocationID = gl_LocalInvocationID;
    const uint LocalInvocationIndex = gl_LocalInvocationIndex;
#endif

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
        packed = packUint2x16(curPixel),       seed = uint2(packed, pushed.rdata.x);
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
          XHIT RPM = rasterize(    origin.xyz,           (raydir), normal, 10000.f, FAST_BW_TRANSPARENT, 0.001f); RES = RPM;

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

#ifdef GLSL
    groupMemoryBarrier(); barrier();
#else
    GroupMemoryBarrierWithGroupSync();
#endif

    //
#ifdef RAY_TRACE
    
    float4 adaptiveData = 10000.f.xxxx;
    XGEO GEO = interpolate(RES);
    XPOL MAT = materialize(RES, GEO);
    if ( (MAT.diffuseColor.w > 0.001f && RES.gBarycentric.w < 9999.f) ) { // 
              float4 origin = float4(RES.origin.xyz-RES.gBarycentric.w*RES.direct.xyz, 1.f);
        const float4 bspher = float4(origin.xyz,10000.f);
        const float inIOR = 1.f, outIOR = 1.6666f;

        // 
        const uint MAX_ITERATION = 3u;
        //for (uint I=0;I<MAX_ITERATION;I++) {
        for (uint m=0;m<2;m++) {
            const uint I = m == 1u ? 2u : (checker ? 0u : 1u);

            // 
            if (!(MAT.diffuseColor.w > 0.001f && RES.gBarycentric.w < 9999.f)) { continue; }; // useless tracing mode

            // still needs shading, except surface transparency
#ifdef HIGH_QUALITY_TRANSPARENCY
            if ( MAT.diffuseColor.w > 0.99f  && I == 2 ) { break; }; // High Quality
#else
            if ( (MAT.diffuseColor.w > 0.99f || !checker) && I == 2 ) { break; }; // Low Quality
#endif

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
            if (I == 2) { imageStore(writeImages[nonuniformEXT(IW_TRANSPAR)], int2(lanQ), float4(clamp(gSignal.xyz, 0.f.xxx, 2.f.xxx), (hasSkybox?1.f:2.f))); };
        };
        imageStore(writeImages[nonuniformEXT(IW_ADAPTIVE)], int2(lanQ), adaptiveData); // For Adaptive Denoise
    };
#endif

#ifdef GLSL
    groupMemoryBarrier(); barrier();
#else
    GroupMemoryBarrierWithGroupSync();
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
