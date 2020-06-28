#define ENABLE_AS
#include "./driver.hlsli"
#include "./global.hlsli"

// 
#define RAY_TRACE
#define FAST_BW_TRANSPARENT false
//#define FAST_BW_TRANSPARENT // Can be denoised, but with WRONG results!
//#define TOP_LAYERED // Has reflection problems

// TODO: X-Based Optimization
const uint workX = 64u, workY = 12u; // Optimal Work Size for RTX 2070
shared XHIT hits[workX*workY];

// Needs 1000$ for fix BROKEN ray query...
const uint MAX_ITERATION = 64u;

/*
struct RayDesc
{
    float3 Origin;
    float TMin;
    float3 Direction;
    float TMax;
};
*/

// Ray Query Broken In Latest Driver... 
XHIT traceRays(in float3 origin, in float3 raydir, in float3 normal, float maxT, bool scatterTransparency, float threshold) {
    uint I = 0, R = 0; float lastMax = maxT, lastMin = 0.001f, fullLength = 0.f; float3 forigin = origin + faceforward(normal.xyz, -raydir.xyz, normal.xyz) * lastMin + raydir.xyz * lastMin, sorigin = forigin;

    // 
    XHIT processing, confirmed;
    processing.origin = float4(origin.xyz, 1.f);
    processing.direct = float4(raydir.xyz, 0.f);
    processing.gIndices = uint4(0u);
    processing.gBarycentric = float4(0.f.xxx, maxT);
    confirmed = processing;

    // 
    bool restart = true, opaque = false;
    while((R++) < 16 && restart) { restart = false; // restart needs for transparency (after every resolve)
        float lastMax = (maxT - fullLength); 
        float3 lastOrigin = forigin;//raydir * fullLength + sorigin; 

        // 
        RayDesc desc;
        desc.Origin = lastOrigin.xyz;
        desc.TMin = 0.001f;
        desc.Direction = raydir;
        desc.TMax = lastMax;

        // 
        RayQuery<RAY_FLAG_FORCE_OPAQUE|RAY_FLAG_CULL_BACK_FACING_TRIANGLES> rayQuery;
        rayQuery.TraceRayInline(Scene, RAY_FLAG_FORCE_OPAQUE|RAY_FLAG_CULL_BACK_FACING_TRIANGLES, 0xFF, desc);

        // BROKEN `rayQueryProceedEXT`
        bool proceed = false;
        I = 0; while((I++) < MAX_ITERATION && (proceed = rayQuery.Proceed())) { // 
            uint nodeMeshID = rayQuery.CandidateInstanceID(); // Mesh ID from Node Mesh List (because indexing)
            uint geometryInstanceID = rayQuery.CandidateGeometryIndex(); // TODO: Using In Ray Tracing (and Query) shaders!
            uint globalInstanceID = rayQuery.CandidateInstanceIndex();
            float2 baryCoord = rayQuery.CandidateTriangleBarycentrics();
            uint primitiveID = rayQuery.CandidatePrimitiveIndex();
            float tHit = rayQuery.CandidateTriangleRayT();

            // 
            if (tHit < lastMax) { lastOrigin = raydir*(lastMax = tHit) + forigin;
                processing.gIndices = uint4(globalInstanceID, geometryInstanceID, primitiveID, 0u);
                processing.gBarycentric.xyz = float3(1.f-baryCoord.x-baryCoord.y,baryCoord);
                processing.origin = float4(raydir*(processing.gBarycentric.w = (fullLength + tHit)) + sorigin, 1.f);

                // Interpolate In Ray-Tracing
                XGEO geometry = interpolate(processing);
                XPOL material = materialize(processing, geometry);

                // confirm that hit 
                if (material.diffuseColor.w > (scatterTransparency ? random(seed) : threshold)) { // Only When Opaque!
                    opaque = true; rayQuery.CommitOpaqueTriangleHit(); // override processing hit
                };
            };
        };

        // 
        processing = confirmed; lastMax = (maxT - fullLength); lastOrigin = raydir*maxT + sorigin; opaque = false;
        if (!proceed) { // Attemp to fix Broken Ray Query
            if (rayQuery.CommittedStatus() != COMMITTED_NOTHING) {
                uint nodeMeshID = rayQuery.CommittedInstanceID(); // Mesh ID from Node Mesh List (because indexing)
                uint geometryInstanceID = rayQuery.CommittedGeometryIndex(); // TODO: Using In Ray Tracing (and Query) shaders!
                uint globalInstanceID = rayQuery.CommittedInstanceIndex();
                float2 baryCoord = rayQuery.CommittedTriangleBarycentrics();
                uint primitiveID = rayQuery.CommittedPrimitiveIndex();
                float tHit = rayQuery.CommittedTriangleRayT();

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
                    if (material.diffuseColor.w > (scatterTransparency ? random(seed) : threshold)) { opaque = true; };
                };
            } else { fullLength = maxT; };
        };
        rayQuery.Abort();

        // 
        if (fullLength <= (maxT-1.f) && !opaque) { restart = true; };
        if (!restart) { break; };
    };

    // 
    if (fullLength <= (maxT-1.f)) { confirmed = processing; };
    return confirmed;
};

// 
#define LAUNCH_ID gl_GlobalInvocationID.xy
#include "./stuff.hlsli"
#define RES hits[lIdx]

// 
const uint3 WorkGroupSize = uint3(workX, workY, 1);

// 14.06.2020
// Fully Refresh Ray Cast Shaders
[numthreads(WorkGroupSize.x, WorkGroupSize.y, WorkGroupSize.z)]
void main(uint LocalInvocationIndex : SV_GroupIndex, uint3 GlobalInvocationID : SV_DispatchThreadID, uint3 LocalInvocationID : SV_GroupThreadID, uint3 WorkGroupID : SV_GroupID) {
    const Box box = { -1.f.xxx, 1.f.xxx }; // TODO: Change Coordinate
    const float4 sphere = float4(float3(16.f,128.f,16.f), 8.f);
    
    // 
    const uint2 locQs = uint2(LocalInvocationID.xy);
    const uint2 locQ = uint2(locQs.x, (locQs.y<<1u) | ((locQs.x+pushed.rdata.x)&1u));
    const uint2 lanQ = uint2(WorkGroupID.xy*WorkGroupSize.xy*uint2(1u,2u) + locQ).xy;
    //const uint lIdx = locQ.y * gl_WorkGroupSize.x + locQ.x;
    const uint lIdx = LocalInvocationIndex;

    // 
    writeImages[IW_POSITION].GetDimensions(launchSize.x, launchSize.y);
    GroupMemoryBarrierWithGroupSync();

    // 
    for (uint Q = 0u; Q < 2u; Q++) {
        const uint2 locQ = uint2(locQs.x, Q*WorkGroupSize.y + locQs.y);
        const uint2 lanQ = uint2(WorkGroupID.xy*uint2(WorkGroupSize.xy*uint2(1u,2u)) + locQ).xy;
        //const uint lIdx = locQ.y * gl_WorkGroupSize.x + locQ.x;
        const uint lIdx = (locQ.y >> 1u) * WorkGroupSize.x + locQ.x;
        
        // 
        const int2 curPixel = int2(lanQ), invPixel = int2(curPixel.x,curPixel.y);
        const int2 sizPixel = int2(launchSize);

        // WARNING! Quality may critically drop when move! 
        const bool checker = bool(((curPixel.x ^ curPixel.y) ^ (pushed.rdata.x^1u))&1u);

        //
        packed = packUint2x16(curPixel)   ,    seed = uint2(packed, pushed.rdata.x);
        const float2 shift = random2(seed),   pixel = float2(invPixel)+(shift*2.f-1.f)*0.25f+0.5f;
        //const float2 shift = 0.5f.xx,       pixel = float2(invPixel)+(shift*2.f-1.f)*0.25f+0.5f;

        // 
        float3 origin = screen2world(float3((float2(pixel)/float2(sizPixel))*2.f-1.f,0.001f));
        float3 target = screen2world(float3((float2(pixel)/float2(sizPixel))*2.f-1.f,0.999f));
        float3 raydir = normalize(target - origin);
        float3 normal = float3(0.f);
        float3 geonrm = float3(0.f);

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
        writeBuffer[BW_GROUNDPS][lanQ] = float4(RPM.origin.xyz, RPM.gBarycentric.w); // Prefer From TOP layer (like as in Minecraft)
        writeImages[IW_INDIRECT][lanQ] = (RPM.gBarycentric.w < 9999.f && checker) ? float4(1.f.xxx, 1.f) : float4(0.f.xxx, 0.f);

        // By Geometry Data
        const uint globalInstanceID = RPM.gIndices.y, geometryInstanceID = RPM.gIndices.x;
        const uint nodeMeshID = getMeshID(rtxInstances[globalInstanceID]);
        float3x4 matras = float3x4(transforms[nodeMeshID][geometryInstanceID]);
        if (!hasTransform(meshInfo[nodeMeshID])) {
            matras = float3x4(float4(1.f,0.f.xxx),float4(0.f,1.f,0.f.xx),float4(0.f.xx,1.f,0.f));
        };

        // Initial Position
        float4 instanceRel = mul(mul(inverse(float4x4(rtxInstances[globalInstanceID].transform, float4(0.f.xxx,1.f))), inverse(float4x4(matras, float4(0.f.xxx,1.f)))), float4(RPM.origin.xyz,1.f));

        // Problem: NOT enough slots for writables
        // Solution: DON'T use for rasterization after 7th slot, maximize up to 12u slots... 
        //imageStore(writeImages[nonuniformEXT(IW_POSITION)], int2(lanQ), float4(RPM.origin .xyz, RPM.gBarycentric.w));
        writeImages[IW_POSITION][lanQ] = float4(instanceRel.xyz, RPM.gBarycentric.w);
        writeImages[IW_GEOMETRY][lanQ] = asfloat(uint4(RPM.gIndices.xy, RPM.gIndices.xy));
        writeImages[IW_INDIRECT][lanQ] = float4(0.f.xxx, 1.f);

        // Will Resampled Itself (anchors)
        writeImages[IW_MATERIAL][lanQ] = float4(MAT.txcmid);
        writeImages[IW_TRANSPAR][lanQ] = float4(0.f.xxx, RPM.gBarycentric.w < 9999.f ? 0.f : 1.f);
        writeImages[IW_REFLECLR][lanQ] = float4(0.f.xxx, 1.f); // Py-Clone

        // 
        writeImages[IW_GEONORML][lanQ] = float4(GEO.gNormal.xyz, 1.f);
        writeImages[IW_MAPNORML][lanQ] = float4(MAT.mapNormal.xyz, RPM.gBarycentric.w < 9999.f ? 1.f : 0.f);


        // Make Visible Color as Anti-Aliased!
        float4 diffused = float4(MAT.diffuseColor.xyz, 1.f);
        float4 emission = float4(MAT.emissionColor.xyz, 1.f);
        diffused.xyz = max(diffused.xyz - emission.xyz, 0.f.xxx);
        if (RPM.gBarycentric.w >= 9999.f) {
            diffused.xyz = 0.f.xxx, emission.xyz = gSkyShader(raydir.xyz, origin.xyz).xyz;
            diffused.xyz += emission.xyz;
        };
        writeImages[IW_SMOOTHED][lanQ] = float4(diffused);
    }

    GroupMemoryBarrierWithGroupSync();

    // BROKEN
#ifdef RAY_TRACE
    XGEO GEO = interpolate(RES);
    XPOL MAT = materialize(RES, GEO);
    float4 adaptiveData = 10000.f.xxxx;
    if ( (MAT.diffuseColor.w > 0.001f && RES.gBarycentric.w < 9999.f) ) { // 
        
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
                float reflectionPower = lerp(clamp(pow(1.0f + dot(raydir.xyz, material.mapNormal.xyz), outIOR/inIOR), 0.f, 1.f) * 0.3333f, 1.f, material.specularColor.z);
                bool couldReflection = random(seed) <= reflectionPower;

                // 
                if ( hit.gBarycentric.w >= 9999.f ) {
                    const float sdepth = raySphereIntersect(origin.xyz, raydir.xyz,bspher.xyz,bspher.w); mvalue = (sdepth <= 0.f ? 10000.f : sdepth);
                    gSignal.xyz = max(fma(gEnergy.xyz, material.emissionColor.xyz, gSignal.xyz), 0.f.xxx), gEnergy *= 0.f;
                } else 
                if ( material.diffuseColor.w > 0.001f ) {
                    if (couldReflection) {
                        gEnergy *= float4(lerp(1.f.xxx, material.diffuseColor.xyz, material.specularColor.zzz), 1.f);
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
            if (I == 0) { writeImages[IW_INDIRECT][lanQ] = float4(gSignal.xyz, 1.f); };
            if (I == 1) { writeImages[IW_REFLECLR][lanQ] = float4(clamp(gSignal.xyz, 0.f.xxx, 2.f.xxx), 1.f); };
            if (I == 2) { writeImages[IW_TRANSPAR][lanQ] = float4(clamp(gSignal.xyz, 0.f.xxx, 2.f.xxx), (hasSkybox?1.f:2.f)); }; // alpha channel reserved, zero always opaque type
        };
        writeImages[IW_ADAPTIVE][lanQ] = adaptiveData; // For Adaptive Denoise
    };
#endif

    GroupMemoryBarrierWithGroupSync();

    // 
    for (uint Q = 0u; Q < 2u; Q++) {
        const uint2 locQ = uint2(locQs.x, Q*WorkGroupSize.y + locQs.y);
        const uint2 lanQ = uint2(WorkGroupID.xy*uint2(WorkGroupSize.xy*uint2(1u,2u)) + locQ).xy;
        //const uint lIdx = locQ.y * gl_WorkGroupSize.x + locQ.x;
        const uint lIdx = (locQ.y >> 1u) * WorkGroupSize.x + locQ.x;

        // 
        float4 gposition = float4(RES.origin.xyz, RES.gBarycentric.w);
        writeBuffer[BW_INDIRECT][lanQ] = writeImages[IW_INDIRECT][lanQ];
        writeBuffer[BW_POSITION][lanQ] = gposition; // Stay The Same...
        writeBuffer[BW_GEONORML][lanQ] = writeImages[IW_GEONORML][lanQ];
        writeBuffer[BW_SMOOTHED][lanQ] = writeImages[IW_SMOOTHED][lanQ];
        writeBuffer[BW_MATERIAL][lanQ] = writeImages[IW_MATERIAL][lanQ];
        writeBuffer[BW_REFLECLR][lanQ] = writeImages[IW_REFLECLR][lanQ];
        writeBuffer[BW_TRANSPAR][lanQ] = writeImages[IW_TRANSPAR][lanQ];
        writeBuffer[BW_ADAPTIVE][lanQ] = writeImages[IW_ADAPTIVE][lanQ];
    }
};
