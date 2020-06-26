#version 460 core // #
#extension GL_GOOGLE_include_directive : require
//#extension GL_EXT_ray_tracing          : require
//#extension GL_ARB_post_depth_coverage  : require
#define ENABLE_AS
#include "./driver.glsl"
#include "./global.glsl"

// 
#define RAY_TRACE
#define FAST_BW_TRANSPARENT false
//#define FAST_BW_TRANSPARENT // Can be denoised, but with WRONG results!
//#define TOP_LAYERED // Has reflection problems

// TODO: X-Based Optimization
const uint workX = 64u, workY = 12u; // Optimal Work Size for RTX 2070
layout ( local_size_x = workX, local_size_y = workY ) in; 

// 
shared XHIT hits[workX*workY];

// Needs 1000$ for fix BROKEN ray query...
const uint MAX_ITERATION = 64u;

// Ray Query Broken In Latest Driver... 
XHIT traceRays(in float3 origin, in float3 raydir, in float3 normal, float maxT, bool scatterTransparency, float threshold) {
    uint32_t I = 0, R = 0; float lastMax = maxT, lastMin = 0.001f, fullLength = 0.f; float3 forigin = origin + faceforward(normal.xyz, -raydir.xyz, normal.xyz) * lastMin + raydir.xyz * lastMin, sorigin = forigin;

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
        rayQueryEXT rayQuery; float lastMax = (maxT - fullLength); float3 lastOrigin = forigin;//raydir * fullLength + sorigin; 
        rayQueryInitializeEXT(rayQuery, Scene, gl_RayFlagsOpaqueEXT|gl_RayFlagsCullNoOpaqueEXT|gl_RayFlagsCullBackFacingTrianglesEXT,
            0xFF, lastOrigin, 0.001f, raydir, lastMax);

        // BROKEN `rayQueryProceedEXT`
        bool proceed = false;
        I = 0; while((I++) < MAX_ITERATION && (proceed = rayQueryProceedEXT(rayQuery))) { // 
            uint nodeMeshID = rayQueryGetIntersectionInstanceCustomIndexEXT(rayQuery, false); // Mesh ID from Node Mesh List (because indexing)
            uint geometryInstanceID = rayQueryGetIntersectionGeometryIndexEXT(rayQuery, false); // TODO: Using In Ray Tracing (and Query) shaders!
            uint globalInstanceID = rayQueryGetIntersectionInstanceIdEXT(rayQuery, false);
            float2 baryCoord = rayQueryGetIntersectionBarycentricsEXT(rayQuery, false);
            uint primitiveID = rayQueryGetIntersectionPrimitiveIndexEXT(rayQuery, false); 
            float3 origin = rayQueryGetIntersectionObjectRayOriginEXT(rayQuery, false);
            float tHit = rayQueryGetIntersectionTEXT(rayQuery, false);

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
                    opaque = true; rayQueryConfirmIntersectionEXT(rayQuery); // override processing hit
                };
            };
        };

        // 
        processing = confirmed; lastMax = (maxT - fullLength); lastOrigin = raydir*maxT + sorigin; opaque = false;
        if (!proceed) { // Attemp to fix Broken Ray Query
            if (rayQueryGetIntersectionTypeEXT(rayQuery, true) != gl_RayQueryCommittedIntersectionNoneEXT) {
                uint nodeMeshID = rayQueryGetIntersectionInstanceCustomIndexEXT(rayQuery, true); // Mesh ID from Node Mesh List (because indexing)
                uint geometryInstanceID = rayQueryGetIntersectionGeometryIndexEXT(rayQuery, true); // TODO: Using In Ray Tracing (and Query) shaders!
                uint globalInstanceID = rayQueryGetIntersectionInstanceIdEXT(rayQuery, true);
                float2 baryCoord = rayQueryGetIntersectionBarycentricsEXT(rayQuery, true);
                uint primitiveID = rayQueryGetIntersectionPrimitiveIndexEXT(rayQuery, true); 
                float3 origin = rayQueryGetIntersectionObjectRayOriginEXT(rayQuery, true);
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
                    if (material.diffuseColor.w > (scatterTransparency ? random(seed) : threshold)) { opaque = true; };
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

#define LAUNCH_ID gl_GlobalInvocationID.xy
#include "./stuff.glsl"
#include "./raytrace-adv-new.glsl"
