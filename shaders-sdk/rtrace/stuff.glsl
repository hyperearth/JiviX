// 
void directLight(inout XHIT RES, in vec4 sphere, in vec3 origin, in vec3 normal, inout uvec2 seed, inout vec4 gSignal, inout vec4 gEnergy) {
    const vec3 lightp = sphere.xyz + randomSphere(seed) * sphere.w; float shdist = distance(lightp.xyz,origin.xyz);
    const vec3 lightd = normalize(lightp.xyz - origin.xyz);
    const vec3 lightc = 1024.f.xxx;

    if ( dot(normal, lightd) >= 0.f ) {
        float sdepth = raySphereIntersect(origin.xyz,lightd,sphere.xyz,sphere.w);
        XHIT result = traceRays(origin, lightd, normal, sdepth = sdepth <= 0.f ? 10000.f : sdepth, true, 0.01f);

        if ( min(sdepth, RES.gBarycentric.w) >= sdepth ) { // If intersects with light
            const float cos_a_max = sqrt(1.f - clamp(sphere.w * sphere.w / dot(sphere.xyz-origin.xyz, sphere.xyz-origin.xyz), 0.f, 1.f));
            gSignal += vec4(gEnergy.xyz * 2.f * (1.f - cos_a_max) * clamp(dot( lightd, normal.xyz ), 0.f, 1.f) * lightc, 0.f);
        };
    };
};

// 
XHIT rasterize(in vec3 origin, in vec3 raydir, in vec3 normal, float maxT, bool scatterTransparency, float threshold) {
    XPOL material; uint32_t I = 0, R = 0; float lastMax = maxT, lastMin = 0.001f; vec3 lastOrigin = origin + faceforward(-normal.xyz, raydir.xyz, normal.xyz) * lastMin + faceforward(raydir.xyz, raydir.xyz, normal.xyz) * lastMin;
    const vec4 skyColor = gSkyShader(raydir.xyz, origin.xyz);//vec4(texture(background, flip(lcts(raydir.xyz))).xyz, 1.f);
    material. diffuseColor = vec4(1.f.xxx, 0.f);
    material.emissionColor = vec4(skyColor.xyz,0.f.x);
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
    XGEO geometry;

    // 
    processing.origin.xyz = origin.xyz;
    processing.direct.xyz = raydir.xyz;
    processing.gIndices = uvec4(0u);
    processing.gBarycentric = vec4(0.f.xxx, lastMax);
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
        const int IdxType = int(meshInfo[nodeMeshID].indexType)-1;

        // Interpolate In Ray-Tracing
        processing.gIndices = indices;
        processing.gBarycentric.xyz = vec3(1.f-baryCoord.x-baryCoord.y,baryCoord);
        geometry = interpolate(processing);

        // TODO: optimize material fetching
        material = materialize(processing, geometry);

        // TODO: rasterization direct diffuse access
        if (material.diffuseColor.w > (scatterTransparency ? random(seed) : threshold)) { // Only When Opaque!
            processing.gBarycentric.w = fullLength + lastMax;
            processing.direct = vec4(raydir.xyz, 0.f);
            processing.origin = vec4(lastOrigin, 1.f);
        }; 
    };

    return processing;
};
