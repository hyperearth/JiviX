#ifndef GLOBAL_HLSL
#define GLOBAL_HLSL

// 
#include "./index.hlsli"
#include "./matrix.hlsli"

// 
struct RCData {
    uint4 udata;
    float4 fdata;
};

// BUT DEFAULT OVERRIDEN!
int2 launchSize = int2(1600, 1200);

// 
struct XPOL {
     float4  diffuseColor;
     float4 emissionColor;
     float4  normalsColor;
     float4 specularColor;

     float4 mapNormal; float4 txcmid;
};

struct XGEO {
    float4 gTangent; float4 gBinormal; float4 gNormal; float4 gTexcoord;
};

struct XHIT {
     float4 origin; float4 direct;
     
     float4 gBarycentric;
    uint4 gIndices;
};

struct CHIT {
    float4 gBarycentric;
    uint4 gIndices;
};

// 
uint packed = 0u;
uint2 seed = uint2(0u.xx);

// RESERVED FOR OTHER OPERATIONS
float3 refractive(in float3 dir) {
    return dir;
};

// 
float4x4 inverse(in float3x4 imat) {
    //return inverse(transpose(float4x4(imat[0],imat[1],imat[2],float4(0.f,0.f,0.f,1.f))));
    return transpose(inverse(float4x4(imat[0],imat[1],imat[2],float4(0.f,0.f,0.f,1.f))));
};



// 
XGEO interpolate(in XHIT hit) { // By Geometry Data
    const uint geometryInstanceID = hit.gIndices.y;
    const uint globalInstanceID = hit.gIndices.x;
    const uint primitiveID = hit.gIndices.z;
    const uint nodeMeshID = getMeshID(rtxInstances[globalInstanceID]);
    const float3 baryCoord = hit.gBarycentric.xyz;

    // 
    float3x4 matras = float3x4(transforms[nodeMeshID][geometryInstanceID]);
    if (!hasTransform(meshInfo[nodeMeshID])) {
        matras = float3x4(float4(1.f,0.f.xxx),float4(0.f,1.f,0.f.xx),float4(0.f.xx,1.f,0.f));
    };

    // By Instance Data
    const float3x4 matra4 = rtxInstances[globalInstanceID].transform;

    // Native Normal Transform
    const float3x3 normalTransform = inverse(transpose(regen3(matras)));
    const float3x3 normInTransform = inverse(transpose(regen3(matra4)));

    // 
    XGEO geometry;

    // 
    uint3 idx3 = uint3(primitiveID*3u+0u,primitiveID*3u+1u,primitiveID*3u+2u);
    geometry.gTexcoord  = float4(triangulate(idx3, 1u, nodeMeshID,baryCoord).xyz,0.f);
    geometry.gNormal    = float4(triangulate(idx3, 2u, nodeMeshID,baryCoord).xyz,0.f);
    geometry.gTangent   = float4(triangulate(idx3, 3u, nodeMeshID,baryCoord).xyz,0.f);
    geometry.gBinormal  = float4(triangulate(idx3, 4u, nodeMeshID,baryCoord).xyz,0.f);

    // 
    geometry.gNormal.xyz *= normalTransform * normInTransform;
    geometry.gTangent.xyz *= normalTransform * normInTransform;
    geometry.gBinormal.xyz *= normalTransform * normInTransform;

    //
    geometry.gNormal.xyz = normalize(geometry.gNormal.xyz);
    geometry.gTangent.xyz = normalize(geometry.gTangent.xyz);
    geometry.gBinormal.xyz = normalize(geometry.gBinormal.xyz);

    // 
    return geometry;
};

// 
XPOL materialize(in XHIT hit, inout XGEO geo) { // 
#define MatID materialID[nodeMeshID][geometryInstanceID]
    XPOL material;
    material. diffuseColor = float4(0.f.xxx, 1.f.x);
    material.emissionColor = gSkyShader(hit.direct.xyz, hit.origin.xyz);
    material. normalsColor = float4(0.5f,0.5f,1.f,1.f);
    material.specularColor = float4(0.f.xxx,0.f.x); // TODO: Correct Specular Initial
    material.mapNormal = geo.gNormal;
    material.txcmid = float4(asfloat(packUnorm2x16(0.f.xx)), 0.f, 0.f, 0.f); // 

    // 
    const uint geometryInstanceID = hit.gIndices.y;
    const uint globalInstanceID = hit.gIndices.x;
    const uint primitiveID = hit.gIndices.z;
    const uint nodeMeshID = getMeshID(rtxInstances[globalInstanceID]);
    const MaterialUnit unit = materials[0u][MatID]; // NEW! 20.04.2020
    const float2 gTexcoord = geo.gTexcoord.xy;
    //const float3 gNormal = geo.gNormal.xyz;

    // 
    if (hit.gBarycentric.w < 9999.f) {
        material. diffuseColor = toLinear(unit. diffuseTexture >= 0 ? textures[unit. diffuseTexture].Sample(samplers[2u],gTexcoord.xy) : unit.diffuse);
        material.emissionColor = toLinear(unit.emissionTexture >= 0 ? textures[unit.emissionTexture].Sample(samplers[2u],gTexcoord.xy) : unit.emission);
        material. normalsColor = unit. normalsTexture >= 0 ? textures[unit. normalsTexture].Sample(samplers[2u],gTexcoord.xy) : unit.normals;
        material.specularColor = unit.specularTexture >= 0 ? textures[unit.specularTexture].Sample(samplers[2u],gTexcoord.xy) : unit.specular;

        // Mapping
        material.mapNormal = float4(normalize(mul(normalize(material.normalsColor.xyz * 2.f - 1.f), float3x3(geo.gTangent.xyz, geo.gBinormal.xyz, geo.gNormal.xyz))), 1.f);

        // Use real origin
        material.txcmid = float4(asfloat(packUnorm2x16(frac(geo.gTexcoord.xy))), asfloat(MatID), 1.f, 0.f); // 
    }

    // 
    return material;
};

// 
XHIT rasterize(in float3 origin, in float3 raydir, in float3 normal, float maxT, bool scatterTransparency, float threshold) {
    uint I = 0, R = 0; float lastMax = maxT, lastMin = 0.001f; float3 lastOrigin = origin + faceforward(normal.xyz, raydir.xyz, normal.xyz) * lastMin + raydir.xyz * lastMin;

    // 
    float fullLength = 0.f;
    float3 forigin = lastOrigin; // REQUIRED!
    bool restart = false;

    // 
    XHIT processing, confirmed;
    processing.origin.xyz = origin.xyz;
    processing.direct.xyz = raydir.xyz;
    processing.gIndices = uint4(0u);
    processing.gBarycentric = float4(0.f.xxx, lastMax);
    confirmed = processing;
    

    // 
    float3 sslr = world2screen(origin);
    const uint2 tsize = uint2(0); rasterBuffers[RS_MATERIAL].GetDimensions(tsize.x, tsize.y);
    const uint2 samplep = uint2((sslr.xy*0.5f+0.5f) * tsize);
    const uint4 indices  = asuint(rasterBuffers[RS_GEOMETRY].Sample(samplers[1u], samplep));
    const uint4 datapass = asuint(rasterBuffers[RS_MATERIAL].Sample(samplers[1u], samplep));

    // 
    const float3 baryCoord = rasterBuffers[RS_BARYCENT].Load(uint3(samplep, 0)); //texelFetch(rasterBuffers[RS_BARYCENT], samplep, 0).xyz;
    const bool isSkybox = dot(baryCoord.yz,1.f.xx)<=0.f; //uintBitsToFloat(datapass.z) <= 0.99f;
    const uint primitiveID = indices.z;
    const uint geometryInstanceID = indices.y;
    const uint globalInstanceID = indices.x;
    const uint nodeMeshID = getMeshID(rtxInstances[globalInstanceID]);

    // 
    if (!isSkybox) { // Only When Opaque!
        processing.direct = float4(raydir.xyz, 0.f);
        processing.origin = rasterBuffers[RS_POSITION].Load(uint3(samplep, 0));

        // Interpolate In Ray-Tracing
        processing.gIndices = indices;
        processing.gBarycentric = float4(baryCoord, distance(processing.origin.xyz, origin.xyz));

        // TODO: optimize material fetching
        XGEO geometry = interpolate(processing);
        XPOL material = materialize(processing, geometry);

        // TODO: rasterization direct diffuse access
        if (material.diffuseColor.w > (scatterTransparency ? random(seed) : threshold)) { // Only When Opaque!
            confirmed = processing;
        };
    };

    return confirmed;
};

#endif
