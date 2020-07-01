#version 460
#extension GL_EXT_shader_explicit_arithmetic_types_int8 : require
#extension GL_EXT_shader_8bit_storage : require
#extension GL_EXT_nonuniform_qualifier : require
#extension GL_EXT_scalar_block_layout : require
layout(local_size_x = 32, local_size_y = 24, local_size_z = 1) in;

struct MaterialUnit
{
    vec4 diffuse;
    vec4 specular;
    vec4 normals;
    vec4 emission;
    int diffuseTexture;
    int specularTexture;
    int normalsTexture;
    int emissionTexture;
    uvec4 udata;
};

struct Binding
{
    uint binding;
    uint stride;
    uint rate;
};

struct Attribute
{
    uint location;
    uint binding;
    uint format;
    uint offset;
};

struct MeshInfo
{
    uint materialID;
    uint indexType;
    uint primitiveCount;
    uint flags;
};

struct RTXInstance
{
    mat3x4 transform;
    uint instance_mask;
    uint offset_flags;
    uvec2 handle;
};

layout(set = 1, binding = 9, std140) uniform Matrices
{
    mat4 projection;
    mat4 projectionInv;
    mat3x4 modelview;
    mat3x4 modelviewInv;
    mat3x4 modelviewPrev;
    mat3x4 modelviewPrevInv;
    uvec4 mdata;
    uvec2 tdata;
    uvec2 rdata;
} _167;

layout(set = 4, binding = 20, std430) readonly buffer Materials
{
    MaterialUnit materials[];
} _565;

layout(set = 0, binding = 0, std430) readonly buffer MeshData
{
    uint8_t data[];
} mesh0[];

layout(set = 0, binding = 2, std430) readonly buffer MapData
{
    uint data[];
} map;

layout(set = 1, binding = 5, scalar) uniform Bindings
{
    Binding data[8];
} bindings[];

layout(set = 1, binding = 6, std140) uniform Attributes
{
    Attribute data[8];
} attributes[];

layout(set = 1, binding = 7, std140) uniform InstanceTransform
{
    mat3x4 transform[16];
} instances[1];

layout(set = 1, binding = 8, scalar) uniform MeshMaterial
{
    uint materialID[16];
} geomMTs[1];

layout(set = 1, binding = 10, std430) readonly buffer MeshInfoData
{
    MeshInfo meshInfo[];
} _971;

layout(set = 1, binding = 11, std430) readonly buffer RTXInstances
{
    RTXInstance rtxInstances[];
} _976;

layout(push_constant, std430) uniform pushConstants
{
    uvec4 data;
} drawInfo;

layout(set = 4, binding = 21) uniform texture2D background;
layout(set = 2, binding = 14) uniform sampler samplers[4];
layout(set = 3, binding = 16, rgba32f) uniform image2D writeBuffer[11];
layout(set = 3, binding = 17, rgba32f) uniform image2D writeImages[];
layout(set = 4, binding = 22) uniform texture2D textures[];
layout(set = 3, binding = 18, rgba32f) uniform writeonly image2D writeImagesBack[];
layout(set = 0, binding = 3, r32ui) uniform readonly uimage2D mapImage[1];
layout(set = 0, binding = 4) uniform texture2D mapColor[1];
layout(set = 1, binding = 12) uniform accelerationStructureNV Scene;
layout(set = 2, binding = 13) uniform texture2D frameBuffers[12];
layout(set = 2, binding = 15) uniform texture2D rasterBuffers[8];

uint counter;
uint SCLOCK;

vec4 getData(ivec2 map_1)
{
    ivec2 size = imageSize(writeImages[4]);
    return imageLoad(writeImages[4], ivec2(map_1.x, map_1.y));
}

vec4 getPosition(ivec2 map_1)
{
    ivec2 size = imageSize(writeBuffer[10]);
    return imageLoad(writeBuffer[10], ivec2(map_1.x, map_1.y));
}

vec3 toLinear(vec3 sRGB)
{
    return mix(pow((sRGB + vec3(0.054999999701976776123046875)) / vec3(1.05499994754791259765625), vec3(2.400000095367431640625)), sRGB / vec3(12.9200000762939453125), lessThan(sRGB, vec3(0.040449999272823333740234375)));
}

vec4 toLinear(vec4 sRGB)
{
    vec3 param = sRGB.xyz;
    return vec4(toLinear(param), sRGB.w);
}

vec4 getNormal(ivec2 map_1)
{
    ivec2 size = imageSize(writeImages[9]);
    return imageLoad(writeImages[9], ivec2(map_1.x, map_1.y));
}

vec2 lcts(vec3 direct)
{
    return vec2(fma(atan(direct.z, direct.x), 0.15915493667125701904296875, 0.5), acos(-direct.y) * 0.3183098733425140380859375);
}

vec2 flip(vec2 texcoord)
{
    return vec2(texcoord.x, 1.0 - texcoord.y);
}

vec4 gSkyShader(vec3 raydir, vec3 origin)
{
    vec3 param = raydir;
    vec2 param_1 = lcts(param);
    return vec4(textureLod(sampler2D(background, samplers[3]), flip(param_1), 0.0).xyz, 1.0);
}

vec3 divW(vec4 vect)
{
    return vect.xyz / vec3(vect.w);
}

vec3 world2screen(vec3 origin)
{
    vec4 param = vec4(vec4(origin, 1.0) * _167.modelview, 1.0) * _167.projection;
    return divW(param);
}

vec4 getIndirect(ivec2 map_1)
{
    ivec2 size = imageSize(writeImages[0]);
    return imageLoad(writeImages[0], ivec2(map_1.x, map_1.y));
}

vec4 getPReflection(ivec2 map_1)
{
    ivec2 size = imageSize(writeImages[2]);
    return imageLoad(writeImages[2], ivec2(map_1.x, map_1.y));
}

vec4 getTransparent(ivec2 map_1)
{
    ivec2 size = imageSize(writeBuffer[3]);
    return imageLoad(writeBuffer[3], ivec2(map_1.x, map_1.y));
}

vec4 getDenoised(ivec2 coord, int type, uint maxc)
{
    ivec2 param = coord;
    vec4 centerNormal = getNormal(param);
    ivec2 param_1 = coord;
    vec3 param_2 = getPosition(param_1).xyz;
    vec3 centerOrigin = world2screen(param_2);
    vec4 sampled = vec4(0.0);
    int scount = 0;
    ivec2 param_3 = coord;
    vec4 centerc = getIndirect(param_3);
    for (uint x = 0u; x < maxc; x++)
    {
        for (uint y = 0u; y < maxc; y++)
        {
            ivec2 map_1 = coord + ivec2(int(x - (maxc >> uint(1))), int(y - (maxc >> uint(1))));
            ivec2 param_4 = map_1;
            vec4 nsample = getNormal(param_4);
            ivec2 param_5 = map_1;
            vec3 param_6 = getPosition(param_5).xyz;
            vec4 psample = vec4(world2screen(param_6), 1.0);
            bool _407 = dot(nsample.xyz, centerNormal.xyz) >= 0.5;
            bool _416;
            if (_407)
            {
                _416 = distance(psample.xyz, centerOrigin) < 0.00999999977648258209228515625;
            }
            else
            {
                _416 = _407;
            }
            bool _427;
            if (_416)
            {
                _427 = abs(centerOrigin.z - psample.z) < 0.004999999888241291046142578125;
            }
            else
            {
                _427 = _416;
            }
            bool _442;
            if (!_427)
            {
                bool _434 = x == (maxc >> uint(1));
                bool _441;
                if (_434)
                {
                    _441 = y == (maxc >> uint(1));
                }
                else
                {
                    _441 = _434;
                }
                _442 = _441;
            }
            else
            {
                _442 = _427;
            }
            bool _456;
            if (!_442)
            {
                bool _449 = centerc.w <= 9.9999997473787516355514526367188e-05;
                bool _455;
                if (_449)
                {
                    _455 = sampled.w <= 9.9999997473787516355514526367188e-05;
                }
                else
                {
                    _455 = _449;
                }
                _456 = _455;
            }
            else
            {
                _456 = _442;
            }
            if (_456)
            {
                vec4 samp = vec4(0.0);
                float simp = 1.0;
                if (type == 0)
                {
                    ivec2 param_7 = map_1;
                    samp = getIndirect(param_7);
                    simp = samp.w;
                }
                if (type == 1)
                {
                    ivec2 param_8 = map_1;
                    samp = getPReflection(param_8);
                    simp = samp.w;
                }
                if (type == 2)
                {
                    ivec2 param_9 = map_1;
                    samp = getTransparent(param_9);
                    simp = samp.w;
                }
                vec3 _506 = clamp(samp.xyz / vec3(max(samp.w, 0.5)), vec3(0.0), (type == 2) ? vec3(2.0) : vec3(16.0)) * samp.w;
                samp = vec4(_506.x, _506.y, _506.z, samp.w);
                samp.w = simp;
                sampled += samp;
            }
        }
    }
    sampled.w = max(sampled.w, 1.0);
    return sampled;
}

vec4 getReflection(ivec2 map_1)
{
    ivec2 size = imageSize(writeBuffer[2]);
    return imageLoad(writeBuffer[2], ivec2(map_1.x, map_1.y));
}

ivec2 mapc(ivec2 map_1)
{
    ivec2 size = imageSize(writeImages[2]);
    return ivec2(map_1.x, map_1.y);
}

void main()
{
    counter = 0u;
    SCLOCK = 0u;
    ivec2 size = imageSize(writeImages[0]);
    ivec2 samplep = ivec2(gl_GlobalInvocationID.xy);
    ivec2 param = samplep;
    vec4 dataflat = getData(param);
    uvec4 datapass = floatBitsToUint(dataflat);
    ivec2 param_1 = samplep;
    vec4 position = getPosition(param_1);
    vec2 texcoord = unpackUnorm2x16(datapass.x);
    bool isSkybox = uintBitsToFloat(datapass.z) <= 0.0;
    MaterialUnit _571;
    _571.diffuse = _565.materials[datapass.y].diffuse;
    _571.specular = _565.materials[datapass.y].specular;
    _571.normals = _565.materials[datapass.y].normals;
    _571.emission = _565.materials[datapass.y].emission;
    _571.diffuseTexture = _565.materials[datapass.y].diffuseTexture;
    _571.specularTexture = _565.materials[datapass.y].specularTexture;
    _571.normalsTexture = _565.materials[datapass.y].normalsTexture;
    _571.emissionTexture = _565.materials[datapass.y].emissionTexture;
    _571.udata = _565.materials[datapass.y].udata;
    MaterialUnit unit = _571;
    vec4 _576;
    if (unit.diffuseTexture >= 0)
    {
        int _584 = unit.diffuseTexture;
        _576 = textureLod(sampler2D(textures[nonuniformEXT(_584)], samplers[2]), texcoord, 0.0);
    }
    else
    {
        _576 = unit.diffuse;
    }
    vec4 param_2 = _576;
    vec4 diffused = toLinear(param_2);
    vec4 _603;
    if (unit.emissionTexture >= 0)
    {
        int _608 = unit.emissionTexture;
        _603 = textureLod(sampler2D(textures[nonuniformEXT(_608)], samplers[2]), texcoord, 0.0);
    }
    else
    {
        _603 = unit.emission;
    }
    vec4 param_3 = _603;
    vec4 emission = toLinear(param_3);
    vec4 _627;
    if (unit.normalsTexture >= 0)
    {
        int _632 = unit.normalsTexture;
        _627 = textureLod(sampler2D(textures[nonuniformEXT(_632)], samplers[2]), texcoord, 0.0);
    }
    else
    {
        _627 = unit.normals;
    }
    vec4 normaled = _627;
    vec4 _649;
    if (unit.specularTexture >= 0)
    {
        int _654 = unit.specularTexture;
        _649 = textureLod(sampler2D(textures[nonuniformEXT(_654)], samplers[2]), texcoord, 0.0);
    }
    else
    {
        _649 = unit.specular;
    }
    vec4 specular = _649;
    vec4 dtexdata = diffused;
    vec3 camera = vec4(position.xyz, 1.0) * _167.modelview;
    vec3 raydir = (_167.modelview * normalize(camera)).xyz;
    ivec2 param_4 = samplep;
    vec3 origin = getPosition(param_4).xyz;
    ivec2 param_5 = samplep;
    vec3 normal = getNormal(param_5).xyz;
    vec3 _700 = max(diffused.xyz - emission.xyz, vec3(0.0));
    diffused = vec4(_700.x, _700.y, _700.z, diffused.w);
    diffused.w = dtexdata.w;
    if (isSkybox)
    {
        diffused = vec4(vec3(0.0).x, vec3(0.0).y, vec3(0.0).z, diffused.w);
        vec3 param_6 = raydir;
        vec3 param_7 = origin;
        vec3 _716 = gSkyShader(param_6, param_7).xyz;
        emission = vec4(_716.x, _716.y, _716.z, emission.w);
    }
    vec4 reflects = vec4(0.0);
    ivec2 param_8 = samplep;
    int param_9 = 0;
    uint param_10 = 9u;
    vec4 coloring = getDenoised(param_8, param_9, param_10);
    if (reflects.w <= 0.0)
    {
        reflects = vec4(0.0);
    }
    if (coloring.w <= 0.0)
    {
        coloring = vec4(0.0, 0.0, 0.0, 1.0);
    }
    coloring = max(coloring, vec4(0.0));
    reflects = max(reflects, vec4(0.0));
    float frefl = mix(clamp(pow(1.0 + dot(raydir, normal), 1.6665999889373779296875), 0.0, 1.0) * 0.33329999446868896484375, 1.0, specular.z) * (isSkybox ? 0.0 : 1.0);
    ivec2 param_11 = samplep;
    vec4 currentReflection = getReflection(param_11);
    ivec2 param_12 = samplep;
    vec4 previousReflection = getPReflection(param_12);
    previousReflection /= vec4(max(previousReflection.w, 1.0));
    currentReflection /= vec4(max(currentReflection.w, 1.0));
    ivec2 param_13 = samplep;
    int param_14 = 2;
    uint param_15 = 3u;
    vec4 transpar = getDenoised(param_13, param_14, param_15);
    vec3 _790 = transpar.xyz / vec3(max(transpar.w, 0.5));
    transpar = vec4(_790.x, _790.y, _790.z, transpar.w);
    transpar.w = float(diffused.w <= 0.99989998340606689453125);
    float alpha = clamp((transpar.w * (1.0 - diffused.w)) * (isSkybox ? 0.0 : 1.0), 0.0, 1.0);
    ivec2 param_16 = samplep;
    vec4 _840 = vec4(clamp(mix(previousReflection.xyz / vec3(max(previousReflection.w, 1.0)), currentReflection.xyz / vec3(max(currentReflection.w, 1.0)), vec3(1.0 - (specular.y * 0.5))), vec3(0.0), vec3(1.0)), 1.0);
    reflects = _840;
    imageStore(writeImages[2], mapc(param_16), _840);
    imageStore(writeBuffer[8], samplep, vec4(((coloring.xyz / vec3(max(coloring.w, 1.0))) * diffused.xyz) + max(emission.xyz, vec3(0.0)), 1.0));
    for (uint I = 0u; I < 12u; I++)
    {
        ivec2 param_17 = samplep;
        ivec2 param_18 = samplep;
        imageStore(writeImagesBack[I], mapc(param_17), imageLoad(writeImages[I], mapc(param_18)));
    }
    bool _891 = !isSkybox;
    bool _899;
    if (_891)
    {
        ivec2 param_19 = samplep;
        _899 = getTransparent(param_19).w > 0.0;
    }
    else
    {
        _899 = _891;
    }
    if (_899)
    {
        ivec2 param_20 = samplep;
        float scount = max(imageLoad(writeImages[0], mapc(param_20)).w, 1.0);
        ivec2 param_21 = samplep;
        imageStore(writeImagesBack[0], mapc(param_21), vec4(diffused.xyz * scount, scount));
    }
}

#version 460
#extension GL_EXT_shader_explicit_arithmetic_types_int8 : require
#extension GL_EXT_shader_8bit_storage : require
#extension GL_EXT_nonuniform_qualifier : require
#extension GL_EXT_scalar_block_layout : require
layout(local_size_x = 32, local_size_y = 24, local_size_z = 1) in;

struct MaterialUnit
{
    vec4 diffuse;
    vec4 specular;
    vec4 normals;
    vec4 emission;
    int diffuseTexture;
    int specularTexture;
    int normalsTexture;
    int emissionTexture;
    uvec4 udata;
};

struct Binding
{
    uint binding;
    uint stride;
    uint rate;
};

struct Attribute
{
    uint location;
    uint binding;
    uint format;
    uint offset;
};

struct MeshInfo
{
    uint materialID;
    uint indexType;
    uint primitiveCount;
    uint flags;
};

struct RTXInstance
{
    mat3x4 transform;
    uint instance_mask;
    uint offset_flags;
    uvec2 handle;
};

layout(set = 1, binding = 9, std140) uniform Matrices
{
    mat4 projection;
    mat4 projectionInv;
    mat3x4 modelview;
    mat3x4 modelviewInv;
    mat3x4 modelviewPrev;
    mat3x4 modelviewPrevInv;
    uvec4 mdata;
    uvec2 tdata;
    uvec2 rdata;
} _167;

layout(set = 4, binding = 20, std430) readonly buffer Materials
{
    MaterialUnit materials[];
} _565;

layout(set = 0, binding = 0, std430) readonly buffer MeshData
{
    uint8_t data[];
} mesh0[];

layout(set = 0, binding = 2, std430) readonly buffer MapData
{
    uint data[];
} map;

layout(set = 1, binding = 5, scalar) uniform Bindings
{
    Binding data[8];
} bindings[];

layout(set = 1, binding = 6, std140) uniform Attributes
{
    Attribute data[8];
} attributes[];

layout(set = 1, binding = 7, std140) uniform InstanceTransform
{
    mat3x4 transform[16];
} instances[1];

layout(set = 1, binding = 8, scalar) uniform MeshMaterial
{
    uint materialID[16];
} geomMTs[1];

layout(set = 1, binding = 10, std430) readonly buffer MeshInfoData
{
    MeshInfo meshInfo[];
} _971;

layout(set = 1, binding = 11, std430) readonly buffer RTXInstances
{
    RTXInstance rtxInstances[];
} _976;

layout(push_constant, std430) uniform pushConstants
{
    uvec4 data;
} drawInfo;

layout(set = 4, binding = 21) uniform texture2D background;
layout(set = 2, binding = 14) uniform sampler samplers[4];
layout(set = 3, binding = 16, rgba32f) uniform image2D writeBuffer[11];
layout(set = 3, binding = 17, rgba32f) uniform image2D writeImages[];
layout(set = 4, binding = 22) uniform texture2D textures[];
layout(set = 3, binding = 18, rgba32f) uniform writeonly image2D writeImagesBack[];
layout(set = 0, binding = 3, r32ui) uniform readonly uimage2D mapImage[1];
layout(set = 0, binding = 4) uniform texture2D mapColor[1];
layout(set = 1, binding = 12) uniform accelerationStructureNV Scene;
layout(set = 2, binding = 13) uniform texture2D frameBuffers[12];
layout(set = 2, binding = 15) uniform texture2D rasterBuffers[8];

uint counter;
uint SCLOCK;

vec4 getData(ivec2 map_1)
{
    ivec2 size = imageSize(writeImages[4]);
    return imageLoad(writeImages[4], ivec2(map_1.x, map_1.y));
}

vec4 getPosition(ivec2 map_1)
{
    ivec2 size = imageSize(writeBuffer[10]);
    return imageLoad(writeBuffer[10], ivec2(map_1.x, map_1.y));
}

vec3 toLinear(vec3 sRGB)
{
    return mix(pow((sRGB + vec3(0.054999999701976776123046875)) / vec3(1.05499994754791259765625), vec3(2.400000095367431640625)), sRGB / vec3(12.9200000762939453125), lessThan(sRGB, vec3(0.040449999272823333740234375)));
}

vec4 toLinear(vec4 sRGB)
{
    vec3 param = sRGB.xyz;
    return vec4(toLinear(param), sRGB.w);
}

vec4 getNormal(ivec2 map_1)
{
    ivec2 size = imageSize(writeImages[9]);
    return imageLoad(writeImages[9], ivec2(map_1.x, map_1.y));
}

vec2 lcts(vec3 direct)
{
    return vec2(fma(atan(direct.z, direct.x), 0.15915493667125701904296875, 0.5), acos(-direct.y) * 0.3183098733425140380859375);
}

vec2 flip(vec2 texcoord)
{
    return vec2(texcoord.x, 1.0 - texcoord.y);
}

vec4 gSkyShader(vec3 raydir, vec3 origin)
{
    vec3 param = raydir;
    vec2 param_1 = lcts(param);
    return vec4(textureLod(sampler2D(background, samplers[3]), flip(param_1), 0.0).xyz, 1.0);
}

vec3 divW(vec4 vect)
{
    return vect.xyz / vec3(vect.w);
}

vec3 world2screen(vec3 origin)
{
    vec4 param = vec4(vec4(origin, 1.0) * _167.modelview, 1.0) * _167.projection;
    return divW(param);
}

vec4 getIndirect(ivec2 map_1)
{
    ivec2 size = imageSize(writeImages[0]);
    return imageLoad(writeImages[0], ivec2(map_1.x, map_1.y));
}

vec4 getPReflection(ivec2 map_1)
{
    ivec2 size = imageSize(writeImages[2]);
    return imageLoad(writeImages[2], ivec2(map_1.x, map_1.y));
}

vec4 getTransparent(ivec2 map_1)
{
    ivec2 size = imageSize(writeBuffer[3]);
    return imageLoad(writeBuffer[3], ivec2(map_1.x, map_1.y));
}

vec4 getDenoised(ivec2 coord, int type, uint maxc)
{
    ivec2 param = coord;
    vec4 centerNormal = getNormal(param);
    ivec2 param_1 = coord;
    vec3 param_2 = getPosition(param_1).xyz;
    vec3 centerOrigin = world2screen(param_2);
    vec4 sampled = vec4(0.0);
    int scount = 0;
    ivec2 param_3 = coord;
    vec4 centerc = getIndirect(param_3);
    for (uint x = 0u; x < maxc; x++)
    {
        for (uint y = 0u; y < maxc; y++)
        {
            ivec2 map_1 = coord + ivec2(int(x - (maxc >> uint(1))), int(y - (maxc >> uint(1))));
            ivec2 param_4 = map_1;
            vec4 nsample = getNormal(param_4);
            ivec2 param_5 = map_1;
            vec3 param_6 = getPosition(param_5).xyz;
            vec4 psample = vec4(world2screen(param_6), 1.0);
            bool _407 = dot(nsample.xyz, centerNormal.xyz) >= 0.5;
            bool _416;
            if (_407)
            {
                _416 = distance(psample.xyz, centerOrigin) < 0.00999999977648258209228515625;
            }
            else
            {
                _416 = _407;
            }
            bool _427;
            if (_416)
            {
                _427 = abs(centerOrigin.z - psample.z) < 0.004999999888241291046142578125;
            }
            else
            {
                _427 = _416;
            }
            bool _442;
            if (!_427)
            {
                bool _434 = x == (maxc >> uint(1));
                bool _441;
                if (_434)
                {
                    _441 = y == (maxc >> uint(1));
                }
                else
                {
                    _441 = _434;
                }
                _442 = _441;
            }
            else
            {
                _442 = _427;
            }
            bool _456;
            if (!_442)
            {
                bool _449 = centerc.w <= 9.9999997473787516355514526367188e-05;
                bool _455;
                if (_449)
                {
                    _455 = sampled.w <= 9.9999997473787516355514526367188e-05;
                }
                else
                {
                    _455 = _449;
                }
                _456 = _455;
            }
            else
            {
                _456 = _442;
            }
            if (_456)
            {
                vec4 samp = vec4(0.0);
                float simp = 1.0;
                if (type == 0)
                {
                    ivec2 param_7 = map_1;
                    samp = getIndirect(param_7);
                    simp = samp.w;
                }
                if (type == 1)
                {
                    ivec2 param_8 = map_1;
                    samp = getPReflection(param_8);
                    simp = samp.w;
                }
                if (type == 2)
                {
                    ivec2 param_9 = map_1;
                    samp = getTransparent(param_9);
                    simp = samp.w;
                }
                vec3 _506 = clamp(samp.xyz / vec3(max(samp.w, 0.5)), vec3(0.0), (type == 2) ? vec3(2.0) : vec3(16.0)) * samp.w;
                samp = vec4(_506.x, _506.y, _506.z, samp.w);
                samp.w = simp;
                sampled += samp;
            }
        }
    }
    sampled.w = max(sampled.w, 1.0);
    return sampled;
}

vec4 getReflection(ivec2 map_1)
{
    ivec2 size = imageSize(writeBuffer[2]);
    return imageLoad(writeBuffer[2], ivec2(map_1.x, map_1.y));
}

ivec2 mapc(ivec2 map_1)
{
    ivec2 size = imageSize(writeImages[2]);
    return ivec2(map_1.x, map_1.y);
}

void main()
{
    counter = 0u;
    SCLOCK = 0u;
    ivec2 size = imageSize(writeImages[0]);
    ivec2 samplep = ivec2(gl_GlobalInvocationID.xy);
    ivec2 param = samplep;
    vec4 dataflat = getData(param);
    uvec4 datapass = floatBitsToUint(dataflat);
    ivec2 param_1 = samplep;
    vec4 position = getPosition(param_1);
    vec2 texcoord = unpackUnorm2x16(datapass.x);
    bool isSkybox = uintBitsToFloat(datapass.z) <= 0.0;
    MaterialUnit _571;
    _571.diffuse = _565.materials[datapass.y].diffuse;
    _571.specular = _565.materials[datapass.y].specular;
    _571.normals = _565.materials[datapass.y].normals;
    _571.emission = _565.materials[datapass.y].emission;
    _571.diffuseTexture = _565.materials[datapass.y].diffuseTexture;
    _571.specularTexture = _565.materials[datapass.y].specularTexture;
    _571.normalsTexture = _565.materials[datapass.y].normalsTexture;
    _571.emissionTexture = _565.materials[datapass.y].emissionTexture;
    _571.udata = _565.materials[datapass.y].udata;
    MaterialUnit unit = _571;
    vec4 _576;
    if (unit.diffuseTexture >= 0)
    {
        int _584 = unit.diffuseTexture;
        _576 = textureLod(sampler2D(textures[nonuniformEXT(_584)], samplers[2]), texcoord, 0.0);
    }
    else
    {
        _576 = unit.diffuse;
    }
    vec4 param_2 = _576;
    vec4 diffused = toLinear(param_2);
    vec4 _603;
    if (unit.emissionTexture >= 0)
    {
        int _608 = unit.emissionTexture;
        _603 = textureLod(sampler2D(textures[nonuniformEXT(_608)], samplers[2]), texcoord, 0.0);
    }
    else
    {
        _603 = unit.emission;
    }
    vec4 param_3 = _603;
    vec4 emission = toLinear(param_3);
    vec4 _627;
    if (unit.normalsTexture >= 0)
    {
        int _632 = unit.normalsTexture;
        _627 = textureLod(sampler2D(textures[nonuniformEXT(_632)], samplers[2]), texcoord, 0.0);
    }
    else
    {
        _627 = unit.normals;
    }
    vec4 normaled = _627;
    vec4 _649;
    if (unit.specularTexture >= 0)
    {
        int _654 = unit.specularTexture;
        _649 = textureLod(sampler2D(textures[nonuniformEXT(_654)], samplers[2]), texcoord, 0.0);
    }
    else
    {
        _649 = unit.specular;
    }
    vec4 specular = _649;
    vec4 dtexdata = diffused;
    vec3 camera = vec4(position.xyz, 1.0) * _167.modelview;
    vec3 raydir = (_167.modelview * normalize(camera)).xyz;
    ivec2 param_4 = samplep;
    vec3 origin = getPosition(param_4).xyz;
    ivec2 param_5 = samplep;
    vec3 normal = getNormal(param_5).xyz;
    vec3 _700 = max(diffused.xyz - emission.xyz, vec3(0.0));
    diffused = vec4(_700.x, _700.y, _700.z, diffused.w);
    diffused.w = dtexdata.w;
    if (isSkybox)
    {
        diffused = vec4(vec3(0.0).x, vec3(0.0).y, vec3(0.0).z, diffused.w);
        vec3 param_6 = raydir;
        vec3 param_7 = origin;
        vec3 _716 = gSkyShader(param_6, param_7).xyz;
        emission = vec4(_716.x, _716.y, _716.z, emission.w);
    }
    vec4 reflects = vec4(0.0);
    ivec2 param_8 = samplep;
    int param_9 = 0;
    uint param_10 = 9u;
    vec4 coloring = getDenoised(param_8, param_9, param_10);
    if (reflects.w <= 0.0)
    {
        reflects = vec4(0.0);
    }
    if (coloring.w <= 0.0)
    {
        coloring = vec4(0.0, 0.0, 0.0, 1.0);
    }
    coloring = max(coloring, vec4(0.0));
    reflects = max(reflects, vec4(0.0));
    float frefl = mix(clamp(pow(1.0 + dot(raydir, normal), 1.6665999889373779296875), 0.0, 1.0) * 0.33329999446868896484375, 1.0, specular.z) * (isSkybox ? 0.0 : 1.0);
    ivec2 param_11 = samplep;
    vec4 currentReflection = getReflection(param_11);
    ivec2 param_12 = samplep;
    vec4 previousReflection = getPReflection(param_12);
    previousReflection /= vec4(max(previousReflection.w, 1.0));
    currentReflection /= vec4(max(currentReflection.w, 1.0));
    ivec2 param_13 = samplep;
    int param_14 = 2;
    uint param_15 = 3u;
    vec4 transpar = getDenoised(param_13, param_14, param_15);
    vec3 _790 = transpar.xyz / vec3(max(transpar.w, 0.5));
    transpar = vec4(_790.x, _790.y, _790.z, transpar.w);
    transpar.w = float(diffused.w <= 0.99989998340606689453125);
    float alpha = clamp((transpar.w * (1.0 - diffused.w)) * (isSkybox ? 0.0 : 1.0), 0.0, 1.0);
    ivec2 param_16 = samplep;
    vec4 _840 = vec4(clamp(mix(previousReflection.xyz / vec3(max(previousReflection.w, 1.0)), currentReflection.xyz / vec3(max(currentReflection.w, 1.0)), vec3(1.0 - (specular.y * 0.5))), vec3(0.0), vec3(1.0)), 1.0);
    reflects = _840;
    imageStore(writeImages[2], mapc(param_16), _840);
    imageStore(writeBuffer[8], samplep, vec4(((coloring.xyz / vec3(max(coloring.w, 1.0))) * diffused.xyz) + max(emission.xyz, vec3(0.0)), 1.0));
    for (uint I = 0u; I < 12u; I++)
    {
        ivec2 param_17 = samplep;
        ivec2 param_18 = samplep;
        imageStore(writeImagesBack[I], mapc(param_17), imageLoad(writeImages[I], mapc(param_18)));
    }
    bool _891 = !isSkybox;
    bool _899;
    if (_891)
    {
        ivec2 param_19 = samplep;
        _899 = getTransparent(param_19).w > 0.0;
    }
    else
    {
        _899 = _891;
    }
    if (_899)
    {
        ivec2 param_20 = samplep;
        float scount = max(imageLoad(writeImages[0], mapc(param_20)).w, 1.0);
        ivec2 param_21 = samplep;
        imageStore(writeImagesBack[0], mapc(param_21), vec4(diffused.xyz * scount, scount));
    }
}

