#version 450
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

layout(set = 0, binding = 5, std140) uniform type_Globals
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

layout(set = 1, binding = 9, scalar) uniform type_ConstantBuffer_Matrices
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

layout(set = 4, binding = 20, std140) uniform type_StructuredBuffer_MaterialUnit
{
    MaterialUnit _m0[];
} materials[];

layout(set = 2, binding = 14) uniform sampler samplers[4];
layout(set = 3, binding = 16, rgba32f) uniform image2D writeBuffer[];
layout(set = 3, binding = 17, rgba32f) uniform image2D writeImages[];
layout(set = 3, binding = 18, rgba32f) uniform writeonly image2D writeImagesBack[];
layout(set = 4, binding = 21) uniform texture2D background;
layout(set = 4, binding = 22) uniform texture2D textures[];

float _101;

void main()
{
    ivec2 _107 = ivec2(gl_GlobalInvocationID.xy);
    ivec2 _113 = ivec2(uvec2(uint(_107.x), uint(_107.y)));
    vec4 _116 = imageLoad(writeImages[4], _113);
    uvec4 _117 = floatBitsToUint(_116);
    uint _121 = _117.x;
    vec2 _127 = vec2(uvec2(_121 & 65535u, (_121 >> 16u) & 65535u)) * vec2(1.52587890625e-05);
    bool _130 = uintBitsToFloat(_117.z) <= 0.0;
    uint _131 = _117.y;
    vec3 _154 = mix(materials[0u]._m0[_131].diffuse, textureLod(sampler2D(textures[materials[0u]._m0[_131].diffuseTexture], samplers[2u]), _127, 0.0), bvec4(materials[0u]._m0[_131].diffuseTexture >= 0)).xyz;
    vec4 _164 = vec4(mix(pow(_154 + vec3(0.052132703363895416259765625), vec3(2.400000095367431640625)), _154 * vec3(0.077399380505084991455078125), mix(vec3(0.0), vec3(1.0), lessThan(_154, vec3(0.040449999272823333740234375)))), _101);
    vec3 _172 = mix(materials[0u]._m0[_131].emission, textureLod(sampler2D(textures[materials[0u]._m0[_131].emissionTexture], samplers[2u]), _127, 0.0), bvec4(materials[0u]._m0[_131].emissionTexture >= 0)).xyz;
    vec4 _182 = vec4(mix(pow(_172 + vec3(0.052132703363895416259765625), vec3(2.400000095367431640625)), _172 * vec3(0.077399380505084991455078125), mix(vec3(0.0), vec3(1.0), lessThan(_172, vec3(0.040449999272823333740234375)))), _101);
    vec4 _198 = pushed.modelview * normalize(vec4(imageLoad(writeBuffer[10], _113).xyz, 1.0) * pushed.modelview);
    vec3 _202 = max(_164.xyz - _182.xyz, vec3(0.0));
    vec4 _228;
    vec4 _229;
    if (_130)
    {
        vec4 _226 = textureLod(sampler2D(background, samplers[3u]), vec2((atan(_198.z, _198.x) * _Globals.INV_TWO_PI) + 0.5, 1.0 - (acos(-_198.y) * _Globals.INV_PI)), 0.0);
        _228 = vec4(_226.x, _226.y, _226.z, _182.w);
        _229 = vec4(vec3(0.0).x, vec3(0.0).y, vec3(0.0).z, _164.w);
    }
    else
    {
        _228 = _182;
        _229 = vec4(_202.x, _202.y, _202.z, _164.w);
    }
    vec4 _246 = vec4(vec4(imageLoad(writeBuffer[10], _113).xyz, 1.0) * pushed.modelview, 1.0) * pushed.projection;
    vec3 _250 = _246.xyz / vec3(_246.w);
    vec4 _255;
    _255 = vec4(0.0);
    vec4 _256;
    for (uint _258 = 0u; _258 < 9u; _255 = _256, _258++)
    {
        _256 = _255;
        vec4 _264;
        for (uint _266 = 0u; _266 < 9u; _256 = _264, _266++)
        {
            ivec2 _276 = _107 + ivec2(int(_258 - 4u), int(_266 - 4u));
            ivec2 _282 = ivec2(uvec2(uint(_276.x), uint(_276.y)));
            vec4 _296 = vec4(vec4(imageLoad(writeBuffer[10], _282).xyz, 1.0) * pushed.modelview, 1.0) * pushed.projection;
            vec3 _300 = _296.xyz / vec3(_296.w);
            if (((((dot(imageLoad(writeImages[9], _282).xyz, imageLoad(writeImages[9], _113).xyz) >= 0.5) && (distance(vec4(_300, 1.0).xyz, _250) < 0.00999999977648258209228515625)) && (abs(_250.z - _300.z) < 0.004999999888241291046142578125)) || ((_258 == 4u) && (_266 == 4u))) || ((imageLoad(writeImages[0], _113).w <= 9.9999997473787516355514526367188e-05) && (_256.w <= 9.9999997473787516355514526367188e-05)))
            {
                vec4 _331 = imageLoad(writeImages[0], _282);
                float _332 = _331.w;
                vec3 _338 = clamp(_331.xyz / vec3(max(_332, 0.5)), vec3(0.0), vec3(16.0)) * _332;
                vec4 _340 = vec4(_338.x, _338.y, _338.z, _331.w);
                _340.w = _332;
                _264 = _256 + _340;
            }
            else
            {
                _264 = _256;
            }
        }
    }
    float _343 = max(_255.w, 1.0);
    vec4 _344 = _255;
    _344.w = _343;
    vec4 _348 = max(mix(_344, vec4(0.0, 0.0, 0.0, 1.0), bvec4(_343 <= 0.0)), vec4(0.0));
    vec4 _351 = imageLoad(writeBuffer[2], _113);
    vec4 _354 = imageLoad(writeImages[2], _113);
    vec4 _358 = _354 / vec4(max(_354.w, 1.0));
    vec4 _362 = _351 / vec4(max(_351.w, 1.0));
    uvec2 _383 = uvec2(_107);
    imageStore(writeImages[2], ivec2(_383), vec4(clamp(mix(_358.xyz / vec3(max(_358.w, 1.0)), _362.xyz / vec3(max(_362.w, 1.0)), vec3(1.0 - (mix(materials[0u]._m0[_131].specular, textureLod(sampler2D(textures[materials[0u]._m0[_131].specularTexture], samplers[2u]), _127, 0.0), bvec4(materials[0u]._m0[_131].specularTexture >= 0)).y * 0.5))), vec3(0.0), vec3(1.0)), 1.0));
    imageStore(writeBuffer[8], ivec2(gl_GlobalInvocationID.xy), vec4(((_348.xyz / vec3(max(_348.w, 1.0))) * _229.xyz) + max(_228.xyz, vec3(0.0)), 1.0));
    imageStore(writeImagesBack[0u], ivec2(_383), imageLoad(writeImages[0u], ivec2(_383)));
    imageStore(writeImagesBack[1u], ivec2(_383), imageLoad(writeImages[1u], ivec2(_383)));
    imageStore(writeImagesBack[2u], ivec2(_383), imageLoad(writeImages[2u], ivec2(_383)));
    imageStore(writeImagesBack[3u], ivec2(_383), imageLoad(writeImages[3u], ivec2(_383)));
    imageStore(writeImagesBack[4u], ivec2(_383), imageLoad(writeImages[4u], ivec2(_383)));
    imageStore(writeImagesBack[5u], ivec2(_383), imageLoad(writeImages[5u], ivec2(_383)));
    imageStore(writeImagesBack[6u], ivec2(_383), imageLoad(writeImages[6u], ivec2(_383)));
    imageStore(writeImagesBack[7u], ivec2(_383), imageLoad(writeImages[7u], ivec2(_383)));
    imageStore(writeImagesBack[8u], ivec2(_383), imageLoad(writeImages[8u], ivec2(_383)));
    imageStore(writeImagesBack[9u], ivec2(_383), imageLoad(writeImages[9u], ivec2(_383)));
    imageStore(writeImagesBack[10u], ivec2(_383), imageLoad(writeImages[10u], ivec2(_383)));
    imageStore(writeImagesBack[11u], ivec2(_383), imageLoad(writeImages[11u], ivec2(_383)));
    if ((!_130) && (imageLoad(writeBuffer[3], _113).w > 0.0))
    {
        float _473 = max(imageLoad(writeImages[0], ivec2(_383)).w, 1.0);
        imageStore(writeImagesBack[0], ivec2(_383), vec4(_229.xyz * _473, _473));
    }
}

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

layout(set = 0, binding = 5, std140) uniform type_Globals
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

layout(set = 4, binding = 20, std140) uniform type_StructuredBuffer_MaterialUnit
{
    MaterialUnit _m0[];
} materials[];

layout(set = 2, binding = 14) uniform sampler samplers[4];
layout(set = 3, binding = 16, rgba32f) uniform image2D writeBuffer[];
layout(set = 3, binding = 17, rgba32f) uniform image2D writeImages[];
layout(set = 3, binding = 18, rgba32f) uniform writeonly image2D writeImagesBack[];
layout(set = 4, binding = 21) uniform texture2D background;
layout(set = 4, binding = 22) uniform texture2D textures[];

float _101;

void main()
{
    ivec2 _107 = ivec2(gl_GlobalInvocationID.xy);
    ivec2 _113 = ivec2(uvec2(uint(_107.x), uint(_107.y)));
    vec4 _116 = imageLoad(writeImages[4], _113);
    uvec4 _117 = floatBitsToUint(_116);
    uint _121 = _117.x;
    vec2 _127 = vec2(uvec2(_121 & 65535u, (_121 >> 16u) & 65535u)) * vec2(1.52587890625e-05);
    bool _130 = uintBitsToFloat(_117.z) <= 0.0;
    uint _131 = _117.y;
    vec3 _154 = mix(materials[0u]._m0[_131].diffuse, textureLod(sampler2D(textures[materials[0u]._m0[_131].diffuseTexture], samplers[2u]), _127, 0.0), bvec4(materials[0u]._m0[_131].diffuseTexture >= 0)).xyz;
    vec4 _164 = vec4(mix(pow(_154 + vec3(0.052132703363895416259765625), vec3(2.400000095367431640625)), _154 * vec3(0.077399380505084991455078125), mix(vec3(0.0), vec3(1.0), lessThan(_154, vec3(0.040449999272823333740234375)))), _101);
    vec3 _172 = mix(materials[0u]._m0[_131].emission, textureLod(sampler2D(textures[materials[0u]._m0[_131].emissionTexture], samplers[2u]), _127, 0.0), bvec4(materials[0u]._m0[_131].emissionTexture >= 0)).xyz;
    vec4 _182 = vec4(mix(pow(_172 + vec3(0.052132703363895416259765625), vec3(2.400000095367431640625)), _172 * vec3(0.077399380505084991455078125), mix(vec3(0.0), vec3(1.0), lessThan(_172, vec3(0.040449999272823333740234375)))), _101);
    vec4 _198 = pushed.modelview * normalize(vec4(imageLoad(writeBuffer[10], _113).xyz, 1.0) * pushed.modelview);
    vec3 _202 = max(_164.xyz - _182.xyz, vec3(0.0));
    vec4 _228;
    vec4 _229;
    if (_130)
    {
        vec4 _226 = textureLod(sampler2D(background, samplers[3u]), vec2((atan(_198.z, _198.x) * _Globals.INV_TWO_PI) + 0.5, 1.0 - (acos(-_198.y) * _Globals.INV_PI)), 0.0);
        _228 = vec4(_226.x, _226.y, _226.z, _182.w);
        _229 = vec4(vec3(0.0).x, vec3(0.0).y, vec3(0.0).z, _164.w);
    }
    else
    {
        _228 = _182;
        _229 = vec4(_202.x, _202.y, _202.z, _164.w);
    }
    vec4 _246 = vec4(vec4(imageLoad(writeBuffer[10], _113).xyz, 1.0) * pushed.modelview, 1.0) * pushed.projection;
    vec3 _250 = _246.xyz / vec3(_246.w);
    vec4 _255;
    _255 = vec4(0.0);
    vec4 _256;
    for (uint _258 = 0u; _258 < 9u; _255 = _256, _258++)
    {
        _256 = _255;
        vec4 _264;
        for (uint _266 = 0u; _266 < 9u; _256 = _264, _266++)
        {
            ivec2 _276 = _107 + ivec2(int(_258 - 4u), int(_266 - 4u));
            ivec2 _282 = ivec2(uvec2(uint(_276.x), uint(_276.y)));
            vec4 _296 = vec4(vec4(imageLoad(writeBuffer[10], _282).xyz, 1.0) * pushed.modelview, 1.0) * pushed.projection;
            vec3 _300 = _296.xyz / vec3(_296.w);
            if (((((dot(imageLoad(writeImages[9], _282).xyz, imageLoad(writeImages[9], _113).xyz) >= 0.5) && (distance(vec4(_300, 1.0).xyz, _250) < 0.00999999977648258209228515625)) && (abs(_250.z - _300.z) < 0.004999999888241291046142578125)) || ((_258 == 4u) && (_266 == 4u))) || ((imageLoad(writeImages[0], _113).w <= 9.9999997473787516355514526367188e-05) && (_256.w <= 9.9999997473787516355514526367188e-05)))
            {
                vec4 _331 = imageLoad(writeImages[0], _282);
                float _332 = _331.w;
                vec3 _338 = clamp(_331.xyz / vec3(max(_332, 0.5)), vec3(0.0), vec3(16.0)) * _332;
                vec4 _340 = vec4(_338.x, _338.y, _338.z, _331.w);
                _340.w = _332;
                _264 = _256 + _340;
            }
            else
            {
                _264 = _256;
            }
        }
    }
    float _343 = max(_255.w, 1.0);
    vec4 _344 = _255;
    _344.w = _343;
    vec4 _348 = max(mix(_344, vec4(0.0, 0.0, 0.0, 1.0), bvec4(_343 <= 0.0)), vec4(0.0));
    vec4 _351 = imageLoad(writeBuffer[2], _113);
    vec4 _354 = imageLoad(writeImages[2], _113);
    vec4 _358 = _354 / vec4(max(_354.w, 1.0));
    vec4 _362 = _351 / vec4(max(_351.w, 1.0));
    uvec2 _383 = uvec2(_107);
    imageStore(writeImages[2], ivec2(_383), vec4(clamp(mix(_358.xyz / vec3(max(_358.w, 1.0)), _362.xyz / vec3(max(_362.w, 1.0)), vec3(1.0 - (mix(materials[0u]._m0[_131].specular, textureLod(sampler2D(textures[materials[0u]._m0[_131].specularTexture], samplers[2u]), _127, 0.0), bvec4(materials[0u]._m0[_131].specularTexture >= 0)).y * 0.5))), vec3(0.0), vec3(1.0)), 1.0));
    imageStore(writeBuffer[8], ivec2(gl_GlobalInvocationID.xy), vec4(((_348.xyz / vec3(max(_348.w, 1.0))) * _229.xyz) + max(_228.xyz, vec3(0.0)), 1.0));
    imageStore(writeImagesBack[0u], ivec2(_383), imageLoad(writeImages[0u], ivec2(_383)));
    imageStore(writeImagesBack[1u], ivec2(_383), imageLoad(writeImages[1u], ivec2(_383)));
    imageStore(writeImagesBack[2u], ivec2(_383), imageLoad(writeImages[2u], ivec2(_383)));
    imageStore(writeImagesBack[3u], ivec2(_383), imageLoad(writeImages[3u], ivec2(_383)));
    imageStore(writeImagesBack[4u], ivec2(_383), imageLoad(writeImages[4u], ivec2(_383)));
    imageStore(writeImagesBack[5u], ivec2(_383), imageLoad(writeImages[5u], ivec2(_383)));
    imageStore(writeImagesBack[6u], ivec2(_383), imageLoad(writeImages[6u], ivec2(_383)));
    imageStore(writeImagesBack[7u], ivec2(_383), imageLoad(writeImages[7u], ivec2(_383)));
    imageStore(writeImagesBack[8u], ivec2(_383), imageLoad(writeImages[8u], ivec2(_383)));
    imageStore(writeImagesBack[9u], ivec2(_383), imageLoad(writeImages[9u], ivec2(_383)));
    imageStore(writeImagesBack[10u], ivec2(_383), imageLoad(writeImages[10u], ivec2(_383)));
    imageStore(writeImagesBack[11u], ivec2(_383), imageLoad(writeImages[11u], ivec2(_383)));
    if ((!_130) && (imageLoad(writeBuffer[3], _113).w > 0.0))
    {
        float _473 = max(imageLoad(writeImages[0], ivec2(_383)).w, 1.0);
        imageStore(writeImagesBack[0], ivec2(_383), vec4(_229.xyz * _473, _473));
    }
}

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

layout(set = 0, binding = 5, std140) uniform type_Globals
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

layout(set = 4, binding = 20, std140) uniform type_StructuredBuffer_MaterialUnit
{
    MaterialUnit _m0[];
} materials[64];

layout(set = 2, binding = 14) uniform sampler samplers[4];
layout(set = 3, binding = 16, rgba32f) uniform image2D writeBuffer[];
layout(set = 3, binding = 17, rgba32f) uniform image2D writeImages[];
layout(set = 3, binding = 18, rgba32f) uniform writeonly image2D writeImagesBack[];
layout(set = 4, binding = 21) uniform texture2D background;
layout(set = 4, binding = 22) uniform texture2D textures[];

float _102;

void main()
{
    ivec2 _108 = ivec2(gl_GlobalInvocationID.xy);
    ivec2 _114 = ivec2(uvec2(uint(_108.x), uint(_108.y)));
    vec4 _117 = imageLoad(writeImages[4], _114);
    uvec4 _118 = floatBitsToUint(_117);
    uint _122 = _118.x;
    vec2 _128 = vec2(uvec2(_122 & 65535u, (_122 >> 16u) & 65535u)) * vec2(1.52587890625e-05);
    bool _131 = uintBitsToFloat(_118.z) <= 0.0;
    uint _132 = _118.y;
    vec3 _155 = mix(materials[0u]._m0[_132].diffuse, textureLod(sampler2D(textures[materials[0u]._m0[_132].diffuseTexture], samplers[2u]), _128, 0.0), bvec4(materials[0u]._m0[_132].diffuseTexture >= 0)).xyz;
    vec4 _165 = vec4(mix(pow(_155 + vec3(0.052132703363895416259765625), vec3(2.400000095367431640625)), _155 * vec3(0.077399380505084991455078125), mix(vec3(0.0), vec3(1.0), lessThan(_155, vec3(0.040449999272823333740234375)))), _102);
    vec3 _173 = mix(materials[0u]._m0[_132].emission, textureLod(sampler2D(textures[materials[0u]._m0[_132].emissionTexture], samplers[2u]), _128, 0.0), bvec4(materials[0u]._m0[_132].emissionTexture >= 0)).xyz;
    vec4 _183 = vec4(mix(pow(_173 + vec3(0.052132703363895416259765625), vec3(2.400000095367431640625)), _173 * vec3(0.077399380505084991455078125), mix(vec3(0.0), vec3(1.0), lessThan(_173, vec3(0.040449999272823333740234375)))), _102);
    vec4 _199 = pushed.modelview * normalize(vec4(imageLoad(writeBuffer[10], _114).xyz, 1.0) * pushed.modelview);
    vec3 _203 = max(_165.xyz - _183.xyz, vec3(0.0));
    vec4 _229;
    vec4 _230;
    if (_131)
    {
        vec4 _227 = textureLod(sampler2D(background, samplers[3u]), vec2((atan(_199.z, _199.x) * _Globals.INV_TWO_PI) + 0.5, 1.0 - (acos(-_199.y) * _Globals.INV_PI)), 0.0);
        _229 = vec4(_227.x, _227.y, _227.z, _183.w);
        _230 = vec4(vec3(0.0).x, vec3(0.0).y, vec3(0.0).z, _165.w);
    }
    else
    {
        _229 = _183;
        _230 = vec4(_203.x, _203.y, _203.z, _165.w);
    }
    vec4 _247 = vec4(vec4(imageLoad(writeBuffer[10], _114).xyz, 1.0) * pushed.modelview, 1.0) * pushed.projection;
    vec3 _251 = _247.xyz / vec3(_247.w);
    vec4 _256;
    _256 = vec4(0.0);
    vec4 _257;
    for (uint _259 = 0u; _259 < 9u; _256 = _257, _259++)
    {
        _257 = _256;
        vec4 _265;
        for (uint _267 = 0u; _267 < 9u; _257 = _265, _267++)
        {
            ivec2 _277 = _108 + ivec2(int(_259 - 4u), int(_267 - 4u));
            ivec2 _283 = ivec2(uvec2(uint(_277.x), uint(_277.y)));
            vec4 _297 = vec4(vec4(imageLoad(writeBuffer[10], _283).xyz, 1.0) * pushed.modelview, 1.0) * pushed.projection;
            vec3 _301 = _297.xyz / vec3(_297.w);
            if (((((dot(imageLoad(writeImages[9], _283).xyz, imageLoad(writeImages[9], _114).xyz) >= 0.5) && (distance(vec4(_301, 1.0).xyz, _251) < 0.00999999977648258209228515625)) && (abs(_251.z - _301.z) < 0.004999999888241291046142578125)) || ((_259 == 4u) && (_267 == 4u))) || ((imageLoad(writeImages[0], _114).w <= 9.9999997473787516355514526367188e-05) && (_257.w <= 9.9999997473787516355514526367188e-05)))
            {
                vec4 _332 = imageLoad(writeImages[0], _283);
                float _333 = _332.w;
                vec3 _339 = clamp(_332.xyz / vec3(max(_333, 0.5)), vec3(0.0), vec3(16.0)) * _333;
                vec4 _341 = vec4(_339.x, _339.y, _339.z, _332.w);
                _341.w = _333;
                _265 = _257 + _341;
            }
            else
            {
                _265 = _257;
            }
        }
    }
    float _344 = max(_256.w, 1.0);
    vec4 _345 = _256;
    _345.w = _344;
    vec4 _349 = max(mix(_345, vec4(0.0, 0.0, 0.0, 1.0), bvec4(_344 <= 0.0)), vec4(0.0));
    vec4 _352 = imageLoad(writeBuffer[2], _114);
    vec4 _355 = imageLoad(writeImages[2], _114);
    vec4 _359 = _355 / vec4(max(_355.w, 1.0));
    vec4 _363 = _352 / vec4(max(_352.w, 1.0));
    uvec2 _384 = uvec2(_108);
    imageStore(writeImages[2], ivec2(_384), vec4(clamp(mix(_359.xyz / vec3(max(_359.w, 1.0)), _363.xyz / vec3(max(_363.w, 1.0)), vec3(1.0 - (mix(materials[0u]._m0[_132].specular, textureLod(sampler2D(textures[materials[0u]._m0[_132].specularTexture], samplers[2u]), _128, 0.0), bvec4(materials[0u]._m0[_132].specularTexture >= 0)).y * 0.5))), vec3(0.0), vec3(1.0)), 1.0));
    imageStore(writeBuffer[8], ivec2(gl_GlobalInvocationID.xy), vec4(((_349.xyz / vec3(max(_349.w, 1.0))) * _230.xyz) + max(_229.xyz, vec3(0.0)), 1.0));
    imageStore(writeImagesBack[0u], ivec2(_384), imageLoad(writeImages[0u], ivec2(_384)));
    imageStore(writeImagesBack[1u], ivec2(_384), imageLoad(writeImages[1u], ivec2(_384)));
    imageStore(writeImagesBack[2u], ivec2(_384), imageLoad(writeImages[2u], ivec2(_384)));
    imageStore(writeImagesBack[3u], ivec2(_384), imageLoad(writeImages[3u], ivec2(_384)));
    imageStore(writeImagesBack[4u], ivec2(_384), imageLoad(writeImages[4u], ivec2(_384)));
    imageStore(writeImagesBack[5u], ivec2(_384), imageLoad(writeImages[5u], ivec2(_384)));
    imageStore(writeImagesBack[6u], ivec2(_384), imageLoad(writeImages[6u], ivec2(_384)));
    imageStore(writeImagesBack[7u], ivec2(_384), imageLoad(writeImages[7u], ivec2(_384)));
    imageStore(writeImagesBack[8u], ivec2(_384), imageLoad(writeImages[8u], ivec2(_384)));
    imageStore(writeImagesBack[9u], ivec2(_384), imageLoad(writeImages[9u], ivec2(_384)));
    imageStore(writeImagesBack[10u], ivec2(_384), imageLoad(writeImages[10u], ivec2(_384)));
    imageStore(writeImagesBack[11u], ivec2(_384), imageLoad(writeImages[11u], ivec2(_384)));
    if ((!_131) && (imageLoad(writeBuffer[3], _114).w > 0.0))
    {
        float _474 = max(imageLoad(writeImages[0], ivec2(_384)).w, 1.0);
        imageStore(writeImagesBack[0], ivec2(_384), vec4(_230.xyz * _474, _474));
    }
}

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

layout(set = 0, binding = 5, std140) uniform type_Globals
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

layout(set = 4, binding = 20, std140) uniform type_StructuredBuffer_MaterialUnit
{
    MaterialUnit _m0[];
} materials[64];

layout(set = 2, binding = 14) uniform sampler samplers[4];
layout(set = 3, binding = 16, rgba32f) uniform image2D writeBuffer[];
layout(set = 3, binding = 17, rgba32f) uniform image2D writeImages[];
layout(set = 3, binding = 18, rgba32f) uniform writeonly image2D writeImagesBack[];
layout(set = 4, binding = 21) uniform texture2D background;
layout(set = 4, binding = 22) uniform texture2D textures[];

float _102;

void main()
{
    ivec2 _108 = ivec2(gl_GlobalInvocationID.xy);
    ivec2 _114 = ivec2(uvec2(uint(_108.x), uint(_108.y)));
    vec4 _117 = imageLoad(writeImages[4], _114);
    uvec4 _118 = floatBitsToUint(_117);
    uint _122 = _118.x;
    vec2 _128 = vec2(uvec2(_122 & 65535u, (_122 >> 16u) & 65535u)) * vec2(1.52587890625e-05);
    bool _131 = uintBitsToFloat(_118.z) <= 0.0;
    uint _132 = _118.y;
    vec3 _155 = mix(materials[0u]._m0[_132].diffuse, textureLod(sampler2D(textures[materials[0u]._m0[_132].diffuseTexture], samplers[2u]), _128, 0.0), bvec4(materials[0u]._m0[_132].diffuseTexture >= 0)).xyz;
    vec4 _165 = vec4(mix(pow(_155 + vec3(0.052132703363895416259765625), vec3(2.400000095367431640625)), _155 * vec3(0.077399380505084991455078125), mix(vec3(0.0), vec3(1.0), lessThan(_155, vec3(0.040449999272823333740234375)))), _102);
    vec3 _173 = mix(materials[0u]._m0[_132].emission, textureLod(sampler2D(textures[materials[0u]._m0[_132].emissionTexture], samplers[2u]), _128, 0.0), bvec4(materials[0u]._m0[_132].emissionTexture >= 0)).xyz;
    vec4 _183 = vec4(mix(pow(_173 + vec3(0.052132703363895416259765625), vec3(2.400000095367431640625)), _173 * vec3(0.077399380505084991455078125), mix(vec3(0.0), vec3(1.0), lessThan(_173, vec3(0.040449999272823333740234375)))), _102);
    vec4 _199 = pushed.modelview * normalize(vec4(imageLoad(writeBuffer[10], _114).xyz, 1.0) * pushed.modelview);
    vec3 _203 = max(_165.xyz - _183.xyz, vec3(0.0));
    vec4 _229;
    vec4 _230;
    if (_131)
    {
        vec4 _227 = textureLod(sampler2D(background, samplers[3u]), vec2((atan(_199.z, _199.x) * _Globals.INV_TWO_PI) + 0.5, 1.0 - (acos(-_199.y) * _Globals.INV_PI)), 0.0);
        _229 = vec4(_227.x, _227.y, _227.z, _183.w);
        _230 = vec4(vec3(0.0).x, vec3(0.0).y, vec3(0.0).z, _165.w);
    }
    else
    {
        _229 = _183;
        _230 = vec4(_203.x, _203.y, _203.z, _165.w);
    }
    vec4 _247 = vec4(vec4(imageLoad(writeBuffer[10], _114).xyz, 1.0) * pushed.modelview, 1.0) * pushed.projection;
    vec3 _251 = _247.xyz / vec3(_247.w);
    vec4 _256;
    _256 = vec4(0.0);
    vec4 _257;
    for (uint _259 = 0u; _259 < 9u; _256 = _257, _259++)
    {
        _257 = _256;
        vec4 _265;
        for (uint _267 = 0u; _267 < 9u; _257 = _265, _267++)
        {
            ivec2 _277 = _108 + ivec2(int(_259 - 4u), int(_267 - 4u));
            ivec2 _283 = ivec2(uvec2(uint(_277.x), uint(_277.y)));
            vec4 _297 = vec4(vec4(imageLoad(writeBuffer[10], _283).xyz, 1.0) * pushed.modelview, 1.0) * pushed.projection;
            vec3 _301 = _297.xyz / vec3(_297.w);
            if (((((dot(imageLoad(writeImages[9], _283).xyz, imageLoad(writeImages[9], _114).xyz) >= 0.5) && (distance(vec4(_301, 1.0).xyz, _251) < 0.00999999977648258209228515625)) && (abs(_251.z - _301.z) < 0.004999999888241291046142578125)) || ((_259 == 4u) && (_267 == 4u))) || ((imageLoad(writeImages[0], _114).w <= 9.9999997473787516355514526367188e-05) && (_257.w <= 9.9999997473787516355514526367188e-05)))
            {
                vec4 _332 = imageLoad(writeImages[0], _283);
                float _333 = _332.w;
                vec3 _339 = clamp(_332.xyz / vec3(max(_333, 0.5)), vec3(0.0), vec3(16.0)) * _333;
                vec4 _341 = vec4(_339.x, _339.y, _339.z, _332.w);
                _341.w = _333;
                _265 = _257 + _341;
            }
            else
            {
                _265 = _257;
            }
        }
    }
    float _344 = max(_256.w, 1.0);
    vec4 _345 = _256;
    _345.w = _344;
    vec4 _349 = max(mix(_345, vec4(0.0, 0.0, 0.0, 1.0), bvec4(_344 <= 0.0)), vec4(0.0));
    vec4 _352 = imageLoad(writeBuffer[2], _114);
    vec4 _355 = imageLoad(writeImages[2], _114);
    vec4 _359 = _355 / vec4(max(_355.w, 1.0));
    vec4 _363 = _352 / vec4(max(_352.w, 1.0));
    uvec2 _384 = uvec2(_108);
    imageStore(writeImages[2], ivec2(_384), vec4(clamp(mix(_359.xyz / vec3(max(_359.w, 1.0)), _363.xyz / vec3(max(_363.w, 1.0)), vec3(1.0 - (mix(materials[0u]._m0[_132].specular, textureLod(sampler2D(textures[materials[0u]._m0[_132].specularTexture], samplers[2u]), _128, 0.0), bvec4(materials[0u]._m0[_132].specularTexture >= 0)).y * 0.5))), vec3(0.0), vec3(1.0)), 1.0));
    imageStore(writeBuffer[8], ivec2(gl_GlobalInvocationID.xy), vec4(((_349.xyz / vec3(max(_349.w, 1.0))) * _230.xyz) + max(_229.xyz, vec3(0.0)), 1.0));
    imageStore(writeImagesBack[0u], ivec2(_384), imageLoad(writeImages[0u], ivec2(_384)));
    imageStore(writeImagesBack[1u], ivec2(_384), imageLoad(writeImages[1u], ivec2(_384)));
    imageStore(writeImagesBack[2u], ivec2(_384), imageLoad(writeImages[2u], ivec2(_384)));
    imageStore(writeImagesBack[3u], ivec2(_384), imageLoad(writeImages[3u], ivec2(_384)));
    imageStore(writeImagesBack[4u], ivec2(_384), imageLoad(writeImages[4u], ivec2(_384)));
    imageStore(writeImagesBack[5u], ivec2(_384), imageLoad(writeImages[5u], ivec2(_384)));
    imageStore(writeImagesBack[6u], ivec2(_384), imageLoad(writeImages[6u], ivec2(_384)));
    imageStore(writeImagesBack[7u], ivec2(_384), imageLoad(writeImages[7u], ivec2(_384)));
    imageStore(writeImagesBack[8u], ivec2(_384), imageLoad(writeImages[8u], ivec2(_384)));
    imageStore(writeImagesBack[9u], ivec2(_384), imageLoad(writeImages[9u], ivec2(_384)));
    imageStore(writeImagesBack[10u], ivec2(_384), imageLoad(writeImages[10u], ivec2(_384)));
    imageStore(writeImagesBack[11u], ivec2(_384), imageLoad(writeImages[11u], ivec2(_384)));
    if ((!_131) && (imageLoad(writeBuffer[3], _114).w > 0.0))
    {
        float _474 = max(imageLoad(writeImages[0], ivec2(_384)).w, 1.0);
        imageStore(writeImagesBack[0], ivec2(_384), vec4(_230.xyz * _474, _474));
    }
}

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

layout(set = 0, binding = 5, std140) uniform type_Globals
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

layout(set = 4, binding = 20, std140) uniform type_StructuredBuffer_MaterialUnit
{
    MaterialUnit _m0[];
} materials[64];

layout(set = 2, binding = 14) uniform sampler samplers[4];
layout(set = 3, binding = 16, rgba32f) uniform image2D writeBuffer[];
layout(set = 3, binding = 17, rgba32f) uniform image2D writeImages[];
layout(set = 3, binding = 18, rgba32f) uniform writeonly image2D writeImagesBack[];
layout(set = 4, binding = 21) uniform texture2D background;
layout(set = 4, binding = 22) uniform texture2D textures[];

float _102;

void main()
{
    ivec2 _108 = ivec2(gl_GlobalInvocationID.xy);
    ivec2 _114 = ivec2(uvec2(uint(_108.x), uint(_108.y)));
    vec4 _117 = imageLoad(writeImages[4], _114);
    uvec4 _118 = floatBitsToUint(_117);
    uint _122 = _118.x;
    vec2 _128 = vec2(uvec2(_122 & 65535u, (_122 >> 16u) & 65535u)) * vec2(1.52587890625e-05);
    bool _131 = uintBitsToFloat(_118.z) <= 0.0;
    uint _132 = _118.y;
    vec3 _155 = mix(materials[0u]._m0[_132].diffuse, textureLod(sampler2D(textures[materials[0u]._m0[_132].diffuseTexture], samplers[2u]), _128, 0.0), bvec4(materials[0u]._m0[_132].diffuseTexture >= 0)).xyz;
    vec4 _165 = vec4(mix(pow(_155 + vec3(0.052132703363895416259765625), vec3(2.400000095367431640625)), _155 * vec3(0.077399380505084991455078125), mix(vec3(0.0), vec3(1.0), lessThan(_155, vec3(0.040449999272823333740234375)))), _102);
    vec3 _173 = mix(materials[0u]._m0[_132].emission, textureLod(sampler2D(textures[materials[0u]._m0[_132].emissionTexture], samplers[2u]), _128, 0.0), bvec4(materials[0u]._m0[_132].emissionTexture >= 0)).xyz;
    vec4 _183 = vec4(mix(pow(_173 + vec3(0.052132703363895416259765625), vec3(2.400000095367431640625)), _173 * vec3(0.077399380505084991455078125), mix(vec3(0.0), vec3(1.0), lessThan(_173, vec3(0.040449999272823333740234375)))), _102);
    vec4 _199 = pushed.modelview * normalize(vec4(imageLoad(writeBuffer[10], _114).xyz, 1.0) * pushed.modelview);
    vec3 _203 = max(_165.xyz - _183.xyz, vec3(0.0));
    vec4 _229;
    vec4 _230;
    if (_131)
    {
        vec4 _227 = textureLod(sampler2D(background, samplers[3u]), vec2((atan(_199.z, _199.x) * _Globals.INV_TWO_PI) + 0.5, 1.0 - (acos(-_199.y) * _Globals.INV_PI)), 0.0);
        _229 = vec4(_227.x, _227.y, _227.z, _183.w);
        _230 = vec4(vec3(0.0).x, vec3(0.0).y, vec3(0.0).z, _165.w);
    }
    else
    {
        _229 = _183;
        _230 = vec4(_203.x, _203.y, _203.z, _165.w);
    }
    vec4 _247 = vec4(vec4(imageLoad(writeBuffer[10], _114).xyz, 1.0) * pushed.modelview, 1.0) * pushed.projection;
    vec3 _251 = _247.xyz / vec3(_247.w);
    vec4 _256;
    _256 = vec4(0.0);
    vec4 _257;
    for (uint _259 = 0u; _259 < 9u; _256 = _257, _259++)
    {
        _257 = _256;
        vec4 _265;
        for (uint _267 = 0u; _267 < 9u; _257 = _265, _267++)
        {
            ivec2 _277 = _108 + ivec2(int(_259 - 4u), int(_267 - 4u));
            ivec2 _283 = ivec2(uvec2(uint(_277.x), uint(_277.y)));
            vec4 _297 = vec4(vec4(imageLoad(writeBuffer[10], _283).xyz, 1.0) * pushed.modelview, 1.0) * pushed.projection;
            vec3 _301 = _297.xyz / vec3(_297.w);
            if (((((dot(imageLoad(writeImages[9], _283).xyz, imageLoad(writeImages[9], _114).xyz) >= 0.5) && (distance(vec4(_301, 1.0).xyz, _251) < 0.00999999977648258209228515625)) && (abs(_251.z - _301.z) < 0.004999999888241291046142578125)) || ((_259 == 4u) && (_267 == 4u))) || ((imageLoad(writeImages[0], _114).w <= 9.9999997473787516355514526367188e-05) && (_257.w <= 9.9999997473787516355514526367188e-05)))
            {
                vec4 _332 = imageLoad(writeImages[0], _283);
                float _333 = _332.w;
                vec3 _339 = clamp(_332.xyz / vec3(max(_333, 0.5)), vec3(0.0), vec3(16.0)) * _333;
                vec4 _341 = vec4(_339.x, _339.y, _339.z, _332.w);
                _341.w = _333;
                _265 = _257 + _341;
            }
            else
            {
                _265 = _257;
            }
        }
    }
    float _344 = max(_256.w, 1.0);
    vec4 _345 = _256;
    _345.w = _344;
    vec4 _349 = max(mix(_345, vec4(0.0, 0.0, 0.0, 1.0), bvec4(_344 <= 0.0)), vec4(0.0));
    vec4 _352 = imageLoad(writeBuffer[2], _114);
    vec4 _355 = imageLoad(writeImages[2], _114);
    vec4 _359 = _355 / vec4(max(_355.w, 1.0));
    vec4 _363 = _352 / vec4(max(_352.w, 1.0));
    uvec2 _384 = uvec2(_108);
    imageStore(writeImages[2], ivec2(_384), vec4(clamp(mix(_359.xyz / vec3(max(_359.w, 1.0)), _363.xyz / vec3(max(_363.w, 1.0)), vec3(1.0 - (mix(materials[0u]._m0[_132].specular, textureLod(sampler2D(textures[materials[0u]._m0[_132].specularTexture], samplers[2u]), _128, 0.0), bvec4(materials[0u]._m0[_132].specularTexture >= 0)).y * 0.5))), vec3(0.0), vec3(1.0)), 1.0));
    imageStore(writeBuffer[8], ivec2(gl_GlobalInvocationID.xy), vec4(((_349.xyz / vec3(max(_349.w, 1.0))) * _230.xyz) + max(_229.xyz, vec3(0.0)), 1.0));
    imageStore(writeImagesBack[0u], ivec2(_384), imageLoad(writeImages[0u], ivec2(_384)));
    imageStore(writeImagesBack[1u], ivec2(_384), imageLoad(writeImages[1u], ivec2(_384)));
    imageStore(writeImagesBack[2u], ivec2(_384), imageLoad(writeImages[2u], ivec2(_384)));
    imageStore(writeImagesBack[3u], ivec2(_384), imageLoad(writeImages[3u], ivec2(_384)));
    imageStore(writeImagesBack[4u], ivec2(_384), imageLoad(writeImages[4u], ivec2(_384)));
    imageStore(writeImagesBack[5u], ivec2(_384), imageLoad(writeImages[5u], ivec2(_384)));
    imageStore(writeImagesBack[6u], ivec2(_384), imageLoad(writeImages[6u], ivec2(_384)));
    imageStore(writeImagesBack[7u], ivec2(_384), imageLoad(writeImages[7u], ivec2(_384)));
    imageStore(writeImagesBack[8u], ivec2(_384), imageLoad(writeImages[8u], ivec2(_384)));
    imageStore(writeImagesBack[9u], ivec2(_384), imageLoad(writeImages[9u], ivec2(_384)));
    imageStore(writeImagesBack[10u], ivec2(_384), imageLoad(writeImages[10u], ivec2(_384)));
    imageStore(writeImagesBack[11u], ivec2(_384), imageLoad(writeImages[11u], ivec2(_384)));
    if ((!_131) && (imageLoad(writeBuffer[3], _114).w > 0.0))
    {
        float _474 = max(imageLoad(writeImages[0], ivec2(_384)).w, 1.0);
        imageStore(writeImagesBack[0], ivec2(_384), vec4(_230.xyz * _474, _474));
    }
}

