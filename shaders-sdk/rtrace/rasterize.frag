#version 460 core // #
#extension GL_GOOGLE_include_directive : require
#extension GL_EXT_ray_tracing          : require
#extension GL_EXT_ray_query            : require
#extension GL_ARB_post_depth_coverage  : require
#include "./driver.glsl"

layout ( binding = 2, set = 1 ) uniform accelerationStructureEXT Scene;
layout ( post_depth_coverage ) in;
layout ( early_fragment_tests ) in; // Reduce Lag Rate! (but transparency may broken!)
// Прозрачность с новой прошивкой починим! @RED21

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
layout (location = NORMALEF) out vec4 normals;
layout (location = EMISSION) out vec4 emission;
layout (location = SPECULAR) out vec4 specular;
layout (location = SAMPLEPT) out vec4 gsamplept; 

layout (location = RFLVALUE) out vec4 reflval;
layout (location = RAYQUERY) out vec4 diffuse; 


// 
struct XHIT {
    vec4  diffuseColor;
    vec4 emissionColor;
    vec4  normalsColor;
    vec4 specularColor;

    vec4 geoNormal;
    vec4 mapNormal;
    vec4 origin;

    vec4 gTangent;
    vec4 gBinormal;
};


XHIT traceRays(in vec3 origin, in vec3 raydir, in vec3 normal, float maxT) {
    XHIT result; uint32_t I = 0, R = 0; float lastMax = maxT, lastMin = 0.001f; vec3 lastOrigin = origin;
    result. diffuseColor = vec4(1.f.xxxx);
    result.emissionColor = vec4(gSkyColor,0.f.x);
    result. normalsColor = vec4(0.5f,0.5f,1.f,1.f);
    result.specularColor = vec4(0.f.xxx,0.f.x); // TODO: Correct Specular Initial
    result.geoNormal = vec4(vec3(0.f,1.f,0.f),lastMax);
    result.mapNormal = vec4(vec3(0.f,1.f,0.f),1.f);
    //result.tangent = vec4(vec3(0.f,0.f,1.f),lastMin); // UNUSED

    float fullLength = 0.f;
    vec3 forigin = lastOrigin; // REQUIRED!
    bool restart = false;

    // 
    XHIT processing = result;
    while((R++) < 4) { // restart needs for transparency (after every resolve)
        rayQueryEXT rayQuery;
        rayQueryInitializeEXT(rayQuery, Scene, gl_RayFlagsOpaqueEXT|gl_RayFlagsCullNoOpaqueEXT,
            0xFF, forigin + faceforward(normal.xyz,-raydir.xyz,normal.xyz) * 0.001f + raydir.xyz * 0.001f, lastMin, raydir, lastMax = (maxT - fullLength));

        while((I++) < 2) {
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
                tHit = rayQueryGetIntersectionTEXT(rayQuery, true);
            } else {
                nodeMeshID = rayQueryGetIntersectionInstanceCustomIndexEXT(rayQuery, false); // Mesh ID from Node Mesh List (because indexing)
                geometryInstanceID = rayQueryGetIntersectionGeometryIndexEXT(rayQuery, false); // TODO: Using In Ray Tracing (and Query) shaders!
                globalInstanceID = rayQueryGetIntersectionInstanceIdEXT(rayQuery, false);
                baryCoord = rayQueryGetIntersectionBarycentricsEXT(rayQuery, false);
                primitiveID = rayQueryGetIntersectionPrimitiveIndexEXT(rayQuery, false); 
                origin = rayQueryGetIntersectionObjectRayOriginEXT(rayQuery, false);
                tHit = rayQueryGetIntersectionTEXT(rayQuery, false);
            };

            // TODO: ???
            //origin += raydir*tHit;

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
                    processing. diffuseColor = diffuseColor;//toLinear(unit. diffuseTexture >= 0 ? texture(textures[nonuniformEXT(unit. diffuseTexture)],gTexcoord.xy) : unit.diffuse);
                    processing.emissionColor = toLinear(unit.emissionTexture >= 0 ? texture(textures[nonuniformEXT(unit.emissionTexture)],gTexcoord.xy) : unit.emission);
                    processing. normalsColor = unit. normalsTexture >= 0 ? texture(textures[nonuniformEXT(unit. normalsTexture)],gTexcoord.xy) : unit.normals;
                    processing.specularColor = unit.specularTexture >= 0 ? texture(textures[nonuniformEXT(unit.specularTexture)],gTexcoord.xy) : unit.specular;

                    // 
                    processing.geoNormal = gNormal;
                    processing.mapNormal = vec4(normalize(mat3x3(gTangent.xyz,gBinormal.xyz,gNormal.xyz) * normalize(processing.normalsColor.xyz * 2.f - 1.f)),1.f);
                    processing.geoNormal.w = fullLength + lastMax;
                    processing.gBinormal = gBinormal;
                    processing.gTangent = gTangent;

                    processing.origin = vec4(lastOrigin,1.f);
                    //processing.tangent = gTangent; // UNUSED

                    if (!complete) { rayQueryConfirmIntersectionEXT(rayQuery); };
                } else { // It's transparent, need ray-trace again! (but with another position)
                    if (complete) { restart = true; };
                };
            };

            if (complete) { break; };
        };
        
        // 
        //rayQueryConfirmIntersectionEXT(rayQuery);
        rayQueryTerminateEXT(rayQuery);

        // 
        fullLength += lastMax, forigin = lastOrigin;
        if (!restart || fullLength >= (maxT-1.f)) { break; } else { restart = false; }; // With Correction* 
    };

    // 
    return processing;///(fullLength >= (maxT-1.f)) ? result : processing;
};

// 
void directLight(in vec4 sphere, in vec3 origin, in vec3 normal, inout uvec2 seed, inout vec4 gSignal, inout vec4 gEnergy){
    const vec3 lightp = sphere.xyz + randomSphere(seed) * sphere.w; float shdist = distance(lightp.xyz,origin.xyz);
    const vec3 lightd = normalize(lightp.xyz - origin.xyz);
    const vec3 lightc = 512.f.xxx;//32.f*4096.f.xxx/(sphere.w*sphere.w);

    float sdepth = raySphereIntersect(origin.xyz,lightd,sphere.xyz,sphere.w);
     XHIT result = traceRays(origin, lightd, normal, sdepth = sdepth <= 0.f ? 10000.f : sdepth);

    if ( min(sdepth, result.geoNormal.w) >= sdepth ) { // If intersects with light
        const float cos_a_max = sqrt(1.f - clamp(sphere.w * sphere.w / dot(sphere.xyz-origin.xyz, sphere.xyz-origin.xyz), 0.f, 1.f));
        gSignal += vec4(gEnergy.xyz * 2.f * (1.f - cos_a_max) * clamp(dot( lightd, normal.xyz ), 0.f, 1.f) * lightc, 0.f);
    };
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
    vec3 gNormal = normalize(TBN*normalize(normalsColor.xyz * 2.f - 1.f));
    TBN[2] = gNormal;
    
    // 
    if (diffuseColor.w > 0.001f) {
#ifndef CONSERVATIVE
        colored   = vec4(max(vec4(diffuseColor.xyz-clamp(emissionColor.xyz*emissionColor.w,0.f.xxx,1.f.xxx),0.f),0.f.xxxx).xyz, 1.f);
        gsamplept = vec4(fPosition.xyz                    , 1.f); // used for ray-start position
        emission  = vec4(emissionColor.xyz*emissionColor.w, 1.f);
        specular  = vec4(specularColor.xyz*specularColor.w, 1.f);

        // Initial
        reflval = vec4(gSkyColor  , 1.f);
        diffuse = vec4(1.f.xxx    , 1.f);
        normals = vec4(gNormal.xyz, 1.f);
#else
        // For Reprojection (COVER)
        samples = vec4(fPosition.xyz, 1.f);
        normals = vec4(gNormal.xyz,   1.f);
#endif
        gl_FragDepth = gl_FragCoord.z;
    } else {
#ifndef CONSERVATIVE
        colored   = vec4(max(vec4(diffuseColor.xyz-clamp(emissionColor.xyz*emissionColor.w,0.f.xxx,1.f.xxx),0.f),0.f.xxxx).xyz, 0.f);
        gsamplept = vec4(fPosition.xyz, 0.f); // used for ray-start position
        emission  = vec4(gSkyColor    , 0.f);
        specular  = vec4(0.f.xxx      , 0.f);

        // Initial
        reflval = vec4(gSkyColor  , 0.f);
        diffuse = vec4(1.f.xxx    , 0.f);
        normals = vec4(gNormal.xyz, 0.f);
#else
        // For Reprojection (COVER)
        samples = vec4(fPosition.xyz, 0.f);
        normals = vec4(gNormal.xyz  , 0.f);
#endif
        gl_FragDepth = 1.f;
    };

/* //DEBUG!
#ifndef CONSERVATIVE
    if (colored.w > 0.001f && diffuseColor.w > 0.001f) {
        const vec3 cameraSample = vec4(fPosition.xyz,1.f)*modelview;
        vec3 origin = vec3(fPosition.xyz), raydir = (modelview * normalize(cameraSample.xyz)).xyz;
        vec3 normal = normalize( faceforward(gNormal.xyz.xyz, raydir.xyz, gNormal.xyz).xyz);//(modelview * normalize(gNormal.xyz)).xyz;
        XHIT result = traceRays(origin, normalize(reflect(raydir,normal)), normal, 10000.f);
        if (result.geoNormal.w <= 9999.f) {
            reflval = vec4(max(result.diffuseColor.xyz-result.emissionColor.xyz,0.f.xxx)+result.emissionColor.xyz,1.f);
        } else {
            reflval = vec4(gSkyColor,1.f);
        };
    };
#endif
*/

// IT'S REPLACEMENT OF SSLR (i.e. ASR, "Almost Screen Reflection")
// Anti-Aliasing and Transparency TBA
#ifndef CONSERVATIVE // Full Version of Ray Tracing 
    if (colored.w > 0.001f && diffuseColor.w > 0.001f) {
        const vec3 cameraSample = vec4(fPosition.xyz,1.f)*modelview;
        vec3 forigin = vec3(fPosition.xyz), fraydir = (modelview * normalize(cameraSample.xyz)).xyz;
        vec3 fnormal = normalize(gNormal.xyz);//(modelview * normalize(gNormal.xyz)).xyz;

        const vec4 sphere = vec4(vec3(16.f,128.f,16.f), 8.f);
        const uint packed = pack32(u16vec2(gl_FragCoord.xy));

        const vec4 bspher = vec4(forigin,10000.f);
        const float inIOR = 1.f, outIOR = 1.6666f;

        uvec2 seed = uvec2(packed,rdata.x);
        for (uint I=0;I<2;I++) {
            vec3 raydir = I == 0 ? randomHemisphereCosine(seed, TBN) : reflectGlossy(seed, fraydir.xyz, TBN, specularColor.y);
            vec3 origin = forigin, normal = normalize(faceforward(fnormal.xyz.xyz, fraydir.xyz, fnormal.xyz).xyz);

            vec4 gEnergy = vec4(1.f.xxxx), gSignal = vec4(0.f.xxx,1.f);
            if ( I == 0 ) { directLight(sphere, origin, normal, seed, gSignal, gEnergy); };

            for (uint i=0;i<2;i++) { // 
                XHIT result = traceRays(origin, raydir, normal, 10000.f);

                // 
                float sdepth = raySphereIntersect(origin.xyz,raydir.xyz,sphere.xyz,sphere.w); sdepth = sdepth <= 0.f ? 10000.f : sdepth;
                float mvalue = min(result.geoNormal.w, 10000.f);

                // power of reflection
                float reflectionPower = mix(clamp(pow(1.0f + dot(raydir.xyz, result.mapNormal.xyz), outIOR/inIOR), 0.f, 1.f) * 0.3333f, 1.f, result.specularColor.z);
                bool couldReflection = random(seed) <= reflectionPower;

                // 
                if ( result.geoNormal.w >= 9999.f ) {
                    const float sdepth = raySphereIntersect(origin.xyz,raydir.xyz,bspher.xyz,bspher.w); mvalue = (sdepth <= 0.f ? 10000.f : sdepth);
                    BACKSKY_COLOR;
                } else 
                if ( result.diffuseColor.w > 0.001f ) {
                    if (couldReflection) {
                        gEnergy *= vec4(mix(1.f.xxx, result.diffuseColor.xyz, result.specularColor.zzz), 1.f);
                    } else {
                        gSignal.xyz += gEnergy.xyz * result.emissionColor.xyz * result.emissionColor.w;
                        gEnergy *= vec4(max(result.diffuseColor.xyz - clamp(result.emissionColor.xyz*result.emissionColor.w,0.f.xxx,1.f.xxx), 0.f.xxx), 1.f);
                    };
                } else { // VOID!
                    gEnergy *= vec4(0.f.xxxx);
                }

                // 
                raydir.xyz = couldReflection ? 
                    reflectGlossy(seed, raydir.xyz, mat3x3(result.gTangent.xyz,result.gBinormal.xyz,result.mapNormal.xyz), result.specularColor.y) : 
                    randomHemisphereCosine(seed, mat3x3(result.gTangent.xyz,result.gBinormal.xyz,result.mapNormal.xyz));

                // 
                normal.xyz = result.mapNormal.xyz;
                normal = normalize(faceforward(normal.xyz.xyz, raydir.xyz, normal.xyz).xyz);
                origin.xyz = result.origin.xyz;
                origin.xyz += faceforward(result.geoNormal.xyz,-raydir.xyz,result.geoNormal.xyz) * 0.0001f + raydir.xyz * 0.0001f;
                if ((dot(gEnergy.xyz,1.f.xxx)/3.f) <= 0.001f || result.geoNormal.w >= 9999.f || dot(raydir.xyz,result.geoNormal.xyz) <= 0.f) { break; }; //

                // 
                directLight(sphere, origin, normal, seed, gSignal, gEnergy);
            };

            // 
            if (I == 0) { diffuse = vec4(gSignal.xyz,1.f); };
            if (I == 1) { reflval = vec4(gSignal.xyz,1.f); };
        };
    };
#endif


    //ivec2 txd = ivec2(gl_FragCoord.xy), txs = imageSize(writeImages[DIFFUSED]);
    //const vec4 dEmi = imageLoad(writeImages[DIFFUSED], ivec2(txd.x,txs.y-txd.y-1));
    //imageStore(writeImages[DIFFUSED], ivec2(txd.x,txs.y-txd.y-1), vec4(emissionColor.xyz*emissionColor.w,0.f)+dEmi);
};
//
