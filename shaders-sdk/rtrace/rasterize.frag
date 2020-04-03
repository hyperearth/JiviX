#version 460 core // #
#extension GL_GOOGLE_include_directive : require
#extension GL_EXT_ray_tracing          : require
#extension GL_EXT_ray_query            : require
#include "./driver.glsl"

layout ( binding = 2, set = 1 ) uniform accelerationStructureEXT Scene;

// 
layout (location = 0) in vec4 fPosition;
layout (location = 1) in vec4 fTexcoord;
layout (location = 2) in vec4 fNormal;
layout (location = 3) in vec4 fTangent;
layout (location = 4) in vec4 fBinormal;
layout (location = 5) flat in uvec4 uData;

// 
layout (location = COLORING) out vec4 colored;
layout (location = POSITION) out vec4 samples;
//layout (location = NORMALED) out vec4 normals;
layout (location = TANGENTS) out vec4 tangent;
layout (location = EMISSION) out vec4 emission;
layout (location = SPECULAR) out vec4 specular;
layout (location = GEONORML) out vec4 geonormal;
layout (location = SAMPLEPT) out vec4 gsamplept; 
layout (location = RAYQUERY) out vec4 diffuse; 


// 
struct XHIT {
    vec4  diffuseColor;
    vec4 emissionColor;
    vec4  normalsColor;
    vec4 specularColor;

    vec4 geoNormal;
    vec4 mapNormal;
    //vec4 tangent;
};


XHIT traceRays(in vec3 origin, in vec3 raydir, in vec3 normal, float maxT) {
    XHIT result; uint32_t I = 0, R = 0; float lastMax = maxT, lastMin = 0.001f; vec3 lastOrigin = origin;
    result. diffuseColor = vec4(1.f.xxxx);
    result.emissionColor = vec4(0.f.xxxx);
    result. normalsColor = vec4(0.5f,0.5f,1.f,1.f);
    result.specularColor = vec4(0.f.xxxx); // TODO: Correct Specular Initial
    result.geoNormal = vec4(vec3(0.f,1.f,0.f),lastMax);
    result.mapNormal = vec4(vec3(0.f,1.f,0.f),1.f);
    //result.tangent = vec4(vec3(0.f,0.f,1.f),lastMin); // UNUSED

    float fullLength = 0.f;
    vec3 forigin = lastOrigin; // REQUIRED!
    bool restart = false;
    rayQueryEXT rayQuery;

    // 
    while((R++) < 4) { // restart needs for transparency (after every resolve)
        rayQueryInitializeEXT(rayQuery, Scene,
            gl_RayFlagsTerminateOnFirstHitEXT|gl_RayFlagsOpaqueEXT,
            0xFF, forigin + faceforward(normal.xyz,-raydir.xyz,normal.xyz) * 0.001f + raydir.xyz * 0.001f, lastMin, raydir, lastMax = (maxT - fullLength));

        while((I++) < 32) {
            bool complete = !rayQueryProceedEXT(rayQuery);

            // Full Instance ID of Node (BY GEOMETRY INSTATNCE!!)
            uint primitiveID = -1, nodeMeshID = -1, geometryInstanceID = -1, globalInstanceID = -1;
            vec2 baryCoord = vec2(0.f,0.f);
            float tHit = lastMax;

            // 
            if (complete) {
                nodeMeshID = rayQueryGetIntersectionInstanceCustomIndexEXT(rayQuery, true); // Mesh ID from Node Mesh List (because indexing)
                geometryInstanceID = rayQueryGetIntersectionGeometryIndexEXT(rayQuery, true); // TODO: Using In Ray Tracing (and Query) shaders!
                globalInstanceID = rayQueryGetIntersectionInstanceIdEXT(rayQuery, true);
                baryCoord = rayQueryGetIntersectionBarycentricsEXT(rayQuery, true);
                primitiveID = rayQueryGetIntersectionPrimitiveIndexEXT(rayQuery, true); 
                origin = rayQueryGetIntersectionObjectRayOriginEXT(rayQuery, true);
                tHit = length(rayQueryGetIntersectionObjectRayOriginEXT(rayQuery, true)-forigin.xyz);
            } else {
                nodeMeshID = rayQueryGetIntersectionInstanceCustomIndexEXT(rayQuery, false); // Mesh ID from Node Mesh List (because indexing)
                geometryInstanceID = rayQueryGetIntersectionGeometryIndexEXT(rayQuery, false); // TODO: Using In Ray Tracing (and Query) shaders!
                globalInstanceID = rayQueryGetIntersectionInstanceIdEXT(rayQuery, false);
                baryCoord = rayQueryGetIntersectionBarycentricsEXT(rayQuery, false);
                primitiveID = rayQueryGetIntersectionPrimitiveIndexEXT(rayQuery, false); 
                origin = rayQueryGetIntersectionObjectRayOriginEXT(rayQuery, false);
                tHit = length(rayQueryGetIntersectionObjectRayOriginEXT(rayQuery, false)-forigin.xyz);
            };

            // 
            if (lastMax > tHit) { lastOrigin = origin, lastMax = tHit; // type definition
                const int IdxType = int(meshInfo[nodeMeshID].indexType)-1;
                uvec3 idx3 = uvec3(primitiveID*3u+0u,primitiveID*3u+1u,primitiveID*3u+2u);
                if (IdxType == IndexU8 ) { idx3 = uvec3(load_u8 (idx3.x*1u, 8u, nodeMeshID),load_u32(idx3.y*1u, 8u, nodeMeshID),load_u32(idx3.z*1u, 8u, nodeMeshID)); };
                if (IdxType == IndexU16) { idx3 = uvec3(load_u16(idx3.x*2u, 8u, nodeMeshID),load_u16(idx3.y*2u, 8u, nodeMeshID),load_u16(idx3.z*2u, 8u, nodeMeshID)); };
                if (IdxType == IndexU32) { idx3 = uvec3(load_u32(idx3.x*4u, 8u, nodeMeshID),load_u32(idx3.y*4u, 8u, nodeMeshID),load_u32(idx3.z*4u, 8u, nodeMeshID)); };

                // Interpolate In Ray-Tracing 
                const vec4 gTexcoord = vec4(triangulate(idx3, 1u, nodeMeshID,vec3(1.f-baryCoord.x-baryCoord.y,baryCoord)).xyz,0.f);
                const vec4 gNormal = vec4(triangulate(idx3, 2u, nodeMeshID,vec3(1.f-baryCoord.x-baryCoord.y,baryCoord)).xyz,0.f);
                const vec4 gTangent = vec4(triangulate(idx3, 3u, nodeMeshID,vec3(1.f-baryCoord.x-baryCoord.y,baryCoord)).xyz,0.f);
                const vec4 gBinormal = vec4(triangulate(idx3, 4u, nodeMeshID,vec3(1.f-baryCoord.x-baryCoord.y,baryCoord)).xyz,0.f);

                // By Geometry Data
                mat3x4 matras = mat3x4(instances[nodeMeshID].transform[geometryInstanceID]);
                if (!hasTransform(meshInfo[nodeMeshID])) {
                    matras = mat3x4(vec4(1.f,0.f.xxx),vec4(0.f,1.f,0.f.xx),vec4(0.f.xx,1.f,0.f));
                };

                // By Instance Data
                const mat3x4 matra4 = rtxInstances[globalInstanceID].transform;

                // Native Normal Transform
                const mat3x3 normalTransform = inverse(transpose(regen3(matras)));
                const mat3x3 normInTransform = inverse(transpose(regen3(matra4)));

                // 
                const MaterialUnit unit = materials[0u].data[meshInfo[nodeMeshID].materialID];
                const vec4 diffuseColor = toLinear(unit. diffuseTexture >= 0 ? texture(textures[nonuniformEXT(unit. diffuseTexture)],gTexcoord.xy) : unit.diffuse);
                if (diffuseColor.w > 0.001f) { // Only When Opaque!
                    result. diffuseColor = diffuseColor;//toLinear(unit. diffuseTexture >= 0 ? texture(textures[nonuniformEXT(unit. diffuseTexture)],gTexcoord.xy) : unit.diffuse);
                    result.emissionColor = toLinear(unit.emissionTexture >= 0 ? texture(textures[nonuniformEXT(unit.emissionTexture)],gTexcoord.xy) : unit.emission);
                    result. normalsColor = unit. normalsTexture >= 0 ? texture(textures[nonuniformEXT(unit. normalsTexture)],gTexcoord.xy) : unit.normals;
                    result.specularColor = unit.specularTexture >= 0 ? texture(textures[nonuniformEXT(unit.specularTexture)],gTexcoord.xy) : unit.specular;

                    // 
                    result.geoNormal = gNormal;
                    result.mapNormal = vec4(normalize(mat3x3(gTangent.xyz,gBinormal.xyz,gNormal.xyz) * normalize(result.normalsColor.xyz * 2.f - 1.f)),1.f);
                    result.geoNormal.w = fullLength;
                    //result.tangent = gTangent; // UNUSED
                } else { // It's transparent, need ray-trace again! (but with another position)
                    if (complete) { restart = true; };
                };
                
            };

            if (complete) { break; };
        };

        // 
        fullLength += lastMax, forigin = lastOrigin;
        if (!restart || fullLength >= (maxT-1.f)) { break; } else { restart = false; }; // With Correction* 
    };

    // 
    return result;
};

// 
void main() { // hasTexcoord(meshInfo[drawInfo.data.x])
    const MaterialUnit unit = materials[0u].data[meshInfo[drawInfo.data.x].materialID];
    vec4 diffuseColor = toLinear(unit.diffuseTexture >= 0 ? texture(textures[nonuniformEXT(unit.diffuseTexture)],fTexcoord.xy,0) : unit.diffuse);
    vec4 normalsColor = unit.normalsTexture >= 0 ? texture(textures[nonuniformEXT(unit.normalsTexture)],fTexcoord.xy,0) : unit.normals;
    vec4 specularColor = unit.specularTexture >= 0 ? texture(textures[nonuniformEXT(unit.specularTexture)],fTexcoord.xy,0) : unit.specular;
    vec4 emissionColor = toLinear(unit.emissionTexture >= 0 ? texture(textures[nonuniformEXT(unit.emissionTexture)],fTexcoord.xy,0) : unit.emission);
    
    // 
    vec3 gTangent = fTangent.xyz - dot(fNormal.xyz,fTangent.xyz)*fNormal.xyz;
    vec3 gBinormal = fBinormal.xyz - dot(fNormal.xyz,fBinormal.xyz)*fNormal.xyz;
    
    // 
    mat3x3 TBN = mat3x3(normalize(gTangent.xyz),normalize(gBinormal),normalize(fNormal.xyz));
    vec3 gNormal = normalize(TBN*(normalsColor.xyz * 2.f - 1.f));

    // 
    if (diffuseColor.w > 0.001f) {
#ifndef CONSERVATIVE
        samples = 0.f.xxxx;
        colored = vec4(max(vec4(diffuseColor.xyz-clamp(emissionColor.xyz*emissionColor.w,0.f.xxx,1.f.xxx),0.f),0.f.xxxx).xyz,1.f);
        gsamplept = vec4(fPosition.xyz,1.f); // used for ray-start position
        emission = vec4(emissionColor.xyz*emissionColor.w,1.f);
        specular = vec4(specularColor.xyz*specularColor.w,1.f);
#else
        samples = vec4(fPosition.xyz,1.f); // covered center of pixel (used for resampling tests)
        colored = 0.f.xxxx;
        gsamplept = 0.f.xxxx;
        specular = 0.f.xxxx;
        emission = 0.f.xxxx;
#endif
        geonormal = vec4(normalize(fNormal.xyz),1.f);
        //normals = vec4(gNormal.xyz,1.f);
        tangent = vec4(gTangent.xyz,1.f);
        gl_FragDepth = gl_FragCoord.z;
    } else {
        colored = 0.f.xxxx;
        specular = 0.f.xxxx;
        emission = 0.f.xxxx;
        //normals = vec4(0.f.xx,0.f.xx);
        tangent = vec4(0.f.xxx,0.f.x);
        samples = vec4(0.f.xxx,0.f.x);
        gsamplept = vec4(0.f.xxx,0.f.x);
        geonormal = vec4(0.f.xxx,0.f.x);
        gl_FragDepth = 1.f;
    };

// IT'S REPLACEMENT OF SSLR (i.e. ASR, "Almost Screen Reflection")
// Anti-Aliasing and Transparency TBA
#ifdef CONSERVATIVE // Full Version of Ray Tracing 
    const vec3 cameraSample = vec4(fPosition.xyz,1.f)*modelview;
    vec3 origin = vec3(fPosition.xyz), raydir = (modelview * normalize(cameraSample.xyz)).xyz, lastOrigin = origin;
    vec3 normal = normalize(gNormal.xyz);//(modelview * normalize(gNormal.xyz)).xyz;

    for (uint i=0;i<1;i++) {
        XHIT result = traceRays(origin,raydir = normalize(reflect(raydir, normal)), normal, 10000.f);
        
        if (result.geoNormal.w <= 9999.f) { diffuse = vec4(max(result.diffuseColor.xyz-result.emissionColor.xyz,0.f.xxx)+result.emissionColor.xyz,1.f); };
    };
#endif

    //ivec2 txd = ivec2(gl_FragCoord.xy), txs = imageSize(writeImages[DIFFUSED]);
    //const vec4 dEmi = imageLoad(writeImages[DIFFUSED], ivec2(txd.x,txs.y-txd.y-1));
    //imageStore(writeImages[DIFFUSED], ivec2(txd.x,txs.y-txd.y-1), vec4(emissionColor.xyz*emissionColor.w,0.f)+dEmi);
};
