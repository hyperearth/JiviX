#version 460
#extension GL_EXT_shader_explicit_arithmetic_types_int8 : require
#extension GL_EXT_shader_8bit_storage : require
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

const vec3 _194[3] = vec3[](vec3(1.0, 0.0, 0.0), vec3(0.0, 1.0, 0.0), vec3(0.0, 0.0, 1.0));

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

layout(set = 1, binding = 11, std430) readonly buffer RTXInstances
{
    RTXInstance rtxInstances[];
} _113;

layout(set = 1, binding = 10, std430) readonly buffer MeshInfoData
{
    MeshInfo meshInfo[];
} _122;

layout(set = 1, binding = 7, std140) uniform InstanceTransform
{
    mat3x4 transform[16];
} instances[];

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
} _225;

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

layout(set = 1, binding = 8, scalar) uniform MeshMaterial
{
    uint materialID[16];
} geomMTs[1];

layout(set = 4, binding = 20, std430) readonly buffer Materials
{
    MaterialUnit materials[];
} _306;

layout(push_constant, std430) uniform pushConstants
{
    uvec4 data;
} drawInfo;

layout(set = 0, binding = 3, r32ui) uniform readonly uimage2D mapImage[1];
layout(set = 0, binding = 4) uniform texture2D mapColor[1];
layout(set = 1, binding = 12) uniform accelerationStructureNV Scene;
layout(set = 2, binding = 13) uniform texture2D frameBuffers[12];
layout(set = 2, binding = 14) uniform sampler samplers[4];
layout(set = 2, binding = 15) uniform texture2D rasterBuffers[8];
layout(set = 3, binding = 16, rgba32f) uniform readonly writeonly image2D writeBuffer[1];
layout(set = 3, binding = 17, rgba32f) uniform readonly writeonly image2D writeImages[1];
layout(set = 3, binding = 18, rgba32f) uniform readonly writeonly image2D writeImagesBack[1];
layout(set = 4, binding = 21) uniform texture2D background;
layout(set = 4, binding = 22) uniform texture2D textures[1];

layout(location = 1) out vec4 fTexcoord;
layout(location = 1) in vec2 iTexcoord;
layout(location = 0) out vec4 fPosition;
layout(location = 0) in vec3 iPosition;
layout(location = 2) out vec4 fBarycent;
layout(location = 3) flat out uvec4 uData;
layout(location = 2) in vec3 iNormals;
layout(location = 3) in vec4 iTangent;
layout(location = 4) in vec4 iBinormal;
uint counter;
uint SCLOCK;

bool hasTransform(MeshInfo meshInfo)
{
    return bitfieldExtract(meshInfo.flags, 0, 1) != 0u;
}

mat3 regen3(mat3x4 T)
{
    return mat3(vec3(T[0].xyz), vec3(T[1].xyz), vec3(T[2].xyz));
}

vec4 mul4(vec4 v, mat3x4 M)
{
    return vec4(v * M, 1.0);
}

void main()
{
    counter = 0u;
    SCLOCK = 0u;
    uint geometryInstanceID = uint(gl_InstanceIndex);
    uint nodeMeshID = drawInfo.data.x;
    uint globalInstanceID = drawInfo.data.z;
    uint idx = uint(gl_VertexIndex);
    mat3x4 matras = mat3x4(vec4(1.0, 0.0, 0.0, 0.0), vec4(0.0, 1.0, 0.0, 0.0), vec4(0.0, 0.0, 1.0, 0.0));
    mat3x4 matra4 = _113.rtxInstances[globalInstanceID].transform;
    MeshInfo _128;
    _128.materialID = _122.meshInfo[nodeMeshID].materialID;
    _128.indexType = _122.meshInfo[nodeMeshID].indexType;
    _128.primitiveCount = _122.meshInfo[nodeMeshID].primitiveCount;
    _128.flags = _122.meshInfo[nodeMeshID].flags;
    MeshInfo param = _128;
    if (hasTransform(param))
    {
        matras = mat3x4(instances[nodeMeshID].transform[geometryInstanceID][0], instances[nodeMeshID].transform[geometryInstanceID][1], instances[nodeMeshID].transform[geometryInstanceID][2]);
    }
    mat3x4 param_1 = matras;
    mat3 normalTransform = inverse(transpose(regen3(param_1)));
    mat3x4 param_2 = matra4;
    mat3 normInTransform = inverse(transpose(regen3(param_2)));
    fTexcoord = vec4(iTexcoord, vec2(0.0));
    vec4 param_3 = vec4(iPosition, 1.0);
    mat3x4 param_4 = matras;
    vec4 param_5 = mul4(param_3, param_4);
    mat3x4 param_6 = matra4;
    fPosition = mul4(param_5, param_6);
    fBarycent = vec4(_194[idx % 3u], 0.0);
    uData = uvec4(uint(gl_InstanceIndex), uvec3(0u));
    gl_Position = vec4(fPosition * _225.modelview, 1.0) * _225.projection;
    gl_Position.y *= (-1.0);
}

#version 460
#extension GL_EXT_shader_explicit_arithmetic_types_int8 : require
#extension GL_EXT_shader_8bit_storage : require
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

const vec3 _194[3] = vec3[](vec3(1.0, 0.0, 0.0), vec3(0.0, 1.0, 0.0), vec3(0.0, 0.0, 1.0));

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

layout(set = 1, binding = 11, std430) readonly buffer RTXInstances
{
    RTXInstance rtxInstances[];
} _113;

layout(set = 1, binding = 10, std430) readonly buffer MeshInfoData
{
    MeshInfo meshInfo[];
} _122;

layout(set = 1, binding = 7, std140) uniform InstanceTransform
{
    mat3x4 transform[16];
} instances[];

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
} _225;

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

layout(set = 1, binding = 8, scalar) uniform MeshMaterial
{
    uint materialID[16];
} geomMTs[1];

layout(set = 4, binding = 20, std430) readonly buffer Materials
{
    MaterialUnit materials[];
} _306;

layout(push_constant, std430) uniform pushConstants
{
    uvec4 data;
} drawInfo;

layout(set = 0, binding = 3, r32ui) uniform readonly uimage2D mapImage[1];
layout(set = 0, binding = 4) uniform texture2D mapColor[1];
layout(set = 1, binding = 12) uniform accelerationStructureNV Scene;
layout(set = 2, binding = 13) uniform texture2D frameBuffers[12];
layout(set = 2, binding = 14) uniform sampler samplers[4];
layout(set = 2, binding = 15) uniform texture2D rasterBuffers[8];
layout(set = 3, binding = 16, rgba32f) uniform readonly writeonly image2D writeBuffer[1];
layout(set = 3, binding = 17, rgba32f) uniform readonly writeonly image2D writeImages[1];
layout(set = 3, binding = 18, rgba32f) uniform readonly writeonly image2D writeImagesBack[1];
layout(set = 4, binding = 21) uniform texture2D background;
layout(set = 4, binding = 22) uniform texture2D textures[1];

layout(location = 1) out vec4 fTexcoord;
layout(location = 1) in vec2 iTexcoord;
layout(location = 0) out vec4 fPosition;
layout(location = 0) in vec3 iPosition;
layout(location = 2) out vec4 fBarycent;
layout(location = 3) flat out uvec4 uData;
layout(location = 2) in vec3 iNormals;
layout(location = 3) in vec4 iTangent;
layout(location = 4) in vec4 iBinormal;
uint counter;
uint SCLOCK;

bool hasTransform(MeshInfo meshInfo)
{
    return bitfieldExtract(meshInfo.flags, 0, 1) != 0u;
}

mat3 regen3(mat3x4 T)
{
    return mat3(vec3(T[0].xyz), vec3(T[1].xyz), vec3(T[2].xyz));
}

vec4 mul4(vec4 v, mat3x4 M)
{
    return vec4(v * M, 1.0);
}

void main()
{
    counter = 0u;
    SCLOCK = 0u;
    uint geometryInstanceID = uint(gl_InstanceIndex);
    uint nodeMeshID = drawInfo.data.x;
    uint globalInstanceID = drawInfo.data.z;
    uint idx = uint(gl_VertexIndex);
    mat3x4 matras = mat3x4(vec4(1.0, 0.0, 0.0, 0.0), vec4(0.0, 1.0, 0.0, 0.0), vec4(0.0, 0.0, 1.0, 0.0));
    mat3x4 matra4 = _113.rtxInstances[globalInstanceID].transform;
    MeshInfo _128;
    _128.materialID = _122.meshInfo[nodeMeshID].materialID;
    _128.indexType = _122.meshInfo[nodeMeshID].indexType;
    _128.primitiveCount = _122.meshInfo[nodeMeshID].primitiveCount;
    _128.flags = _122.meshInfo[nodeMeshID].flags;
    MeshInfo param = _128;
    if (hasTransform(param))
    {
        matras = mat3x4(instances[nodeMeshID].transform[geometryInstanceID][0], instances[nodeMeshID].transform[geometryInstanceID][1], instances[nodeMeshID].transform[geometryInstanceID][2]);
    }
    mat3x4 param_1 = matras;
    mat3 normalTransform = inverse(transpose(regen3(param_1)));
    mat3x4 param_2 = matra4;
    mat3 normInTransform = inverse(transpose(regen3(param_2)));
    fTexcoord = vec4(iTexcoord, vec2(0.0));
    vec4 param_3 = vec4(iPosition, 1.0);
    mat3x4 param_4 = matras;
    vec4 param_5 = mul4(param_3, param_4);
    mat3x4 param_6 = matra4;
    fPosition = mul4(param_5, param_6);
    fBarycent = vec4(_194[idx % 3u], 0.0);
    uData = uvec4(uint(gl_InstanceIndex), uvec3(0u));
    gl_Position = vec4(fPosition * _225.modelview, 1.0) * _225.projection;
    gl_Position.y *= (-1.0);
}

