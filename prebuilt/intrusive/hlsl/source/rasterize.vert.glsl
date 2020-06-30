#version 450
#extension GL_EXT_nonuniform_qualifier : require
#extension GL_EXT_scalar_block_layout : require

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

layout(set = 1, binding = 7, scalar) uniform type_StructuredBuffer_mat3v4float
{
    layout(row_major) mat3x4 _m0[];
} tmatrices[];

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

layout(set = 1, binding = 10, std430) buffer type_RWStructuredBuffer_MeshInfo
{
    MeshInfo _m0[];
} meshInfo;

layout(set = 1, binding = 11, scalar) buffer type_RWStructuredBuffer_RTXInstance
{
    layout(row_major) RTXInstance _m0[];
} rtxInstances;

layout(push_constant, std430) uniform type_PushConstant_DrawInfo
{
    uvec4 data;
} drawInfo;

layout(location = 0) in vec3 in_var_LOCATION0;
layout(location = 1) in vec2 in_var_LOCATION1;
layout(location = 2) in vec3 in_var_LOCATION2;
layout(location = 3) in vec4 in_var_LOCATION3;
layout(location = 4) in vec4 in_var_LOCATION4;
layout(location = 0) out float out_var_PSIZE0;
layout(location = 1) out vec4 out_var_POSITION0;
layout(location = 2) out vec4 out_var_TEXCOORD0;
layout(location = 3) out vec4 out_var_TEXCOORD1;
layout(location = 4) flat out vec4 out_var_COLOR0;

void main()
{
    mat3x4 _100;
    if (((meshInfo._m0[drawInfo.data.x].flags >> (uint(0) & 31u)) & ((1u << (uint(1) & 31u)) - 1u)) != 0u)
    {
        _100 = tmatrices[drawInfo.data.x]._m0[uint(gl_InstanceIndex)];
    }
    else
    {
        _100 = mat3x4(vec4(1.0, 0.0, 0.0, 0.0), vec4(0.0, 1.0, 0.0, 0.0), vec4(0.0, 0.0, 1.0, 0.0));
    }
    vec4 _117 = vec4(vec4(vec4(in_var_LOCATION0, 1.0) * _100, 1.0) * rtxInstances._m0[drawInfo.data.z].transform, 1.0);
    vec4 _129 = vec4(_117 * pushed.modelview, 1.0) * pushed.projection;
    vec4 _132 = _129;
    _132.y = _129.y * (-1.0);
    gl_Position = _132;
    out_var_POSITION0 = _117;
    out_var_TEXCOORD0 = vec4(in_var_LOCATION1, 0.0, 0.0);
    out_var_TEXCOORD1 = vec4(0.0);
    out_var_COLOR0 = vec4(uvec4(uint(gl_InstanceIndex), 0u, 0u, 0u));
}

#version 450
#extension GL_EXT_nonuniform_qualifier : require
#extension GL_EXT_scalar_block_layout : require

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

layout(set = 1, binding = 7, scalar) uniform type_StructuredBuffer_mat3v4float
{
    layout(row_major) mat3x4 _m0[];
} tmatrices[];

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

layout(set = 1, binding = 10, std430) buffer type_RWStructuredBuffer_MeshInfo
{
    MeshInfo _m0[];
} meshInfo;

layout(set = 1, binding = 11, scalar) buffer type_RWStructuredBuffer_RTXInstance
{
    layout(row_major) RTXInstance _m0[];
} rtxInstances;

layout(push_constant, std430) uniform type_PushConstant_DrawInfo
{
    uvec4 data;
} drawInfo;

layout(location = 0) in vec3 in_var_LOCATION0;
layout(location = 1) in vec2 in_var_LOCATION1;
layout(location = 2) in vec3 in_var_LOCATION2;
layout(location = 3) in vec4 in_var_LOCATION3;
layout(location = 4) in vec4 in_var_LOCATION4;
layout(location = 0) out float out_var_PSIZE0;
layout(location = 1) out vec4 out_var_POSITION0;
layout(location = 2) out vec4 out_var_TEXCOORD0;
layout(location = 3) out vec4 out_var_TEXCOORD1;
layout(location = 4) flat out vec4 out_var_COLOR0;

void main()
{
    mat3x4 _100;
    if (((meshInfo._m0[drawInfo.data.x].flags >> (uint(0) & 31u)) & ((1u << (uint(1) & 31u)) - 1u)) != 0u)
    {
        _100 = tmatrices[drawInfo.data.x]._m0[uint(gl_InstanceIndex)];
    }
    else
    {
        _100 = mat3x4(vec4(1.0, 0.0, 0.0, 0.0), vec4(0.0, 1.0, 0.0, 0.0), vec4(0.0, 0.0, 1.0, 0.0));
    }
    vec4 _117 = vec4(vec4(vec4(in_var_LOCATION0, 1.0) * _100, 1.0) * rtxInstances._m0[drawInfo.data.z].transform, 1.0);
    vec4 _129 = vec4(_117 * pushed.modelview, 1.0) * pushed.projection;
    vec4 _132 = _129;
    _132.y = _129.y * (-1.0);
    gl_Position = _132;
    out_var_POSITION0 = _117;
    out_var_TEXCOORD0 = vec4(in_var_LOCATION1, 0.0, 0.0);
    out_var_TEXCOORD1 = vec4(0.0);
    out_var_COLOR0 = vec4(uvec4(uint(gl_InstanceIndex), 0u, 0u, 0u));
}

#version 450
#extension GL_EXT_nonuniform_qualifier : require
#extension GL_EXT_scalar_block_layout : require

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

layout(set = 1, binding = 7, scalar) uniform type_StructuredBuffer_mat3v4float
{
    layout(row_major) mat3x4 _m0[];
} tmatrices[];

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

layout(set = 1, binding = 10, std430) buffer type_RWStructuredBuffer_MeshInfo
{
    MeshInfo _m0[];
} meshInfo;

layout(set = 1, binding = 11, scalar) buffer type_RWStructuredBuffer_RTXInstance
{
    layout(row_major) RTXInstance _m0[];
} rtxInstances;

layout(push_constant, std430) uniform type_PushConstant_DrawInfo
{
    uvec4 data;
} drawInfo;

layout(location = 0) in vec3 in_var_LOCATION0;
layout(location = 1) in vec2 in_var_LOCATION1;
layout(location = 2) in vec3 in_var_LOCATION2;
layout(location = 3) in vec4 in_var_LOCATION3;
layout(location = 4) in vec4 in_var_LOCATION4;
layout(location = 0) out float out_var_PSIZE0;
layout(location = 1) out vec4 out_var_POSITION0;
layout(location = 2) out vec4 out_var_TEXCOORD0;
layout(location = 3) out vec4 out_var_TEXCOORD1;
layout(location = 4) flat out vec4 out_var_COLOR0;

void main()
{
    mat3x4 _100;
    if (((meshInfo._m0[drawInfo.data.x].flags >> (uint(0) & 31u)) & ((1u << (uint(1) & 31u)) - 1u)) != 0u)
    {
        _100 = tmatrices[drawInfo.data.x]._m0[uint(gl_InstanceIndex)];
    }
    else
    {
        _100 = mat3x4(vec4(1.0, 0.0, 0.0, 0.0), vec4(0.0, 1.0, 0.0, 0.0), vec4(0.0, 0.0, 1.0, 0.0));
    }
    vec4 _117 = vec4(vec4(vec4(in_var_LOCATION0, 1.0) * _100, 1.0) * rtxInstances._m0[drawInfo.data.z].transform, 1.0);
    vec4 _129 = vec4(_117 * pushed.modelview, 1.0) * pushed.projection;
    vec4 _132 = _129;
    _132.y = _129.y * (-1.0);
    gl_Position = _132;
    out_var_POSITION0 = _117;
    out_var_TEXCOORD0 = vec4(in_var_LOCATION1, 0.0, 0.0);
    out_var_TEXCOORD1 = vec4(0.0);
    out_var_COLOR0 = vec4(uvec4(uint(gl_InstanceIndex), 0u, 0u, 0u));
}

#version 450
#extension GL_EXT_nonuniform_qualifier : require
#extension GL_EXT_scalar_block_layout : require

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

layout(set = 1, binding = 7, scalar) uniform type_StructuredBuffer_mat3v4float
{
    layout(row_major) mat3x4 _m0[];
} tmatrices[];

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

layout(set = 1, binding = 10, std430) buffer type_RWStructuredBuffer_MeshInfo
{
    MeshInfo _m0[];
} meshInfo;

layout(set = 1, binding = 11, scalar) buffer type_RWStructuredBuffer_RTXInstance
{
    layout(row_major) RTXInstance _m0[];
} rtxInstances;

layout(push_constant, std430) uniform type_PushConstant_DrawInfo
{
    uvec4 data;
} drawInfo;

layout(location = 0) in vec3 in_var_LOCATION0;
layout(location = 1) in vec2 in_var_LOCATION1;
layout(location = 2) in vec3 in_var_LOCATION2;
layout(location = 3) in vec4 in_var_LOCATION3;
layout(location = 4) in vec4 in_var_LOCATION4;
layout(location = 0) out float out_var_PSIZE0;
layout(location = 1) out vec4 out_var_POSITION0;
layout(location = 2) out vec4 out_var_TEXCOORD0;
layout(location = 3) out vec4 out_var_TEXCOORD1;
layout(location = 4) flat out vec4 out_var_COLOR0;

void main()
{
    mat3x4 _100;
    if (((meshInfo._m0[drawInfo.data.x].flags >> (uint(0) & 31u)) & ((1u << (uint(1) & 31u)) - 1u)) != 0u)
    {
        _100 = tmatrices[drawInfo.data.x]._m0[uint(gl_InstanceIndex)];
    }
    else
    {
        _100 = mat3x4(vec4(1.0, 0.0, 0.0, 0.0), vec4(0.0, 1.0, 0.0, 0.0), vec4(0.0, 0.0, 1.0, 0.0));
    }
    vec4 _117 = vec4(vec4(vec4(in_var_LOCATION0, 1.0) * _100, 1.0) * rtxInstances._m0[drawInfo.data.z].transform, 1.0);
    vec4 _129 = vec4(_117 * pushed.modelview, 1.0) * pushed.projection;
    vec4 _132 = _129;
    _132.y = _129.y * (-1.0);
    gl_Position = _132;
    out_var_POSITION0 = _117;
    out_var_TEXCOORD0 = vec4(in_var_LOCATION1, 0.0, 0.0);
    out_var_TEXCOORD1 = vec4(0.0);
    out_var_COLOR0 = vec4(uvec4(uint(gl_InstanceIndex), 0u, 0u, 0u));
}

