#version 450
#extension GL_EXT_nonuniform_qualifier : require
#extension GL_EXT_scalar_block_layout : require

struct Binding
{
    uint bufvsd;
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

layout(set = 0, binding = 0, scalar) uniform type_ByteAddressBuffer
{
    uint _m0[];
} mesh0[];

layout(set = 0, binding = 2, scalar) uniform type_StructuredBuffer_Binding
{
    Binding _m0[];
} bindings[];

layout(set = 0, binding = 3, std140) uniform type_StructuredBuffer_Attribute
{
    Attribute _m0[];
} attributes[];

layout(location = 0) in vec3 in_var_LOCATION0;
layout(location = 1) in vec2 in_var_LOCATION1;
layout(location = 2) in vec3 in_var_LOCATION2;
layout(location = 3) in vec4 in_var_LOCATION3;
layout(location = 4) in vec4 in_var_LOCATION4;
layout(location = 5) in vec4 in_var_LOCATION5;
layout(location = 0) out vec4 out_var_POSITION;
layout(location = 1) out vec4 out_var_TEXCOORD;
layout(location = 2) out vec4 out_var_NORMAL;
layout(location = 3) out vec4 out_var_TANGENT;
layout(location = 4) out float out_var_PSIZE;

vec4 _65;

void main()
{
    uint _82 = (bindings[attributes[0u]._m0[uint(gl_VertexIndex)].binding]._m0[attributes[0u]._m0[uint(gl_VertexIndex)].binding].stride * uint(gl_VertexIndex)) + attributes[0u]._m0[uint(gl_VertexIndex)].offset;
    vec4 _91;
    if (bindings[attributes[0u]._m0[uint(gl_VertexIndex)].binding]._m0[attributes[0u]._m0[uint(gl_VertexIndex)].binding].stride > 0u)
    {
        vec4 _90 = vec4(0.0);
        _90.x = uintBitsToFloat(mesh0[bindings[attributes[0u]._m0[uint(gl_VertexIndex)].binding]._m0[attributes[0u]._m0[uint(gl_VertexIndex)].binding].bufvsd]._m0[_82 >> 2u]);
        _91 = _90;
    }
    else
    {
        _91 = vec4(0.0);
    }
    vec4 _101;
    if (bindings[attributes[0u]._m0[uint(gl_VertexIndex)].binding]._m0[attributes[0u]._m0[uint(gl_VertexIndex)].binding].stride > 4u)
    {
        vec4 _100 = _91;
        _100.y = uintBitsToFloat(mesh0[bindings[attributes[0u]._m0[uint(gl_VertexIndex)].binding]._m0[attributes[0u]._m0[uint(gl_VertexIndex)].binding].bufvsd]._m0[(_82 + 4u) >> 2u]);
        _101 = _100;
    }
    else
    {
        _101 = _91;
    }
    vec4 _111;
    if (bindings[attributes[0u]._m0[uint(gl_VertexIndex)].binding]._m0[attributes[0u]._m0[uint(gl_VertexIndex)].binding].stride > 8u)
    {
        vec4 _110 = _101;
        _110.z = uintBitsToFloat(mesh0[bindings[attributes[0u]._m0[uint(gl_VertexIndex)].binding]._m0[attributes[0u]._m0[uint(gl_VertexIndex)].binding].bufvsd]._m0[(_82 + 8u) >> 2u]);
        _111 = _110;
    }
    else
    {
        _111 = _101;
    }
    uint _123 = (bindings[attributes[1u]._m0[uint(gl_VertexIndex)].binding]._m0[attributes[1u]._m0[uint(gl_VertexIndex)].binding].stride * uint(gl_VertexIndex)) + attributes[1u]._m0[uint(gl_VertexIndex)].offset;
    vec4 _132;
    if (bindings[attributes[1u]._m0[uint(gl_VertexIndex)].binding]._m0[attributes[1u]._m0[uint(gl_VertexIndex)].binding].stride > 0u)
    {
        vec4 _131 = vec4(0.0);
        _131.x = uintBitsToFloat(mesh0[bindings[attributes[1u]._m0[uint(gl_VertexIndex)].binding]._m0[attributes[1u]._m0[uint(gl_VertexIndex)].binding].bufvsd]._m0[_123 >> 2u]);
        _132 = _131;
    }
    else
    {
        _132 = vec4(0.0);
    }
    vec4 _142;
    if (bindings[attributes[1u]._m0[uint(gl_VertexIndex)].binding]._m0[attributes[1u]._m0[uint(gl_VertexIndex)].binding].stride > 4u)
    {
        vec4 _141 = _132;
        _141.y = uintBitsToFloat(mesh0[bindings[attributes[1u]._m0[uint(gl_VertexIndex)].binding]._m0[attributes[1u]._m0[uint(gl_VertexIndex)].binding].bufvsd]._m0[(_123 + 4u) >> 2u]);
        _142 = _141;
    }
    else
    {
        _142 = _132;
    }
    uint _154 = (bindings[attributes[2u]._m0[uint(gl_VertexIndex)].binding]._m0[attributes[2u]._m0[uint(gl_VertexIndex)].binding].stride * uint(gl_VertexIndex)) + attributes[2u]._m0[uint(gl_VertexIndex)].offset;
    vec4 _163;
    if (bindings[attributes[2u]._m0[uint(gl_VertexIndex)].binding]._m0[attributes[2u]._m0[uint(gl_VertexIndex)].binding].stride > 0u)
    {
        vec4 _162 = vec4(0.0);
        _162.x = uintBitsToFloat(mesh0[bindings[attributes[2u]._m0[uint(gl_VertexIndex)].binding]._m0[attributes[2u]._m0[uint(gl_VertexIndex)].binding].bufvsd]._m0[_154 >> 2u]);
        _163 = _162;
    }
    else
    {
        _163 = vec4(0.0);
    }
    vec4 _173;
    if (bindings[attributes[2u]._m0[uint(gl_VertexIndex)].binding]._m0[attributes[2u]._m0[uint(gl_VertexIndex)].binding].stride > 4u)
    {
        vec4 _172 = _163;
        _172.y = uintBitsToFloat(mesh0[bindings[attributes[2u]._m0[uint(gl_VertexIndex)].binding]._m0[attributes[2u]._m0[uint(gl_VertexIndex)].binding].bufvsd]._m0[(_154 + 4u) >> 2u]);
        _173 = _172;
    }
    else
    {
        _173 = _163;
    }
    vec4 _183;
    if (bindings[attributes[2u]._m0[uint(gl_VertexIndex)].binding]._m0[attributes[2u]._m0[uint(gl_VertexIndex)].binding].stride > 8u)
    {
        vec4 _182 = _173;
        _182.z = uintBitsToFloat(mesh0[bindings[attributes[2u]._m0[uint(gl_VertexIndex)].binding]._m0[attributes[2u]._m0[uint(gl_VertexIndex)].binding].bufvsd]._m0[(_154 + 8u) >> 2u]);
        _183 = _182;
    }
    else
    {
        _183 = _173;
    }
    uint _195 = (bindings[attributes[3u]._m0[uint(gl_VertexIndex)].binding]._m0[attributes[3u]._m0[uint(gl_VertexIndex)].binding].stride * uint(gl_VertexIndex)) + attributes[3u]._m0[uint(gl_VertexIndex)].offset;
    vec4 _204;
    if (bindings[attributes[3u]._m0[uint(gl_VertexIndex)].binding]._m0[attributes[3u]._m0[uint(gl_VertexIndex)].binding].stride > 0u)
    {
        vec4 _203 = vec4(0.0);
        _203.x = uintBitsToFloat(mesh0[bindings[attributes[3u]._m0[uint(gl_VertexIndex)].binding]._m0[attributes[3u]._m0[uint(gl_VertexIndex)].binding].bufvsd]._m0[_195 >> 2u]);
        _204 = _203;
    }
    else
    {
        _204 = vec4(0.0);
    }
    vec4 _214;
    if (bindings[attributes[3u]._m0[uint(gl_VertexIndex)].binding]._m0[attributes[3u]._m0[uint(gl_VertexIndex)].binding].stride > 4u)
    {
        vec4 _213 = _204;
        _213.y = uintBitsToFloat(mesh0[bindings[attributes[3u]._m0[uint(gl_VertexIndex)].binding]._m0[attributes[3u]._m0[uint(gl_VertexIndex)].binding].bufvsd]._m0[(_195 + 4u) >> 2u]);
        _214 = _213;
    }
    else
    {
        _214 = _204;
    }
    vec4 _224;
    if (bindings[attributes[3u]._m0[uint(gl_VertexIndex)].binding]._m0[attributes[3u]._m0[uint(gl_VertexIndex)].binding].stride > 8u)
    {
        vec4 _223 = _214;
        _223.z = uintBitsToFloat(mesh0[bindings[attributes[3u]._m0[uint(gl_VertexIndex)].binding]._m0[attributes[3u]._m0[uint(gl_VertexIndex)].binding].bufvsd]._m0[(_195 + 8u) >> 2u]);
        _224 = _223;
    }
    else
    {
        _224 = _214;
    }
    vec4 _229 = vec4(_111.xyz, 1.0);
    uvec4 _232 = uvec4(in_var_LOCATION5 * 255.0) & uvec4(255u);
    out_var_POSITION = _229;
    out_var_TEXCOORD = vec4(_142.x, _142.y, _65.z, _65.w);
    out_var_NORMAL = vec4(_183.xyz, uintBitsToFloat((((_232.x >> 0u) | (_232.y << 8u)) | (_232.z << 16u)) | (_232.w << 24u)));
    out_var_TANGENT = vec4(_224.xyz, 0.0);
    gl_Position = _229;
}

#version 450
#extension GL_EXT_nonuniform_qualifier : require
#extension GL_EXT_scalar_block_layout : require

struct Binding
{
    uint bufvsd;
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

layout(set = 0, binding = 0, scalar) uniform type_ByteAddressBuffer
{
    uint _m0[];
} mesh0[];

layout(set = 0, binding = 2, scalar) uniform type_StructuredBuffer_Binding
{
    Binding _m0[];
} bindings[];

layout(set = 0, binding = 3, std140) uniform type_StructuredBuffer_Attribute
{
    Attribute _m0[];
} attributes[];

layout(location = 0) in vec3 in_var_LOCATION0;
layout(location = 1) in vec2 in_var_LOCATION1;
layout(location = 2) in vec3 in_var_LOCATION2;
layout(location = 3) in vec4 in_var_LOCATION3;
layout(location = 4) in vec4 in_var_LOCATION4;
layout(location = 5) in vec4 in_var_LOCATION5;
layout(location = 0) out vec4 out_var_POSITION;
layout(location = 1) out vec4 out_var_TEXCOORD;
layout(location = 2) out vec4 out_var_NORMAL;
layout(location = 3) out vec4 out_var_TANGENT;
layout(location = 4) out float out_var_PSIZE;

vec4 _65;

void main()
{
    uint _82 = (bindings[attributes[0u]._m0[uint(gl_VertexIndex)].binding]._m0[attributes[0u]._m0[uint(gl_VertexIndex)].binding].stride * uint(gl_VertexIndex)) + attributes[0u]._m0[uint(gl_VertexIndex)].offset;
    vec4 _91;
    if (bindings[attributes[0u]._m0[uint(gl_VertexIndex)].binding]._m0[attributes[0u]._m0[uint(gl_VertexIndex)].binding].stride > 0u)
    {
        vec4 _90 = vec4(0.0);
        _90.x = uintBitsToFloat(mesh0[bindings[attributes[0u]._m0[uint(gl_VertexIndex)].binding]._m0[attributes[0u]._m0[uint(gl_VertexIndex)].binding].bufvsd]._m0[_82 >> 2u]);
        _91 = _90;
    }
    else
    {
        _91 = vec4(0.0);
    }
    vec4 _101;
    if (bindings[attributes[0u]._m0[uint(gl_VertexIndex)].binding]._m0[attributes[0u]._m0[uint(gl_VertexIndex)].binding].stride > 4u)
    {
        vec4 _100 = _91;
        _100.y = uintBitsToFloat(mesh0[bindings[attributes[0u]._m0[uint(gl_VertexIndex)].binding]._m0[attributes[0u]._m0[uint(gl_VertexIndex)].binding].bufvsd]._m0[(_82 + 4u) >> 2u]);
        _101 = _100;
    }
    else
    {
        _101 = _91;
    }
    vec4 _111;
    if (bindings[attributes[0u]._m0[uint(gl_VertexIndex)].binding]._m0[attributes[0u]._m0[uint(gl_VertexIndex)].binding].stride > 8u)
    {
        vec4 _110 = _101;
        _110.z = uintBitsToFloat(mesh0[bindings[attributes[0u]._m0[uint(gl_VertexIndex)].binding]._m0[attributes[0u]._m0[uint(gl_VertexIndex)].binding].bufvsd]._m0[(_82 + 8u) >> 2u]);
        _111 = _110;
    }
    else
    {
        _111 = _101;
    }
    uint _123 = (bindings[attributes[1u]._m0[uint(gl_VertexIndex)].binding]._m0[attributes[1u]._m0[uint(gl_VertexIndex)].binding].stride * uint(gl_VertexIndex)) + attributes[1u]._m0[uint(gl_VertexIndex)].offset;
    vec4 _132;
    if (bindings[attributes[1u]._m0[uint(gl_VertexIndex)].binding]._m0[attributes[1u]._m0[uint(gl_VertexIndex)].binding].stride > 0u)
    {
        vec4 _131 = vec4(0.0);
        _131.x = uintBitsToFloat(mesh0[bindings[attributes[1u]._m0[uint(gl_VertexIndex)].binding]._m0[attributes[1u]._m0[uint(gl_VertexIndex)].binding].bufvsd]._m0[_123 >> 2u]);
        _132 = _131;
    }
    else
    {
        _132 = vec4(0.0);
    }
    vec4 _142;
    if (bindings[attributes[1u]._m0[uint(gl_VertexIndex)].binding]._m0[attributes[1u]._m0[uint(gl_VertexIndex)].binding].stride > 4u)
    {
        vec4 _141 = _132;
        _141.y = uintBitsToFloat(mesh0[bindings[attributes[1u]._m0[uint(gl_VertexIndex)].binding]._m0[attributes[1u]._m0[uint(gl_VertexIndex)].binding].bufvsd]._m0[(_123 + 4u) >> 2u]);
        _142 = _141;
    }
    else
    {
        _142 = _132;
    }
    uint _154 = (bindings[attributes[2u]._m0[uint(gl_VertexIndex)].binding]._m0[attributes[2u]._m0[uint(gl_VertexIndex)].binding].stride * uint(gl_VertexIndex)) + attributes[2u]._m0[uint(gl_VertexIndex)].offset;
    vec4 _163;
    if (bindings[attributes[2u]._m0[uint(gl_VertexIndex)].binding]._m0[attributes[2u]._m0[uint(gl_VertexIndex)].binding].stride > 0u)
    {
        vec4 _162 = vec4(0.0);
        _162.x = uintBitsToFloat(mesh0[bindings[attributes[2u]._m0[uint(gl_VertexIndex)].binding]._m0[attributes[2u]._m0[uint(gl_VertexIndex)].binding].bufvsd]._m0[_154 >> 2u]);
        _163 = _162;
    }
    else
    {
        _163 = vec4(0.0);
    }
    vec4 _173;
    if (bindings[attributes[2u]._m0[uint(gl_VertexIndex)].binding]._m0[attributes[2u]._m0[uint(gl_VertexIndex)].binding].stride > 4u)
    {
        vec4 _172 = _163;
        _172.y = uintBitsToFloat(mesh0[bindings[attributes[2u]._m0[uint(gl_VertexIndex)].binding]._m0[attributes[2u]._m0[uint(gl_VertexIndex)].binding].bufvsd]._m0[(_154 + 4u) >> 2u]);
        _173 = _172;
    }
    else
    {
        _173 = _163;
    }
    vec4 _183;
    if (bindings[attributes[2u]._m0[uint(gl_VertexIndex)].binding]._m0[attributes[2u]._m0[uint(gl_VertexIndex)].binding].stride > 8u)
    {
        vec4 _182 = _173;
        _182.z = uintBitsToFloat(mesh0[bindings[attributes[2u]._m0[uint(gl_VertexIndex)].binding]._m0[attributes[2u]._m0[uint(gl_VertexIndex)].binding].bufvsd]._m0[(_154 + 8u) >> 2u]);
        _183 = _182;
    }
    else
    {
        _183 = _173;
    }
    uint _195 = (bindings[attributes[3u]._m0[uint(gl_VertexIndex)].binding]._m0[attributes[3u]._m0[uint(gl_VertexIndex)].binding].stride * uint(gl_VertexIndex)) + attributes[3u]._m0[uint(gl_VertexIndex)].offset;
    vec4 _204;
    if (bindings[attributes[3u]._m0[uint(gl_VertexIndex)].binding]._m0[attributes[3u]._m0[uint(gl_VertexIndex)].binding].stride > 0u)
    {
        vec4 _203 = vec4(0.0);
        _203.x = uintBitsToFloat(mesh0[bindings[attributes[3u]._m0[uint(gl_VertexIndex)].binding]._m0[attributes[3u]._m0[uint(gl_VertexIndex)].binding].bufvsd]._m0[_195 >> 2u]);
        _204 = _203;
    }
    else
    {
        _204 = vec4(0.0);
    }
    vec4 _214;
    if (bindings[attributes[3u]._m0[uint(gl_VertexIndex)].binding]._m0[attributes[3u]._m0[uint(gl_VertexIndex)].binding].stride > 4u)
    {
        vec4 _213 = _204;
        _213.y = uintBitsToFloat(mesh0[bindings[attributes[3u]._m0[uint(gl_VertexIndex)].binding]._m0[attributes[3u]._m0[uint(gl_VertexIndex)].binding].bufvsd]._m0[(_195 + 4u) >> 2u]);
        _214 = _213;
    }
    else
    {
        _214 = _204;
    }
    vec4 _224;
    if (bindings[attributes[3u]._m0[uint(gl_VertexIndex)].binding]._m0[attributes[3u]._m0[uint(gl_VertexIndex)].binding].stride > 8u)
    {
        vec4 _223 = _214;
        _223.z = uintBitsToFloat(mesh0[bindings[attributes[3u]._m0[uint(gl_VertexIndex)].binding]._m0[attributes[3u]._m0[uint(gl_VertexIndex)].binding].bufvsd]._m0[(_195 + 8u) >> 2u]);
        _224 = _223;
    }
    else
    {
        _224 = _214;
    }
    vec4 _229 = vec4(_111.xyz, 1.0);
    uvec4 _232 = uvec4(in_var_LOCATION5 * 255.0) & uvec4(255u);
    out_var_POSITION = _229;
    out_var_TEXCOORD = vec4(_142.x, _142.y, _65.z, _65.w);
    out_var_NORMAL = vec4(_183.xyz, uintBitsToFloat((((_232.x >> 0u) | (_232.y << 8u)) | (_232.z << 16u)) | (_232.w << 24u)));
    out_var_TANGENT = vec4(_224.xyz, 0.0);
    gl_Position = _229;
}

