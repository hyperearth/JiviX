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

layout(set = 0, binding = 0, scalar) uniform type_RWByteAddressBuffer
{
    uint _m0[];
} buffers[];

layout(set = 0, binding = 1, std430) buffer type_RWStructuredBuffer_Binding
{
    Binding _m0[];
} bindings;

layout(set = 0, binding = 2, std430) buffer type_RWStructuredBuffer_Attribute
{
    Attribute _m0[];
} attributes;

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

vec4 _63;

void main()
{
    uint _77 = (bindings._m0[attributes._m0[0u].binding].stride * uint(gl_VertexIndex)) + attributes._m0[0u].offset;
    vec4 _86;
    if (bindings._m0[attributes._m0[0u].binding].stride > 0u)
    {
        vec4 _85 = vec4(0.0);
        _85.x = uintBitsToFloat(buffers[bindings._m0[attributes._m0[0u].binding].bufvsd]._m0[_77 >> 2u]);
        _86 = _85;
    }
    else
    {
        _86 = vec4(0.0);
    }
    vec4 _96;
    if (bindings._m0[attributes._m0[0u].binding].stride > 4u)
    {
        vec4 _95 = _86;
        _95.y = uintBitsToFloat(buffers[bindings._m0[attributes._m0[0u].binding].bufvsd]._m0[(_77 + 4u) >> 2u]);
        _96 = _95;
    }
    else
    {
        _96 = _86;
    }
    vec4 _106;
    if (bindings._m0[attributes._m0[0u].binding].stride > 8u)
    {
        vec4 _105 = _96;
        _105.z = uintBitsToFloat(buffers[bindings._m0[attributes._m0[0u].binding].bufvsd]._m0[(_77 + 8u) >> 2u]);
        _106 = _105;
    }
    else
    {
        _106 = _96;
    }
    uint _116 = (bindings._m0[attributes._m0[1u].binding].stride * uint(gl_VertexIndex)) + attributes._m0[1u].offset;
    vec4 _125;
    if (bindings._m0[attributes._m0[1u].binding].stride > 0u)
    {
        vec4 _124 = vec4(0.0);
        _124.x = uintBitsToFloat(buffers[bindings._m0[attributes._m0[1u].binding].bufvsd]._m0[_116 >> 2u]);
        _125 = _124;
    }
    else
    {
        _125 = vec4(0.0);
    }
    vec4 _135;
    if (bindings._m0[attributes._m0[1u].binding].stride > 4u)
    {
        vec4 _134 = _125;
        _134.y = uintBitsToFloat(buffers[bindings._m0[attributes._m0[1u].binding].bufvsd]._m0[(_116 + 4u) >> 2u]);
        _135 = _134;
    }
    else
    {
        _135 = _125;
    }
    uint _145 = (bindings._m0[attributes._m0[2u].binding].stride * uint(gl_VertexIndex)) + attributes._m0[2u].offset;
    vec4 _154;
    if (bindings._m0[attributes._m0[2u].binding].stride > 0u)
    {
        vec4 _153 = vec4(0.0);
        _153.x = uintBitsToFloat(buffers[bindings._m0[attributes._m0[2u].binding].bufvsd]._m0[_145 >> 2u]);
        _154 = _153;
    }
    else
    {
        _154 = vec4(0.0);
    }
    vec4 _164;
    if (bindings._m0[attributes._m0[2u].binding].stride > 4u)
    {
        vec4 _163 = _154;
        _163.y = uintBitsToFloat(buffers[bindings._m0[attributes._m0[2u].binding].bufvsd]._m0[(_145 + 4u) >> 2u]);
        _164 = _163;
    }
    else
    {
        _164 = _154;
    }
    vec4 _174;
    if (bindings._m0[attributes._m0[2u].binding].stride > 8u)
    {
        vec4 _173 = _164;
        _173.z = uintBitsToFloat(buffers[bindings._m0[attributes._m0[2u].binding].bufvsd]._m0[(_145 + 8u) >> 2u]);
        _174 = _173;
    }
    else
    {
        _174 = _164;
    }
    uint _184 = (bindings._m0[attributes._m0[3u].binding].stride * uint(gl_VertexIndex)) + attributes._m0[3u].offset;
    vec4 _193;
    if (bindings._m0[attributes._m0[3u].binding].stride > 0u)
    {
        vec4 _192 = vec4(0.0);
        _192.x = uintBitsToFloat(buffers[bindings._m0[attributes._m0[3u].binding].bufvsd]._m0[_184 >> 2u]);
        _193 = _192;
    }
    else
    {
        _193 = vec4(0.0);
    }
    vec4 _203;
    if (bindings._m0[attributes._m0[3u].binding].stride > 4u)
    {
        vec4 _202 = _193;
        _202.y = uintBitsToFloat(buffers[bindings._m0[attributes._m0[3u].binding].bufvsd]._m0[(_184 + 4u) >> 2u]);
        _203 = _202;
    }
    else
    {
        _203 = _193;
    }
    vec4 _213;
    if (bindings._m0[attributes._m0[3u].binding].stride > 8u)
    {
        vec4 _212 = _203;
        _212.z = uintBitsToFloat(buffers[bindings._m0[attributes._m0[3u].binding].bufvsd]._m0[(_184 + 8u) >> 2u]);
        _213 = _212;
    }
    else
    {
        _213 = _203;
    }
    vec4 _218 = vec4(_106.xyz, 1.0);
    uvec4 _221 = uvec4(in_var_LOCATION5 * 255.0) & uvec4(255u);
    out_var_POSITION = _218;
    out_var_TEXCOORD = vec4(_135.x, _135.y, _63.z, _63.w);
    out_var_NORMAL = vec4(_174.xyz, uintBitsToFloat((((_221.x >> 0u) | (_221.y << 8u)) | (_221.z << 16u)) | (_221.w << 24u)));
    out_var_TANGENT = vec4(_213.xyz, 0.0);
    gl_Position = _218;
}

