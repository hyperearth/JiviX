#version 460
#extension GL_EXT_shader_explicit_arithmetic_types_int8 : require
#extension GL_EXT_shader_8bit_storage : require
#extension GL_EXT_shader_explicit_arithmetic_types_int16 : require
#extension GL_EXT_shader_16bit_storage : require
#extension GL_EXT_nonuniform_qualifier : require

struct Attribute
{
    uint location;
    uint binding;
    uint format;
    uint offset;
};

struct Binding
{
    uint bufvsd;
    uint stride;
    uint rate;
};

layout(set = 1, binding = 0, std430) readonly buffer MeshData
{
    uint8_t data[];
} buffers[];

layout(set = 0, binding = 2, std430) readonly buffer Attributes
{
    Attribute attributes[];
} _88;

layout(set = 0, binding = 1, std430) readonly buffer Bindings
{
    Binding bindings[];
} _101;

layout(push_constant, std430) uniform pushConstants
{
    uvec4 data;
} drawInfo;

layout(location = 1) out vec4 gTexcoord;
layout(location = 0) out vec4 gPosition;
layout(location = 2) out vec4 gNormal;
layout(location = 4) in vec4 iColor;
layout(location = 3) out vec4 gTangent;
layout(location = 0) in vec3 iPosition;
layout(location = 1) in vec2 iTexcoord;
layout(location = 2) in vec3 iNormals;
layout(location = 3) in vec4 iTangent;

uint8_t load_u8(uint offset, uint bufferID)
{
    uint _38 = bufferID;
    return buffers[nonuniformEXT(_38)].data[offset];
}

uint16_t load_u16(uint offset, uint bufferID)
{
    uint param = offset;
    uint param_1 = bufferID;
    uint param_2 = offset + 1u;
    uint param_3 = bufferID;
    return pack16(u8vec2(load_u8(param, param_1), load_u8(param_2, param_3)));
}

uint load_u32(uint offset, uint bufferID)
{
    uint param = offset;
    uint param_1 = bufferID;
    uint param_2 = offset + 2u;
    uint param_3 = bufferID;
    return packUint2x16(u16vec2(load_u16(param, param_1), load_u16(param_2, param_3)));
}

vec4 get_vec4(uint idx, uint loc)
{
    Attribute _93;
    _93.location = _88.attributes[loc].location;
    _93.binding = _88.attributes[loc].binding;
    _93.format = _88.attributes[loc].format;
    _93.offset = _88.attributes[loc].offset;
    Attribute attrib = _93;
    Binding _108;
    _108.bufvsd = _101.bindings[attrib.binding].bufvsd;
    _108.stride = _101.bindings[attrib.binding].stride;
    _108.rate = _101.bindings[attrib.binding].rate;
    Binding binding = _108;
    uint boffset = (binding.stride * idx) + attrib.offset;
    vec4 vec = vec4(0.0);
    if (binding.stride > 0u)
    {
        uint param = boffset + 0u;
        uint param_1 = binding.bufvsd;
        vec.x = uintBitsToFloat(load_u32(param, param_1));
    }
    if (binding.stride > 4u)
    {
        uint param_2 = boffset + 4u;
        uint param_3 = binding.bufvsd;
        vec.y = uintBitsToFloat(load_u32(param_2, param_3));
    }
    if (binding.stride > 8u)
    {
        uint param_4 = boffset + 8u;
        uint param_5 = binding.bufvsd;
        vec.z = uintBitsToFloat(load_u32(param_4, param_5));
    }
    if (binding.stride > 12u)
    {
        uint param_6 = boffset + 12u;
        uint param_7 = binding.bufvsd;
        vec.w = uintBitsToFloat(load_u32(param_6, param_7));
    }
    return vec;
}

void main()
{
    int IdxType = int(drawInfo.data.y) - 1;
    uint idx = uint(gl_VertexIndex);
    uint param = idx;
    uint param_1 = 0u;
    vec4 iPosition_1 = get_vec4(param, param_1);
    uint param_2 = idx;
    uint param_3 = 1u;
    vec4 iTexcoord_1 = get_vec4(param_2, param_3);
    uint param_4 = idx;
    uint param_5 = 2u;
    vec4 iNormals_1 = get_vec4(param_4, param_5);
    uint param_6 = idx;
    uint param_7 = 3u;
    vec4 iTangent_1 = get_vec4(param_6, param_7);
    uint param_8 = idx;
    uint param_9 = 4u;
    vec4 iBinormal = get_vec4(param_8, param_9);
    gTexcoord = vec4(iTexcoord_1.xy.x, iTexcoord_1.xy.y, gTexcoord.z, gTexcoord.w);
    gPosition = vec4(iPosition_1.xyz, 1.0);
    gNormal = vec4(iNormals_1.xyz, uintBitsToFloat(packUnorm4x8(iColor)));
    gTangent = vec4(iTangent_1.xyz, 0.0);
    gl_Position = vec4(iPosition_1.xyz, 1.0);
}

