#version 450
#extension GL_EXT_nonuniform_qualifier : require
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

layout(set = 0, binding = 1, std140) uniform type_Globals
{
    float HDR_GAMMA;
    float PHI;
    float SFN;
    float SFO;
    float INFINITY;
    float N_INFINITY;
    float PI;
    float TWO_PI;
    float SQRT_OF_ONE_THIRD;
    float E;
    float INV_PI;
    float TWO_INV_PI;
    float INV_TWO_PI;
} _Globals;

layout(set = 1, binding = 9, std140) uniform type_ConstantBuffer_Matrices
{
    layout(row_major) mat4 projection;
    layout(row_major) mat4 projectionInv;
    layout(row_major) mat3x4 modelview;
    layout(row_major) mat3x4 modelviewInv;
    layout(row_major) mat3x4 modelviewPrev;
    layout(row_major) mat3x4 modelviewPrevInv;
    uvec4 mdata;
    uvec2 tdata;
    uvec2 rdata;
} pushed;

layout(set = 4, binding = 20, std430) buffer type_RWStructuredBuffer_MaterialUnit
{
    MaterialUnit _m0[];
} materials;

layout(set = 2, binding = 14) uniform sampler samplers[4];
layout(set = 3, binding = 16, rgba32f) uniform image2D writeBuffer[];
layout(set = 3, binding = 17, rgba32f) uniform image2D writeImages[];
layout(set = 3, binding = 18, rgba32f) uniform writeonly image2D writeImagesBack[];
layout(set = 4, binding = 21) uniform texture2D background;
layout(set = 4, binding = 22) uniform texture2D textures[];

float _100;

void main()
{
    ivec2 _104 = ivec2(gl_GlobalInvocationID.xy);
    ivec2 _110 = ivec2(uvec2(uint(_104.x), uint(_104.y)));
    vec4 _113 = imageLoad(writeImages[4], _110);
    uvec4 _114 = floatBitsToUint(_113);
    uint _118 = _114.x;
    vec2 _124 = vec2(uvec2(_118 & 65535u, (_118 >> 16u) & 65535u)) * vec2(1.52587890625e-05);
    bool _127 = uintBitsToFloat(_114.z) <= 0.0;
    uint _128 = _114.y;
    vec3 _146 = mix(materials._m0[_128].diffuse, textureLod(sampler2D(textures[materials._m0[_128].diffuseTexture], samplers[2u]), _124, 0.0), bvec4(materials._m0[_128].diffuseTexture >= 0)).xyz;
    vec4 _156 = vec4(mix(pow(_146 + vec3(0.052132703363895416259765625), vec3(2.400000095367431640625)), _146 * vec3(0.077399380505084991455078125), mix(vec3(0.0), vec3(1.0), lessThan(_146, vec3(0.040449999272823333740234375)))), _100);
    vec3 _164 = mix(materials._m0[_128].emission, textureLod(sampler2D(textures[materials._m0[_128].emissionTexture], samplers[2u]), _124, 0.0), bvec4(materials._m0[_128].emissionTexture >= 0)).xyz;
    vec4 _174 = vec4(mix(pow(_164 + vec3(0.052132703363895416259765625), vec3(2.400000095367431640625)), _164 * vec3(0.077399380505084991455078125), mix(vec3(0.0), vec3(1.0), lessThan(_164, vec3(0.040449999272823333740234375)))), _100);
    vec4 _190 = pushed.modelview * normalize(vec4(imageLoad(writeBuffer[10], _110).xyz, 1.0) * pushed.modelview);
    vec3 _194 = max(_156.xyz - _174.xyz, vec3(0.0));
    vec4 _220;
    vec4 _221;
    if (_127)
    {
        vec4 _218 = textureLod(sampler2D(background, samplers[3u]), vec2((atan(_190.z, _190.x) * _Globals.INV_TWO_PI) + 0.5, 1.0 - (acos(-_190.y) * _Globals.INV_PI)), 0.0);
        _220 = vec4(_218.x, _218.y, _218.z, _174.w);
        _221 = vec4(vec3(0.0).x, vec3(0.0).y, vec3(0.0).z, _156.w);
    }
    else
    {
        _220 = _174;
        _221 = vec4(_194.x, _194.y, _194.z, _156.w);
    }
    vec4 _238 = vec4(vec4(imageLoad(writeBuffer[10], _110).xyz, 1.0) * pushed.modelview, 1.0) * pushed.projection;
    vec3 _242 = _238.xyz / vec3(_238.w);
    vec4 _247;
    _247 = vec4(0.0);
    vec4 _248;
    for (uint _250 = 0u; _250 < 9u; _247 = _248, _250++)
    {
        _248 = _247;
        vec4 _256;
        for (uint _258 = 0u; _258 < 9u; _248 = _256, _258++)
        {
            ivec2 _268 = _104 + ivec2(int(_250 - 4u), int(_258 - 4u));
            ivec2 _274 = ivec2(uvec2(uint(_268.x), uint(_268.y)));
            vec4 _288 = vec4(vec4(imageLoad(writeBuffer[10], _274).xyz, 1.0) * pushed.modelview, 1.0) * pushed.projection;
            vec3 _292 = _288.xyz / vec3(_288.w);
            if (((((dot(imageLoad(writeImages[9], _274).xyz, imageLoad(writeImages[9], _110).xyz) >= 0.5) && (distance(vec4(_292, 1.0).xyz, _242) < 0.00999999977648258209228515625)) && (abs(_242.z - _292.z) < 0.004999999888241291046142578125)) || ((_250 == 4u) && (_258 == 4u))) || ((imageLoad(writeImages[0], _110).w <= 9.9999997473787516355514526367188e-05) && (_248.w <= 9.9999997473787516355514526367188e-05)))
            {
                vec4 _323 = imageLoad(writeImages[0], _274);
                float _324 = _323.w;
                vec3 _330 = clamp(_323.xyz / vec3(max(_324, 0.5)), vec3(0.0), vec3(16.0)) * _324;
                vec4 _332 = vec4(_330.x, _330.y, _330.z, _323.w);
                _332.w = _324;
                _256 = _248 + _332;
            }
            else
            {
                _256 = _248;
            }
        }
    }
    float _335 = max(_247.w, 1.0);
    vec4 _336 = _247;
    _336.w = _335;
    vec4 _340 = max(mix(_336, vec4(0.0, 0.0, 0.0, 1.0), bvec4(_335 <= 0.0)), vec4(0.0));
    vec4 _343 = imageLoad(writeBuffer[2], _110);
    vec4 _346 = imageLoad(writeImages[2], _110);
    vec4 _350 = _346 / vec4(max(_346.w, 1.0));
    vec4 _354 = _343 / vec4(max(_343.w, 1.0));
    uvec2 _375 = uvec2(_104);
    imageStore(writeImages[2], ivec2(_375), vec4(clamp(mix(_350.xyz / vec3(max(_350.w, 1.0)), _354.xyz / vec3(max(_354.w, 1.0)), vec3(1.0 - (mix(materials._m0[_128].specular, textureLod(sampler2D(textures[materials._m0[_128].specularTexture], samplers[2u]), _124, 0.0), bvec4(materials._m0[_128].specularTexture >= 0)).y * 0.5))), vec3(0.0), vec3(1.0)), 1.0));
    imageStore(writeBuffer[8], ivec2(gl_GlobalInvocationID.xy), vec4(((_340.xyz / vec3(max(_340.w, 1.0))) * _221.xyz) + max(_220.xyz, vec3(0.0)), 1.0));
    imageStore(writeImagesBack[0u], ivec2(_375), imageLoad(writeImages[0u], ivec2(_375)));
    imageStore(writeImagesBack[1u], ivec2(_375), imageLoad(writeImages[1u], ivec2(_375)));
    imageStore(writeImagesBack[2u], ivec2(_375), imageLoad(writeImages[2u], ivec2(_375)));
    imageStore(writeImagesBack[3u], ivec2(_375), imageLoad(writeImages[3u], ivec2(_375)));
    imageStore(writeImagesBack[4u], ivec2(_375), imageLoad(writeImages[4u], ivec2(_375)));
    imageStore(writeImagesBack[5u], ivec2(_375), imageLoad(writeImages[5u], ivec2(_375)));
    imageStore(writeImagesBack[6u], ivec2(_375), imageLoad(writeImages[6u], ivec2(_375)));
    imageStore(writeImagesBack[7u], ivec2(_375), imageLoad(writeImages[7u], ivec2(_375)));
    imageStore(writeImagesBack[8u], ivec2(_375), imageLoad(writeImages[8u], ivec2(_375)));
    imageStore(writeImagesBack[9u], ivec2(_375), imageLoad(writeImages[9u], ivec2(_375)));
    imageStore(writeImagesBack[10u], ivec2(_375), imageLoad(writeImages[10u], ivec2(_375)));
    imageStore(writeImagesBack[11u], ivec2(_375), imageLoad(writeImages[11u], ivec2(_375)));
    if ((!_127) && (imageLoad(writeBuffer[3], _110).w > 0.0))
    {
        float _465 = max(imageLoad(writeImages[0], ivec2(_375)).w, 1.0);
        imageStore(writeImagesBack[0], ivec2(_375), vec4(_221.xyz * _465, _465));
    }
}

