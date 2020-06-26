// 
void directLight(in XHIT RES, in vec4 sphere, in vec3 origin, in vec3 normal, inout uvec2 seed, inout vec4 gSignal, inout vec4 gEnergy) {
    const vec3 lightp = sphere.xyz + randomSphere(seed) * sphere.w; float shdist = distance(lightp.xyz,origin.xyz);
    const vec3 lightd = normalize(lightp.xyz - origin.xyz);
    const vec3 lightc = 32.f*4096.f.xxx/(sphere.w*sphere.w);

    if ( dot(normal, lightd) >= 0.f ) {
        float sdepth = raySphereIntersect(origin.xyz,lightd,sphere.xyz,sphere.w);
        XHIT result = traceRays(origin, lightd, normal, sdepth = sdepth <= 0.f ? 10000.f : sdepth, true, 0.01f);

        if ( min(sdepth, result.gBarycentric.w) >= sdepth ) { // If intersects with light
            const float cos_a_max = sqrt(1.f - clamp(sphere.w * sphere.w / dot(sphere.xyz-origin.xyz, sphere.xyz-origin.xyz), 0.f, 1.f));
            gSignal += vec4(gEnergy.xyz * 2.f * (1.f - cos_a_max) * clamp(dot( lightd, normal.xyz ), 0.f, 1.f) * lightc, 0.f);
        };
    };
};
