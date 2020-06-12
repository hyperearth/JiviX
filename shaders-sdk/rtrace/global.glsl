
// 
struct RCData {
    uvec4 udata;
    vec4 fdata;
};

// BUT DEFAULT OVERRIDEN!
ivec2 launchSize = ivec2(1600, 1200);

// 
struct XPOL {
     vec4  diffuseColor;
     vec4 emissionColor;
     vec4  normalsColor;
     vec4 specularColor;

     vec4 mapNormal; vec4 txcmid;
};

struct XGEO {
    vec4 gTangent; vec4 gBinormal; vec4 gNormal; vec4 gTexcoord;
};

struct XHIT {
     vec4 origin; vec4 direct;
     
     vec4 gBarycentric;
    uvec4 gIndices;
};

// 
uint packed = 0u;
uvec2 seed = uvec2(0u.xx);

// RESERVED FOR OTHER OPERATIONS
vec3 refractive(in vec3 dir) {
    return dir;
};

// 
mat4x4 inverse(in mat3x4 imat) {
    //return inverse(transpose(mat4x4(imat[0],imat[1],imat[2],vec4(0.f,0.f,0.f,1.f))));
    return transpose(inverse(mat4x4(imat[0],imat[1],imat[2],vec4(0.f,0.f,0.f,1.f))));
};



// 
XGEO interpolate(inout XHIT hit) { // By Geometry Data
    const uint geometryInstanceID = hit.gIndices.y;
    const uint globalInstanceID = hit.gIndices.x;
    const uint primitiveID = hit.gIndices.z;
    const uint nodeMeshID = getMeshID(rtxInstances[globalInstanceID]);
    const vec3 baryCoord = hit.gBarycentric.xyz;

    // 
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
    XGEO geometry;

    // 
    uvec3 idx3 = uvec3(primitiveID*3u+0u,primitiveID*3u+1u,primitiveID*3u+2u);
    geometry.gTexcoord  = vec4(triangulate(idx3, 1u, nodeMeshID,baryCoord).xyz,0.f);
    geometry.gNormal    = vec4(triangulate(idx3, 2u, nodeMeshID,baryCoord).xyz,0.f);
    geometry.gTangent   = vec4(triangulate(idx3, 3u, nodeMeshID,baryCoord).xyz,0.f);
    geometry.gBinormal  = vec4(triangulate(idx3, 4u, nodeMeshID,baryCoord).xyz,0.f);

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
}

// 
XPOL materialize(inout XHIT hit, inout XGEO geo) { // 
#define MatID geomMTs[nonuniformEXT(nodeMeshID)].materialID[geometryInstanceID]
    XPOL material; const vec4 skyColor = gSkyShader(hit.direct.xyz, hit.origin.xyz);

    // 
    material. diffuseColor = vec4(1.f.xxx, 0.f.x);
    material.emissionColor = skyColor;
    material. normalsColor = vec4(0.5f,0.5f,1.f,1.f);
    material.specularColor = vec4(0.f.xxx,0.f.x); // TODO: Correct Specular Initial
    material.mapNormal = geo.gNormal;
    material.txcmid = vec4(uintBitsToFloat(packUnorm2x16(0.f.xx)), 0.f, 0.f, 0.f); // 

    // 
    const uint geometryInstanceID = hit.gIndices.y;
    const uint globalInstanceID = hit.gIndices.x;
    const uint primitiveID = hit.gIndices.z;
    const uint nodeMeshID = getMeshID(rtxInstances[globalInstanceID]);
    const MaterialUnit unit = materials[0u].data[MatID]; // NEW! 20.04.2020
    const vec2 gTexcoord = geo.gTexcoord.xy;
    //const vec3 gNormal = geo.gNormal.xyz;

    // 
    if (hit.gBarycentric.w < 9999.f) {
        material. diffuseColor = toLinear(unit. diffuseTexture >= 0 ? texture(textures[nonuniformEXT(unit. diffuseTexture)],gTexcoord.xy) : unit.diffuse);
        material.emissionColor = toLinear(unit.emissionTexture >= 0 ? texture(textures[nonuniformEXT(unit.emissionTexture)],gTexcoord.xy) : unit.emission);
        material. normalsColor = unit. normalsTexture >= 0 ? texture(textures[nonuniformEXT(unit. normalsTexture)],gTexcoord.xy) : unit.normals;
        material.specularColor = unit.specularTexture >= 0 ? texture(textures[nonuniformEXT(unit.specularTexture)],gTexcoord.xy) : unit.specular;

        // Mapping
        material.mapNormal = vec4(normalize(mat3x3(geo.gTangent.xyz, geo.gBinormal.xyz, geo.gNormal.xyz) * normalize(material.normalsColor.xyz * 2.f - 1.f)), 1.f);

        // Use real origin
        material.txcmid = vec4(uintBitsToFloat(packUnorm2x16(fract(geo.gTexcoord.xy))), uintBitsToFloat(MatID), 1.f, 0.f); // 
    }

    // 
    return material;
}

// 
XHIT rasterize(in vec3 origin, in vec3 raydir, in vec3 normal, float maxT, bool scatterTransparency, float threshold) {
    XPOL material; uint32_t I = 0, R = 0; float lastMax = maxT, lastMin = 0.001f; vec3 lastOrigin = origin + faceforward(-normal.xyz, raydir.xyz, normal.xyz) * lastMin + faceforward(raydir.xyz, raydir.xyz, normal.xyz) * lastMin;
    material. diffuseColor = vec4(1.f.xxx, 0.f);
    material.emissionColor = vec4(gSkyShader(raydir.xyz, origin.xyz).xyz, 0.f.x);
    material. normalsColor = vec4(0.5f,0.5f,1.f,1.f);
    material.specularColor = vec4(0.f.xxx,0.f.x); // TODO: Correct Specular Initial
    material.mapNormal = vec4(vec3(0.f,1.f,0.f),1.f);
    material.txcmid = vec4(uintBitsToFloat(packUnorm2x16(0.f.xx)), 0.f, 0.f, 0.f); // 

    // 
    float fullLength = 0.f;
    vec3 forigin = lastOrigin; // REQUIRED!
    bool restart = false;

    // 
    XHIT processing;
    processing.origin.xyz = origin.xyz;
    processing.direct.xyz = raydir.xyz;
    processing.gIndices = uvec4(0u);
    processing.gBarycentric = vec4(0.f.xxx, lastMax);

    //
    XGEO geometry;
    geometry.gNormal = vec4(vec3(0.f,1.f,0.f),0.f);

    // 
    const ivec2 tsize = textureSize(rasterBuffers[RS_MATERIAL], 0);
    const ivec2 samplep = ivec2((world2screen(origin).xy*0.5f+0.5f) * textureSize(rasterBuffers[RS_MATERIAL], 0)); //samplep.y = tsize.y - samplep.y;
    const uvec4 indices  = floatBitsToUint(texelFetch(rasterBuffers[RS_GEOMETRY], samplep, 0));
    const uvec4 datapass = floatBitsToUint(texelFetch(rasterBuffers[RS_MATERIAL], samplep, 0));

    // 
    const vec2 baryCoord = uintBitsToFloat(datapass.xy);
    const bool isSkybox = dot(baryCoord.xy,1.f.xx)<=0.f; //uintBitsToFloat(datapass.z) <= 0.99f;
    const uint primitiveID = indices.z;
    const uint geometryInstanceID = indices.y;
    const uint globalInstanceID = indices.x;
    const uint nodeMeshID = getMeshID(rtxInstances[globalInstanceID]);

    // 
    if (!isSkybox) { // Only When Opaque!
        processing.direct = vec4(raydir.xyz, 0.f);
        processing.origin = texelFetch(rasterBuffers[RS_POSITION], samplep, 0);

        // Interpolate In Ray-Tracing
        processing.gIndices = indices;
        processing.gBarycentric.xyz = vec3(1.f-baryCoord.x-baryCoord.y,baryCoord);
        processing.gBarycentric.w = distance(processing.origin.xyz, origin.xyz);

        // TODO: optimize material fetching
        geometry = interpolate(processing);
        material = materialize(processing, geometry);
        
        // TODO: rasterization direct diffuse access
        if (material.diffuseColor.w > (scatterTransparency ? random(seed) : threshold)) { // Only When Opaque!
            
        };
    };
    //processing.gBarycentric.w = lastMax; // Debug

    return processing;
};

