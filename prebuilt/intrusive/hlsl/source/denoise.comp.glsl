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

layout(set = 1, binding = 9, std140) uniform type_ConstantBuffer_Matrices
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

float _97;

void main()
{
    ivec2 _101 = ivec2(gl_GlobalInvocationID.xy);
    ivec2 _107 = ivec2(uvec2(uint(_101.x), uint(_101.y)));
    vec4 _110 = imageLoad(writeImages[4], _107);
    uvec4 _111 = floatBitsToUint(_110);
    uint _115 = _111.x;
    vec2 _121 = vec2(uvec2(_115 & 65535u, (_115 >> 16u) & 65535u)) * vec2(1.52587890625e-05);
    bool _124 = uintBitsToFloat(_111.z) <= 0.0;
    uint _125 = _111.y;
    vec3 _143 = mix(materials._m0[_125].diffuse, textureLod(sampler2D(textures[materials._m0[_125].diffuseTexture], samplers[2u]), _121, 0.0), bvec4(materials._m0[_125].diffuseTexture >= 0)).xyz;
    vec4 _153 = vec4(mix(pow(_143 + vec3(0.052132703363895416259765625), vec3(2.400000095367431640625)), _143 * vec3(0.077399380505084991455078125), mix(vec3(0.0), vec3(1.0), lessThan(_143, vec3(0.040449999272823333740234375)))), _97);
    vec3 _161 = mix(materials._m0[_125].emission, textureLod(sampler2D(textures[materials._m0[_125].emissionTexture], samplers[2u]), _121, 0.0), bvec4(materials._m0[_125].emissionTexture >= 0)).xyz;
    vec4 _171 = vec4(mix(pow(_161 + vec3(0.052132703363895416259765625), vec3(2.400000095367431640625)), _161 * vec3(0.077399380505084991455078125), mix(vec3(0.0), vec3(1.0), lessThan(_161, vec3(0.040449999272823333740234375)))), _97);
    vec4 _187 = pushed.modelview * normalize(vec4(imageLoad(writeBuffer[10], _107).xyz, 1.0) * pushed.modelview);
    vec3 _191 = max(_153.xyz - _171.xyz, vec3(0.0));
    vec4 _213;
    vec4 _214;
    if (_124)
    {
        vec4 _211 = textureLod(sampler2D(background, samplers[3u]), vec2((atan(_187.z, _187.x) * 0.15915493667125701904296875) + 0.5, 1.0 - (acos(-_187.y) * 0.3183098733425140380859375)), 0.0);
        _213 = vec4(_211.x, _211.y, _211.z, _171.w);
        _214 = vec4(vec3(0.0).x, vec3(0.0).y, vec3(0.0).z, _153.w);
    }
    else
    {
        _213 = _171;
        _214 = vec4(_191.x, _191.y, _191.z, _153.w);
    }
    vec4 _231 = vec4(vec4(imageLoad(writeBuffer[10], _107).xyz, 1.0) * pushed.modelview, 1.0) * pushed.projection;
    vec3 _235 = _231.xyz / vec3(_231.w);
    vec4 _240;
    _240 = vec4(0.0);
    vec4 _241;
    for (uint _243 = 0u; _243 < 9u; _240 = _241, _243++)
    {
        _241 = _240;
        vec4 _249;
        for (uint _251 = 0u; _251 < 9u; _241 = _249, _251++)
        {
            ivec2 _261 = _101 + ivec2(int(_243 - 4u), int(_251 - 4u));
            ivec2 _267 = ivec2(uvec2(uint(_261.x), uint(_261.y)));
            vec4 _281 = vec4(vec4(imageLoad(writeBuffer[10], _267).xyz, 1.0) * pushed.modelview, 1.0) * pushed.projection;
            vec3 _285 = _281.xyz / vec3(_281.w);
            if (((((dot(imageLoad(writeImages[9], _267).xyz, imageLoad(writeImages[9], _107).xyz) >= 0.5) && (distance(vec4(_285, 1.0).xyz, _235) < 0.00999999977648258209228515625)) && (abs(_235.z - _285.z) < 0.004999999888241291046142578125)) || ((_243 == 4u) && (_251 == 4u))) || ((imageLoad(writeImages[0], _107).w <= 9.9999997473787516355514526367188e-05) && (_241.w <= 9.9999997473787516355514526367188e-05)))
            {
                vec4 _316 = imageLoad(writeImages[0], _267);
                float _317 = _316.w;
                vec3 _323 = clamp(_316.xyz / vec3(max(_317, 0.5)), vec3(0.0), vec3(16.0)) * _317;
                vec4 _325 = vec4(_323.x, _323.y, _323.z, _316.w);
                _325.w = _317;
                _249 = _241 + _325;
            }
            else
            {
                _249 = _241;
            }
        }
    }
    float _328 = max(_240.w, 1.0);
    vec4 _329 = _240;
    _329.w = _328;
    vec4 _333 = max(mix(_329, vec4(0.0, 0.0, 0.0, 1.0), bvec4(_328 <= 0.0)), vec4(0.0));
    vec4 _336 = imageLoad(writeBuffer[2], _107);
    vec4 _339 = imageLoad(writeImages[2], _107);
    vec4 _343 = _339 / vec4(max(_339.w, 1.0));
    vec4 _347 = _336 / vec4(max(_336.w, 1.0));
    uvec2 _368 = uvec2(_101);
    imageStore(writeImages[2], ivec2(_368), vec4(clamp(mix(_343.xyz / vec3(max(_343.w, 1.0)), _347.xyz / vec3(max(_347.w, 1.0)), vec3(1.0 - (mix(materials._m0[_125].specular, textureLod(sampler2D(textures[materials._m0[_125].specularTexture], samplers[2u]), _121, 0.0), bvec4(materials._m0[_125].specularTexture >= 0)).y * 0.5))), vec3(0.0), vec3(1.0)), 1.0));
    imageStore(writeBuffer[8], ivec2(gl_GlobalInvocationID.xy), vec4(((_333.xyz / vec3(max(_333.w, 1.0))) * _214.xyz) + max(_213.xyz, vec3(0.0)), 1.0));
    imageStore(writeImagesBack[0u], ivec2(_368), imageLoad(writeImages[0u], ivec2(_368)));
    imageStore(writeImagesBack[1u], ivec2(_368), imageLoad(writeImages[1u], ivec2(_368)));
    imageStore(writeImagesBack[2u], ivec2(_368), imageLoad(writeImages[2u], ivec2(_368)));
    imageStore(writeImagesBack[3u], ivec2(_368), imageLoad(writeImages[3u], ivec2(_368)));
    imageStore(writeImagesBack[4u], ivec2(_368), imageLoad(writeImages[4u], ivec2(_368)));
    imageStore(writeImagesBack[5u], ivec2(_368), imageLoad(writeImages[5u], ivec2(_368)));
    imageStore(writeImagesBack[6u], ivec2(_368), imageLoad(writeImages[6u], ivec2(_368)));
    imageStore(writeImagesBack[7u], ivec2(_368), imageLoad(writeImages[7u], ivec2(_368)));
    imageStore(writeImagesBack[8u], ivec2(_368), imageLoad(writeImages[8u], ivec2(_368)));
    imageStore(writeImagesBack[9u], ivec2(_368), imageLoad(writeImages[9u], ivec2(_368)));
    imageStore(writeImagesBack[10u], ivec2(_368), imageLoad(writeImages[10u], ivec2(_368)));
    imageStore(writeImagesBack[11u], ivec2(_368), imageLoad(writeImages[11u], ivec2(_368)));
    if ((!_124) && (imageLoad(writeBuffer[3], _107).w > 0.0))
    {
        float _458 = max(imageLoad(writeImages[0], ivec2(_368)).w, 1.0);
        imageStore(writeImagesBack[0], ivec2(_368), vec4(_214.xyz * _458, _458));
    }
}

