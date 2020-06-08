// 
void directLight(in vec4 sphere, in vec3 origin, in vec3 normal, inout uvec2 seed, inout vec4 gSignal, inout vec4 gEnergy) {
    const vec3 lightp = sphere.xyz + randomSphere(seed) * sphere.w; float shdist = distance(lightp.xyz,origin.xyz);
    const vec3 lightd = normalize(lightp.xyz - origin.xyz);
    const vec3 lightc = 1024.f.xxx;

    if ( dot(normal, lightd) >= 0.f ) {
        float sdepth = raySphereIntersect(origin.xyz,lightd,sphere.xyz,sphere.w);
        XHIT result = traceRays(origin, lightd, normal, sdepth = sdepth <= 0.f ? 10000.f : sdepth, true, 0.01f);

        if ( min(sdepth, result.geoNormal.w) >= sdepth ) { // If intersects with light
            const float cos_a_max = sqrt(1.f - clamp(sphere.w * sphere.w / dot(sphere.xyz-origin.xyz, sphere.xyz-origin.xyz), 0.f, 1.f));
            gSignal += vec4(gEnergy.xyz * 2.f * (1.f - cos_a_max) * clamp(dot( lightd, normal.xyz ), 0.f, 1.f) * lightc, 0.f);
        };
    };
};

// 
XHIT rasterize(in vec3 origin, in vec3 raydir, in vec3 normal, float maxT, bool scatterTransparency, float threshold) {
    XHIT result; uint32_t I = 0, R = 0; float lastMax = maxT, lastMin = 0.001f; vec3 lastOrigin = origin;
    const vec4 skyColor = gSkyShader(raydir.xyz, origin.xyz);//vec4(texture(background, flip(lcts(raydir.xyz))).xyz, 1.f);
    result. diffuseColor = vec4(1.f.xxxx);
    result.emissionColor = vec4(skyColor.xyz,0.f.x);
    result. normalsColor = vec4(0.5f,0.5f,1.f,1.f);
    result.specularColor = vec4(0.f.xxx,0.f.x); // TODO: Correct Specular Initial
    result.geoNormal = vec4(vec3(0.f,1.f,0.f),lastMax);
    result.mapNormal = vec4(vec3(0.f,1.f,0.f),1.f);
    result.txcmid = vec4(uintBitsToFloat(packUnorm2x16(0.f.xx)), 0.f, 0.f, 0.f); // 
    result.origin.xyz = origin.xyz;
    result.gIndices = uvec4(0u);

    // 
    const ivec2 tsize = textureSize(rasterBuffers[RS_MATERIAL], 0);
    const ivec2 samplep = ivec2((world2screen(origin).xy*0.5f+0.5f) * textureSize(rasterBuffers[RS_MATERIAL], 0)); //samplep.y = tsize.y - samplep.y;
    const uvec4 indices  = floatBitsToUint(texelFetch(rasterBuffers[RS_GEOMETRY], samplep, 0));
    const uvec4 datapass = floatBitsToUint(texelFetch(rasterBuffers[RS_MATERIAL], samplep, 0));

    // 
    //const vec2 gTexcoord = unpackUnorm2x16(datapass.x);
    const vec2 baryCoord = uintBitsToFloat(datapass.xy);
    const bool isSkybox = dot(baryCoord.xy,1.f.xx)<=0.f; //uintBitsToFloat(datapass.z) <= 0.99f;

    // 
    const uint primitiveID = indices.z;
    const uint geometryInstanceID = indices.y;
    const uint globalInstanceID = indices.x;
    const uint nodeMeshID = getMeshID(rtxInstances[globalInstanceID]);

    // 
    const int IdxType = int(meshInfo[nodeMeshID].indexType)-1;
    uvec3 idx3 = uvec3(primitiveID*3u+0u,primitiveID*3u+1u,primitiveID*3u+2u);
    if (IdxType == IndexU8 ) { idx3 = uvec3(load_u8 (idx3.x*1u, 8u, nodeMeshID),load_u32(idx3.y*1u, 8u, nodeMeshID),load_u32(idx3.z*1u, 8u, nodeMeshID)); };
    if (IdxType == IndexU16) { idx3 = uvec3(load_u16(idx3.x*2u, 8u, nodeMeshID),load_u16(idx3.y*2u, 8u, nodeMeshID),load_u16(idx3.z*2u, 8u, nodeMeshID)); };
    if (IdxType == IndexU32) { idx3 = uvec3(load_u32(idx3.x*4u, 8u, nodeMeshID),load_u32(idx3.y*4u, 8u, nodeMeshID),load_u32(idx3.z*4u, 8u, nodeMeshID)); };

    // Interpolate In Ray-Tracing 
    //texelFetch(rasterBuffers[RS_BARYCENT], samplep, 0).xyz;
    const vec4 gTexcoord = vec4(unpackUnorm2x16(datapass.z),0.f.xx); //vec4(triangulate(idx3, 1u, nodeMeshID,vec3(1.f-baryCoord.x-baryCoord.y,baryCoord.xy)).xyz,0.f);
    const vec4 gNormal = vec4(triangulate(idx3, 2u, nodeMeshID,vec3(1.f-baryCoord.x-baryCoord.y,baryCoord.xy)).xyz,0.f);
    const vec4 gTangent = vec4(triangulate(idx3, 3u, nodeMeshID,vec3(1.f-baryCoord.x-baryCoord.y,baryCoord.xy)).xyz,0.f);
    const vec4 gBinormal = vec4(triangulate(idx3, 4u, nodeMeshID,vec3(1.f-baryCoord.x-baryCoord.y,baryCoord.xy)).xyz,0.f);

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
    const MaterialUnit unit = materials[0u].data[MatID]; // NEW! 20.04.2020
    //vec4 diffuseColor = texelFetch(rasterBuffers[RS_DIFFUSED], samplep, 0);//
    //diffuseColor.w = (toLinear(unit. diffuseTexture >= 0 ? texture(textures[nonuniformEXT(unit. diffuseTexture)],gTexcoord.xy) : unit.diffuse)).w;
    const vec4 diffuseColor = toLinear(unit. diffuseTexture >= 0 ? texture(textures[nonuniformEXT(unit. diffuseTexture)],gTexcoord.xy) : unit.diffuse);
    if (!isSkybox) { // Only When Opaque!
        result.origin = texelFetch(rasterBuffers[RS_POSITION], samplep, 0);
        result.txcmid = uintBitsToFloat(uvec4(packUnorm2x16(gTexcoord.xy), MatID, floatBitsToUint(1.f), floatBitsToUint(0.f)));//texelFetch(rasterBuffers[RS_MATERIAL], samplep, 0);

        // 
        result. diffuseColor = diffuseColor;//toLinear(unit. diffuseTexture >= 0 ? texture(textures[nonuniformEXT(unit. diffuseTexture)],gTexcoord.xy) : unit.diffuse);
        result.emissionColor = toLinear(unit.emissionTexture >= 0 ? texture(textures[nonuniformEXT(unit.emissionTexture)],gTexcoord.xy) : unit.emission);
        result. normalsColor = unit. normalsTexture >= 0 ? texture(textures[nonuniformEXT(unit. normalsTexture)],gTexcoord.xy) : unit.normals;
        result.specularColor = unit.specularTexture >= 0 ? texture(textures[nonuniformEXT(unit.specularTexture)],gTexcoord.xy) : unit.specular;

        // 
        result.geoNormal = normalize(gNormal);
        result.gBinormal = normalize(gBinormal);
        result.gTangent  = normalize(gTangent);

        // FIX NORMAL ISSUE (04.04.2020)
        result.geoNormal.xyz = normalize(result.geoNormal.xyz * normalTransform * normInTransform);
        result.gBinormal.xyz = normalize(result.gBinormal.xyz * normalTransform * normInTransform);
        result.gTangent .xyz = normalize(result.gTangent .xyz * normalTransform * normInTransform);

        // Mapping
        result.geoNormal.xyz = normalize(faceforward(result.geoNormal.xyz, raydir.xyz, result.geoNormal.xyz));
        result.mapNormal = vec4(normalize(mat3x3(result.gTangent.xyz, result.gBinormal.xyz, result.geoNormal.xyz) * normalize(result.normalsColor.xyz * 2.f - 1.f)), 1.f);
        result.geoNormal.w = distance(result.origin.xyz, origin.xyz);

        // Normalize
        result.geoNormal.xyz = normalize(result.geoNormal.xyz);
        result.gTangent .xyz = normalize(result.gTangent.xyz);
        result.gBinormal.xyz = normalize(result.gBinormal.xyz);
        result.gIndices = uvec4(geometryInstanceID, globalInstanceID, 0u, 0u); // already nodeMeshID used by instance

        // Fix light leaks
        result.mapNormal.xyz = normalize(faceforward(result.mapNormal.xyz, raydir.xyz, result.geoNormal.xyz));
    };

    return result;
};

