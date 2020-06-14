#define RES hits[lIdx]

// 14.06.2020
// Fully Refresh Ray Cast Shaders
void main() {
    const Box box = { -1.f.xxx, 1.f.xxx }; // TODO: Change Coordinate
    const vec4 sphere = vec4(vec3(16.f,128.f,16.f), 8.f);
    const vec3 lightc = 32.f*4096.f.xxx/(sphere.w*sphere.w);

    // 
    const uvec2 locQs = uvec2(gl_LocalInvocationID.xy);
    const uvec2 locQ = uvec2(locQs.x, (locQs.y<<1u) | ((locQs.x+rdata.x)&1u));
    const uvec2 lanQ = uvec2(gl_WorkGroupID.xy*gl_WorkGroupSize.xy*uvec2(1u,2u) + locQ).xy;
    //const uint lIdx = locQ.y * gl_WorkGroupSize.x + locQ.x;
    const uint lIdx = gl_LocalInvocationIndex;

    // 
    launchSize = imageSize(writeImages[IW_POSITION]);
    subgroupBarrier(); barrier();

    // 
    for (uint Q = 0u; Q < 2u; Q++) {
        const uvec2 locQ = uvec2(locQs.x, Q*gl_WorkGroupSize.y + locQs.y);
        const uvec2 lanQ = uvec2(gl_WorkGroupID.xy*uvec2(gl_WorkGroupSize.xy*uvec2(1u,2u)) + locQ).xy;
        //const uint lIdx = locQ.y * gl_WorkGroupSize.x + locQ.x;
        const uint lIdx = (locQ.y >> 1u) * gl_WorkGroupSize.x + locQ.x;
        
        // 
        const ivec2 curPixel = ivec2(lanQ), invPixel = ivec2(curPixel.x,launchSize.y-curPixel.y-1u);
        const ivec2 sizPixel = ivec2(launchSize);

        // WARNING! Quality may critically drop when move! 
        const bool checker = bool(((curPixel.x ^ curPixel.y) ^ (rdata.x^1))&1u);

        //
        packed = pack32(u16vec2(curPixel)), seed = uvec2(packed, rdata.x);
        const vec2 shift = random2(seed),   pixel = vec2(invPixel)+(shift*2.f-1.f)*0.25f+0.5f;
        //const vec2 shift = 0.5f.xx,       pixel = vec2(invPixel)+(shift*2.f-1.f)*0.25f+0.5f;

        // 
        vec3 origin = screen2world(vec3((vec2(pixel)/vec2(sizPixel))*2.f-1.f,0.001f));
        vec3 target = screen2world(vec3((vec2(pixel)/vec2(sizPixel))*2.f-1.f,0.999f));
        vec3 raydir = normalize(target - origin);
        vec3 normal = vec3(0.f);
        vec3 geonrm = vec3(0.f);

        // Replacement for rasterization
        //XHIT RPM = traceRays(    origin.xyz,           (raydir), normal, 10000.f, FAST_BW_TRANSPARENT, 0.001f);
          XHIT RPM = rasterize(    origin.xyz,           (raydir), normal, 10000.f, FAST_BW_TRANSPARENT, 0.001f);
        if (checker) { RES = RPM; };

        // TODO: Optimize Fetching and Interpolation 
        XGEO GEO = interpolate(RPM);
        XPOL MAT = materialize(RPM, GEO);

        // 
        vec4 gposition = vec4(RPM.origin.xyz, RPM.gBarycentric.w);

        //XHIT RPM = SUF.txcmid.z >=0.99f && SUF.diffuseColor.w <= 0.99f ? traceRays(SUF.origin.xyz, refractive(raydir), normal, 10000.f, false, 0.99f) : SUF; // Ground Deep
        imageStore(writeBuffer[nonuniformEXT(BW_GROUNDPS)], ivec2(lanQ), vec4(RPM.origin.xyz, RPM.gBarycentric.w)); // Prefer From TOP layer (like as in Minecraft)
        imageStore(writeImages[nonuniformEXT(IW_INDIRECT)], ivec2(lanQ), (RPM.gBarycentric.w < 9999.f && checker) ? vec4(1.f.xxx, 1.f) : vec4(0.f.xxx, 0.f));

        // By Geometry Data
        const uint globalInstanceID = RPM.gIndices.y, geometryInstanceID = RPM.gIndices.x;
        const uint nodeMeshID = getMeshID(rtxInstances[globalInstanceID]);
        mat3x4 matras = mat3x4(instances[nodeMeshID].transform[geometryInstanceID]);
        if (!hasTransform(meshInfo[nodeMeshID])) {
            matras = mat3x4(vec4(1.f,0.f.xxx),vec4(0.f,1.f,0.f.xx),vec4(0.f.xx,1.f,0.f));
        };

        // Initial Position
        vec4 instanceRel = inverse(matras) * inverse(rtxInstances[globalInstanceID].transform) * vec4(RPM.origin.xyz,1.f);

        // Problem: NOT enough slots for writables
        // Solution: DON'T use for rasterization after 7th slot, maximize up to 12u slots... 
        //imageStore(writeImages[nonuniformEXT(IW_POSITION)], ivec2(lanQ), vec4(RPM.origin .xyz, RPM.gBarycentric.w));
        imageStore(writeImages[nonuniformEXT(IW_POSITION)], ivec2(lanQ), vec4(instanceRel.xyz, RPM.gBarycentric.w));
        imageStore(writeImages[nonuniformEXT(IW_GEOMETRY)], ivec2(lanQ), uintBitsToFloat(uvec4(RPM.gIndices.xy, RPM.gIndices.xy)));
        imageStore(writeImages[nonuniformEXT(IW_INDIRECT)], ivec2(lanQ), vec4(0.f.xxx, 1.f));

        // Will Resampled Itself (anchors)
        imageStore(writeImages[nonuniformEXT(IW_MATERIAL)], ivec2(lanQ), vec4(MAT.txcmid  ));
        imageStore(writeImages[nonuniformEXT(IW_TRANSPAR)], ivec2(lanQ), vec4(0.f.xxx, 0.f));
        imageStore(writeImages[nonuniformEXT(IW_REFLECLR)], ivec2(lanQ), vec4(0.f.xxx, 1.f)); // Py-Clone

        // 
        imageStore(writeImages[nonuniformEXT(IW_GEONORML)], ivec2(lanQ), vec4(GEO.gNormal.xyz, 1.f));
        imageStore(writeImages[nonuniformEXT(IW_MAPNORML)], ivec2(lanQ), vec4(MAT.mapNormal.xyz, RPM.gBarycentric.w < 9999.f ? 1.f : 0.f));


        // Make Visible Color as Anti-Aliased!
        vec4 diffused = vec4(MAT.diffuseColor.xyz, 1.f);
        vec4 emission = vec4(MAT.emissionColor.xyz, 1.f);
        diffused.xyz = max(diffused.xyz - emission.xyz, 0.f.xxx);
        if (RPM.gBarycentric.w >= 9999.f) {
            diffused.xyz = 0.f.xxx, emission.xyz = gSkyShader(raydir.xyz, origin.xyz).xyz;
            diffused.xyz += emission.xyz;
        };
        imageStore(writeImages[nonuniformEXT(IW_SMOOTHED)], ivec2(lanQ), vec4(diffused));
    }

    subgroupBarrier(); barrier();

    //
#ifdef RAY_TRACE
    XGEO GEO = interpolate(RES);
    XPOL MAT = materialize(RES, GEO);
    vec4 adaptiveData = 10000.f.xxxx;
    if ( (MAT.diffuseColor.w > 0.001f && RES.gBarycentric.w < 9999.f) ) { // 
        
              vec4 origin = vec4(RES.origin.xyz-RES.gBarycentric.w*RES.direct.xyz, 1.f);
        const vec4 bspher = vec4(origin.xyz,10000.f);
        const float inIOR = 1.f, outIOR = 1.6666f;

        // 
        const uint MAX_ITERATION = 3u;
        for (uint I=0;I<MAX_ITERATION;I++) {
            if (!(MAT.diffuseColor.w > 0.001f && RES.gBarycentric.w < 9999.f)) { continue; }; // useless tracing mode
            if (  MAT.diffuseColor.w > 0.99f  && I == 2 ) { break; }; // still needs shading, except surface transparency

            // 
            mat3 TBN = mat3(GEO.gTangent.xyz, GEO.gBinormal.xyz, GEO.gNormal.xyz);
            vec3 forigin = (origin.xyz = RES.origin.xyz), fraydir = RES.direct.xyz, fnormal = MAT.mapNormal.xyz;

            // 
            fnormal = normalize(faceforward(fnormal, fraydir.xyz, TBN[2]));
            TBN[2]  = normalize(faceforward(TBN[2] , fraydir.xyz, TBN[2]));

            // 
            vec3 raydir = I == 2 ? refractive(fraydir.xyz) : (I == 0 ? randomHemisphereCosine(seed, TBN) : reflectGlossy(seed, fraydir.xyz, TBN, MAT.specularColor.y));
            vec3 origin = forigin, normal = normalize(faceforward(fnormal.xyz, -raydir.xyz, TBN[2]));
            vec4 gEnergy = vec4(1.f.xxxx), gSignal = vec4(0.f.xxx,1.f);

            // 
            if ( I == 1 ) {  }; // TODO: metallic reflection
            if ( I == 0 ) { directLight(RES, sphere, origin, fnormal, seed, gSignal, gEnergy); };

            // 
            for (uint i=0;i<2;i++) { // fast trace
                XHIT hit = traceRays(origin, raydir, normal, 10000.f, true, 0.001f);
                XGEO result = interpolate(hit);
                XPOL material = materialize(hit, result);

                // 
                normal.xyz = material.mapNormal.xyz = normalize(faceforward(material.mapNormal.xyz, raydir.xyz, result.gNormal.xyz));

                // 
                float sdepth = raySphereIntersect(origin.xyz,raydir.xyz,sphere.xyz,sphere.w); sdepth = sdepth <= 0.f ? 10000.f : sdepth;
                float mvalue = min(hit.gBarycentric.w, 10000.f);

                // power of reflection
                float reflectionPower = mix(clamp(pow(1.0f + dot(raydir.xyz, material.mapNormal.xyz), outIOR/inIOR), 0.f, 1.f) * 0.3333f, 1.f, material.specularColor.z);
                bool couldReflection = random(seed) <= reflectionPower;

                // 
                if ( hit.gBarycentric.w >= 9999.f ) {
                    const float sdepth = raySphereIntersect(origin.xyz, raydir.xyz,bspher.xyz,bspher.w); mvalue = (sdepth <= 0.f ? 10000.f : sdepth);
                    gSignal.xyz = max(fma(gEnergy.xyz, material.emissionColor.xyz, gSignal.xyz), 0.f.xxx), gEnergy *= 0.f;
                } else 
                if ( material.diffuseColor.w > 0.001f ) {
                    if (couldReflection) {
                        gEnergy *= vec4(mix(1.f.xxx, material.diffuseColor.xyz, material.specularColor.zzz), 1.f);
                    } else {
                        gSignal.xyz += gEnergy.xyz * material.emissionColor.xyz * material.emissionColor.w;
                        gEnergy *= vec4(max(material.diffuseColor.xyz - clamp(material.emissionColor.xyz*material.emissionColor.w,0.f.xxx,1.f.xxx), 0.f.xxx), 1.f);
                    };
                } else { // VOID!
                    gEnergy *= vec4(0.f.xxxx);
                }

                // 
                const mat3x3 TBN = mat3x3(result.gTangent.xyz,result.gBinormal.xyz,material.mapNormal.xyz);
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
            if (I == 0) { imageStore(writeImages[nonuniformEXT(IW_INDIRECT)], ivec2(lanQ), vec4(gSignal.xyz, 1.f)); };
            if (I == 1) { imageStore(writeImages[nonuniformEXT(IW_REFLECLR)], ivec2(lanQ), vec4(clamp(gSignal.xyz, 0.f.xxx, 2.f.xxx), 1.f)); };
            if (I == 2) { imageStore(writeImages[nonuniformEXT(IW_TRANSPAR)], ivec2(lanQ), vec4(gSignal.xyz, RES.gBarycentric.w < 9999.f ? 1.f : 0.f)); }; // alpha channel reserved, zero always opaque type
        
        };
        imageStore(writeImages[nonuniformEXT(IW_ADAPTIVE)], ivec2(lanQ), adaptiveData); // For Adaptive Denoise
    };

#endif

    subgroupBarrier(); barrier();

    // 
    for (uint Q = 0u; Q < 2u; Q++) {
        const uvec2 locQ = uvec2(locQs.x, Q*gl_WorkGroupSize.y + locQs.y);
        const uvec2 lanQ = uvec2(gl_WorkGroupID.xy*uvec2(gl_WorkGroupSize.xy*uvec2(1u,2u)) + locQ).xy;
        //const uint lIdx = locQ.y * gl_WorkGroupSize.x + locQ.x;
        const uint lIdx = (locQ.y >> 1u) * gl_WorkGroupSize.x + locQ.x;

        // 
        vec4 gposition = vec4(RES.origin.xyz, RES.gBarycentric.w);

        // Used By Reprojection (comparsion)
        imageStore(writeBuffer[nonuniformEXT(BW_INDIRECT)], ivec2(lanQ), imageLoad(writeImages[nonuniformEXT(IW_INDIRECT)], ivec2(lanQ)));
        imageStore(writeBuffer[nonuniformEXT(BW_POSITION)], ivec2(lanQ), gposition); // Stay The Same...
        imageStore(writeBuffer[nonuniformEXT(BW_GEONORML)], ivec2(lanQ), imageLoad(writeImages[nonuniformEXT(IW_GEONORML)], ivec2(lanQ)));
        imageStore(writeBuffer[nonuniformEXT(BW_SMOOTHED)], ivec2(lanQ), imageLoad(writeImages[nonuniformEXT(IW_SMOOTHED)], ivec2(lanQ)));
        imageStore(writeBuffer[nonuniformEXT(BW_MATERIAL)], ivec2(lanQ), imageLoad(writeImages[nonuniformEXT(IW_MATERIAL)], ivec2(lanQ)));
        imageStore(writeBuffer[nonuniformEXT(BW_REFLECLR)], ivec2(lanQ), imageLoad(writeImages[nonuniformEXT(IW_REFLECLR)], ivec2(lanQ)));
        imageStore(writeBuffer[nonuniformEXT(BW_TRANSPAR)], ivec2(lanQ), imageLoad(writeImages[nonuniformEXT(IW_TRANSPAR)], ivec2(lanQ)));
        imageStore(writeBuffer[nonuniformEXT(BW_ADAPTIVE)], ivec2(lanQ), imageLoad(writeImages[nonuniformEXT(IW_ADAPTIVE)], ivec2(lanQ)));
    }
};
