#version 460 core // #
#extension GL_GOOGLE_include_directive : require
#extension GL_EXT_ray_tracing          : require

#define ENABLE_AS
#include "./driver.glsl"
#include "./global.glsl"

// 
#define RAY_TRACE
#define FAST_BW_TRANSPARENT false

//#define FAST_BW_TRANSPARENT // Can be denoised, but with WRONG results!
//#define TOP_LAYERED // Has reflection problems

// 
//layout (local_size_x = 32u, local_size_y = 24u) in;
layout ( location = 0u ) rayPayloadEXT CHIT hit;

// For Cache
XHIT RES;

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
        float lastMax = (maxT - fullLength); float3 lastOrigin = forigin;//raydir * fullLength + sorigin; 
        traceRayEXT (Scene, gl_RayFlagsOpaqueEXT|gl_RayFlagsCullNoOpaqueEXT|gl_RayFlagsCullBackFacingTrianglesEXT,
            0xFFu, 0u, 1u, 0u, lastOrigin, 0.001f, raydir, lastMax, 0);

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

#define LAUNCH_ID gl_LaunchIDEXT.xy
#include "./stuff.glsl"
#include "./raytrace-new.glsl"
