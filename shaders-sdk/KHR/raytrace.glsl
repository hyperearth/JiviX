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
    const uvec4 indices = floatBitsToUint(texelFetch(rasterBuffers[RS_GEOMETRY], samplep, 0));
    const uvec4 datapass = floatBitsToUint(texelFetch(rasterBuffers[RS_MATERIAL], samplep, 0));

    // 
    //const vec2 gTexcoord = unpackUnorm2x16(datapass.x);
    const bool isSkybox = uintBitsToFloat(datapass.z) <= 0.f;

    // 
    const uint primitiveID = indices.z;
    const uint geometryInstanceID = indices.y;
    const uint globalInstanceID = indices.x;
    const uint nodeMeshID = getMeshID(rtxInstances[globalInstanceID]);


    const int IdxType = int(meshInfo[nodeMeshID].indexType)-1;
    uvec3 idx3 = uvec3(primitiveID*3u+0u,primitiveID*3u+1u,primitiveID*3u+2u);
    if (IdxType == IndexU8 ) { idx3 = uvec3(load_u8 (idx3.x*1u, 8u, nodeMeshID),load_u32(idx3.y*1u, 8u, nodeMeshID),load_u32(idx3.z*1u, 8u, nodeMeshID)); };
    if (IdxType == IndexU16) { idx3 = uvec3(load_u16(idx3.x*2u, 8u, nodeMeshID),load_u16(idx3.y*2u, 8u, nodeMeshID),load_u16(idx3.z*2u, 8u, nodeMeshID)); };
    if (IdxType == IndexU32) { idx3 = uvec3(load_u32(idx3.x*4u, 8u, nodeMeshID),load_u32(idx3.y*4u, 8u, nodeMeshID),load_u32(idx3.z*4u, 8u, nodeMeshID)); };

    // Interpolate In Ray-Tracing 
    const vec3 baryCoord = texelFetch(rasterBuffers[RS_BARYCENT], samplep, 0).xyz;
    const vec4 gTexcoord = vec4(triangulate(idx3, 1u, nodeMeshID,vec3(1.f-baryCoord.x-baryCoord.y,baryCoord.xy)).xyz,0.f);
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
    const MaterialUnit unit = materials[0u].data[datapass.y]; // NEW! 20.04.2020
    vec4 diffuseColor = texelFetch(rasterBuffers[RS_DIFFUSED], samplep, 0);//
    diffuseColor.w = (toLinear(unit. diffuseTexture >= 0 ? texture(textures[nonuniformEXT(unit. diffuseTexture)],gTexcoord.xy) : unit.diffuse)).w;
    //const vec4 diffuseColor = toLinear(unit. diffuseTexture >= 0 ? texture(textures[nonuniformEXT(unit. diffuseTexture)],gTexcoord.xy) : unit.diffuse);
    if (!isSkybox) { // Only When Opaque!
        result.origin = texelFetch(rasterBuffers[RS_POSITION], samplep, 0);
        result.txcmid = texelFetch(rasterBuffers[RS_MATERIAL], samplep, 0);

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

// 
void main() {
    const uvec2 lanQ = LAUNCH_ID;//gl_LaunchIDEXT.xy;//gl_GlobalInvocationID.xy;
    launchSize = imageSize(writeImages[IW_POSITION]);

    // 
    const ivec2 curPixel = ivec2(lanQ), invPixel = ivec2(curPixel.x,launchSize.y-curPixel.y-1u);
    const ivec2 sizPixel = ivec2(launchSize);

    // WARNING! Quality may critically drop when move! 
    const bool checker = bool(((curPixel.x ^ curPixel.y) ^ (rdata.x))&1u);

    // 
    const Box box = { -1.f.xxx, 1.f.xxx }; // TODO: Change Coordinate
    const vec4 sphere = vec4(vec3(16.f,128.f,16.f), 8.f);
    const vec3 lightc = 32.f*4096.f.xxx/(sphere.w*sphere.w);

    //
    packed = pack32(u16vec2(curPixel)), seed = uvec2(packed, rdata.x);
      const vec2 shift = random2(seed), pixel = vec2(invPixel)+(shift*2.f-1.f)*0.25f+0.5f;
    //const vec2 shift = 0.5f.xx,       pixel = vec2(invPixel)+(shift*2.f-1.f)*0.25f+0.5f;
    
    // 
     vec3 origin = screen2world(vec3((vec2(pixel)/vec2(sizPixel))*2.f-1.f,0.001f));
     vec3 target = screen2world(vec3((vec2(pixel)/vec2(sizPixel))*2.f-1.f,0.999f));
     vec3 raydir = normalize(target - origin);
     vec3 normal = vec3(0.f);
     vec3 geonrm = vec3(0.f);

    // Replacement for rasterization
    //XHIT SUF =                                                       traceRays(    origin.xyz,           (raydir), normal, 10000.f, FAST_BW_TRANSPARENT, 0.01f);
      XHIT SUF =                                                       rasterize(    origin.xyz,           (raydir), normal, 10000.f, FAST_BW_TRANSPARENT, 0.01f);
      XHIT RES = SUF.txcmid.z >=0.99f && SUF.diffuseColor.w <= 0.99f ? traceRays(SUF.origin.xyz, refractive(raydir), normal, 10000.f, false, 0.99f) : SUF; // Ground Deep
    imageStore(writeBuffer[nonuniformEXT(BW_GROUNDPS)], ivec2(lanQ), vec4(SUF.origin.xyz, SUF.geoNormal.w)); XHIT GRD = RES; RES = SUF; // Prefer From TOP layer (like as in Minecraft)
    imageStore(writeImages[nonuniformEXT(IW_INDIRECT)], ivec2(lanQ), RES.geoNormal.w < 9999.f && checker ? vec4(1.f.xxx, 1.f) : vec4(0.f.xxx, 0.f));

    // By Geometry Data
    const uint globalInstanceID = RES.gIndices.y;
    const uint nodeMeshID = getMeshID(rtxInstances[globalInstanceID]);
    const uint geometryInstanceID = RES.gIndices.x;
    mat3x4 matras = mat3x4(instances[nodeMeshID].transform[geometryInstanceID]);
    if (!hasTransform(meshInfo[nodeMeshID])) {
        matras = mat3x4(vec4(1.f,0.f.xxx),vec4(0.f,1.f,0.f.xx),vec4(0.f.xx,1.f,0.f));
    };

    // Initial Position
    vec4 instanceRel = inverse(matras) * inverse(rtxInstances[globalInstanceID].transform) * vec4(RES.origin.xyz,1.f);

    // Problem: NOT enough slots for writables
    // Solution: DON'T use for rasterization after 7th slot, maximize up to 12u slots... 
    //imageStore(writeImages[nonuniformEXT(IW_POSITION)], ivec2(lanQ), vec4(RES.origin .xyz, RES.geoNormal.w));
      imageStore(writeImages[nonuniformEXT(IW_POSITION)], ivec2(lanQ), vec4(instanceRel.xyz, RES.geoNormal.w));
      imageStore(writeImages[nonuniformEXT(IW_GEOMETRY)], ivec2(lanQ), uintBitsToFloat(uvec4(RES.gIndices.xy, SUF.gIndices.xy)));

    // Will Resampled Itself (anchors)
    imageStore(writeImages[nonuniformEXT(IW_GEONORML)], ivec2(lanQ), vec4(RES.geoNormal.xyz, 1.f));
    imageStore(writeImages[nonuniformEXT(IW_MATERIAL)], ivec2(lanQ), vec4(RES.txcmid  ));
    imageStore(writeImages[nonuniformEXT(IW_TRANSPAR)], ivec2(lanQ), vec4(0.f.xxx, 0.f));
    imageStore(writeImages[nonuniformEXT(IW_REFLECLR)], ivec2(lanQ), vec4(0.f.xxx, 1.f)); // Py-Clone
    imageStore(writeImages[nonuniformEXT(IW_MAPNORML)], ivec2(lanQ), vec4(RES.mapNormal.xyz, RES.geoNormal.w < 9999.f ? 1.f : 0.f));


    // Make Visible Color as Anti-Aliased!
    vec4 diffused = vec4(RES.diffuseColor.xyz, 1.f);
    vec4 emission = vec4(RES.emissionColor.xyz, 1.f);
    diffused.xyz = max(diffused.xyz - emission.xyz, 0.f.xxx);
    if (SUF.txcmid.z < 0.99f) { 
        diffused.xyz = 0.f.xxx, emission.xyz = gSkyShader(raydir.xyz, origin.xyz).xyz;
        diffused.xyz += emission.xyz;
    };
    imageStore(writeImages[nonuniformEXT(IW_SMOOTHED)], ivec2(lanQ), vec4(diffused));


    //
    vec4 adaptiveData = 10000.f.xxxx;

    // 
#ifdef RAY_TRACE
    if (!checker) { // TODO: Fix Broken Transparency
        
    } else 
    if ( (RES.diffuseColor.w > 0.001f && RES.geoNormal.w < 9999.f || SUF.diffuseColor.w > 0.001f && SUF.geoNormal.w < 9999.f) && checker ) {
        //vec3 forigin = (origin = RES.origin.xyz), fraydir = raydir, fnormal = RES.mapNormal.xyz;
        //TBN[2] = normalize(faceforward(TBN[2], fraydir.xyz, TBN[2]));

        // 
        const vec4 sphere = vec4(vec3(16.f,128.f,16.f), 8.f);
        const vec4 bspher = vec4(origin,10000.f);
        const float inIOR = 1.f, outIOR = 1.6666f;

        // 
        const uint MAX_ITERATION = 3u;
        for (uint I=0;I<MAX_ITERATION;I++) {
            //if ( I == 2 ) { RES = SUF; }; // Top Layer Overlap
            if (!(RES.diffuseColor.w > 0.001f && RES.geoNormal.w < 9999.f)) { continue; }; // useless tracing mode
            if (  RES.diffuseColor.w > 0.99f  && I == 2 ) { break; }; // still needs shading, except surface transparency

            // 
            mat3 TBN = mat3(RES.gTangent.xyz, RES.gBinormal.xyz, RES.geoNormal.xyz);
            vec3 forigin = (origin = RES.origin.xyz), fraydir = raydir, fnormal = RES.mapNormal.xyz;
            TBN[2] = normalize(faceforward(TBN[2], fraydir.xyz, TBN[2]));

            {
                vec3 raydir = I == 2 ? refractive(fraydir.xyz) : (I == 0 ? randomHemisphereCosine(seed, TBN) : reflectGlossy(seed, fraydir.xyz, TBN, RES.specularColor.y));
                vec3 origin = forigin, normal = normalize(faceforward(fnormal.xyz, raydir.xyz, -TBN[2]));
                //raydir.xyz = normalize(faceforward(-raydir.xyz, raydir.xyz, -TBN[2])); // don't do it more!

                vec4 gEnergy = vec4(1.f.xxxx), gSignal = vec4(0.f.xxx,1.f);

                //if ( I == 2 ) { gEnergy.xyz *= RES.diffuseColor.xyz; };
                if ( I == 1 ) {  }; // TODO: metallic reflection
                if ( I == 0 ) { directLight(sphere, origin, normal, seed, gSignal, gEnergy); };
                //if ( I == 0 || I == 2 ) { directLight(sphere, origin, normal, seed, gSignal, gEnergy); };

                // 
                for (uint i=0;i<2;i++) { // fast trace
                    XHIT result = i == 0 && I == 2 ? GRD : traceRays(origin, raydir, normal, 10000.f, true, 0.01f); // Useless to tracing for transparency...
                    //if ( I == 2 && result.diffuseColor.w > 0.99f ) { break; }; // No needs for transparent layers

                    // 
                    float sdepth = raySphereIntersect(origin.xyz,raydir.xyz,sphere.xyz,sphere.w); sdepth = sdepth <= 0.f ? 10000.f : sdepth;
                    float mvalue = min(result.geoNormal.w, 10000.f);

                    // power of reflection
                    float reflectionPower = mix(clamp(pow(1.0f + dot(raydir.xyz, result.mapNormal.xyz), outIOR/inIOR), 0.f, 1.f) * 0.3333f, 1.f, result.specularColor.z);
                     bool couldReflection = random(seed) <= reflectionPower;

                    // 
                    if ( result.geoNormal.w >= 9999.f ) {
                        const float sdepth = raySphereIntersect(origin.xyz,raydir.xyz,bspher.xyz,bspher.w); mvalue = (sdepth <= 0.f ? 10000.f : sdepth);
                        gSignal.xyz = max(fma(gEnergy.xyz, result.emissionColor.xyz, gSignal.xyz) ,0.f.xxx), gEnergy *= 0.f;
                    } else 
                    if ( result.diffuseColor.w > 0.001f ) {
                        if (couldReflection) {
+                            gEnergy *= vec4(mix(1.f.xxx, result.diffuseColor.xyz, result.specularColor.zzz), 1.f);
                        } else {
                            gSignal.xyz += gEnergy.xyz * result.emissionColor.xyz * result.emissionColor.w;
                            gEnergy *= vec4(max(result.diffuseColor.xyz - clamp(result.emissionColor.xyz*result.emissionColor.w,0.f.xxx,1.f.xxx), 0.f.xxx), 1.f);
                        };
                    } else { // VOID!
                        gEnergy *= vec4(0.f.xxxx);
                    }

                    // 
                    const mat3x3 TBN = mat3x3(result.gTangent.xyz,result.gBinormal.xyz,result.mapNormal.xyz);
                    raydir.xyz = couldReflection ? 
                        reflectGlossy(seed, raydir.xyz, TBN, result.specularColor.y) : 
                        randomHemisphereCosine(seed, TBN);

                    // 
                    //raydir.xyz = normalize(faceforward(raydir.xyz, raydir.xyz, -result.geoNormal.xyz)); // don't do it more!

                    // 
                    normal.xyz = result.mapNormal.xyz = normalize(faceforward(result.mapNormal.xyz, -raydir.xyz, result.geoNormal.xyz));

                    // 
                    origin.xyz = result.origin.xyz;
                    origin.xyz += faceforward(result.geoNormal.xyz,-raydir.xyz,result.geoNormal.xyz) * 0.0001f + raydir.xyz * 0.0001f;

                    // 
                    if ( i == 0 ) { adaptiveData[I] = result.geoNormal.w; }; // length of ray for adaptive denoise
                    if ((dot(gEnergy.xyz,1.f.xxx)/3.f) <= 0.001f || result.geoNormal.w >= 9999.f || dot(raydir.xyz,result.geoNormal.xyz) <= 0.f) { break; }; //

                    // 
                    directLight(sphere, origin, normal, seed, gSignal, gEnergy);
                    
                };

                // 
                { gSignal.xyz = clamp(gSignal.xyz,0.f.xxx,16.f.xxx); };
                if (I == 0) { imageStore(writeImages[nonuniformEXT(IW_INDIRECT)], ivec2(lanQ), vec4(gSignal.xyz, 1.f)); };
                if (I == 1) {
                    imageStore(writeImages[nonuniformEXT(IW_REFLECLR)], ivec2(lanQ), vec4(clamp(gSignal.xyz, 0.f.xxx, 2.f.xxx), 1.f));
                };
                if (I == 2) {
                    imageStore(writeImages[nonuniformEXT(IW_TRANSPAR)], ivec2(lanQ), vec4(gSignal.xyz, SUF.txcmid.z)); // alpha channel reserved, zero always opaque type
                };
            };
        };
    };
#endif

    // Used By Reprojection (comparsion)
    imageStore(writeImages[nonuniformEXT(IW_ADAPTIVE)], ivec2(lanQ), adaptiveData); // For Adaptive Denoise
    imageStore(writeBuffer[nonuniformEXT(BW_INDIRECT)], ivec2(lanQ), imageLoad(writeImages[nonuniformEXT(IW_INDIRECT)], ivec2(lanQ)));
    imageStore(writeBuffer[nonuniformEXT(BW_POSITION)], ivec2(lanQ), vec4(RES.origin.xyz, RES.geoNormal.w)); // Stay The Same... 
    //imageStore(writeBuffer[nonuniformEXT(BW_POSITION)], ivec2(lanQ), imageLoad(writeImages[nonuniformEXT(IW_POSITION)], ivec2(lanQ))); // Broken!
    imageStore(writeBuffer[nonuniformEXT(BW_GEONORML)], ivec2(lanQ), imageLoad(writeImages[nonuniformEXT(IW_GEONORML)], ivec2(lanQ)));
    imageStore(writeBuffer[nonuniformEXT(BW_SMOOTHED)], ivec2(lanQ), imageLoad(writeImages[nonuniformEXT(IW_SMOOTHED)], ivec2(lanQ)));
    imageStore(writeBuffer[nonuniformEXT(BW_MATERIAL)], ivec2(lanQ), imageLoad(writeImages[nonuniformEXT(IW_MATERIAL)], ivec2(lanQ)));
    imageStore(writeBuffer[nonuniformEXT(BW_REFLECLR)], ivec2(lanQ), imageLoad(writeImages[nonuniformEXT(IW_REFLECLR)], ivec2(lanQ)));
    imageStore(writeBuffer[nonuniformEXT(BW_TRANSPAR)], ivec2(lanQ), imageLoad(writeImages[nonuniformEXT(IW_TRANSPAR)], ivec2(lanQ)));
    imageStore(writeBuffer[nonuniformEXT(BW_ADAPTIVE)], ivec2(lanQ), imageLoad(writeImages[nonuniformEXT(IW_ADAPTIVE)], ivec2(lanQ)));
};
