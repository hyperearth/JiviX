#version 460
#extension GL_EXT_shader_explicit_arithmetic_types_int8 : require
#extension GL_EXT_shader_8bit_storage : require
#extension GL_EXT_nonuniform_qualifier : require
layout(triangles) in;
layout(max_vertices = 3, triangle_strip) out;

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

layout(set = 1, binding = 0, std430) readonly buffer MeshData
{
    uint8_t data[];
} buffers[];

layout(set = 0, binding = 1, std430) readonly buffer Bindings
{
    Binding bindings[];
} _218;

layout(set = 0, binding = 2, std430) readonly buffer Attributes
{
    Attribute attributes[];
} _223;

layout(push_constant, std430) uniform pushConstants
{
    uvec4 data;
} drawInfo;

layout(location = 0) in vec4 gPosition[3];
layout(location = 1) in vec4 gTexcoord[3];
layout(location = 0) out vec4 fPosition;
layout(location = 1) out vec4 fTexcoord;
layout(location = 3) out vec4 fTangent;
layout(location = 3) in vec4 gTangent[3];
layout(location = 2) out vec4 fNormal;
layout(location = 2) in vec4 gNormal[3];
layout(location = 4) out vec4 fBinormal;

bool hasNormal()
{
    return bitfieldExtract(drawInfo.data.w, 1, 1) != 0u;
}

bool hasTangent()
{
    return bitfieldExtract(drawInfo.data.w, 3, 1) != 0u;
}

void main()
{
    vec4 dp1 = gPosition[1] - gPosition[0];
    vec4 dp2 = gPosition[2] - gPosition[0];
    vec4 tx1 = gTexcoord[1] - gTexcoord[0];
    vec4 tx2 = gTexcoord[2] - gTexcoord[0];
    vec3 normal = normalize(cross(dp1.xyz, dp2.xyz));
    for (uint i = 0u; i < 3u; i++)
    {
        gl_Position = gl_in[i].gl_Position;
        fPosition = gPosition[i];
        fTexcoord = gTexcoord[i];
        fTangent = gTangent[i];
        fNormal = gNormal[i];
        float coef = 1.0 / ((tx1.x * tx2.y) - (tx2.x * tx1.y));
        vec3 tangent = ((dp1.xyz * tx2.yyy) - (dp2.xyz * tx1.yyy)) * coef;
        vec3 binorml = ((dp1.xyz * tx2.xxx) - (dp2.xyz * tx1.xxx)) * coef;
        if (!hasNormal())
        {
            fNormal = vec4(normal, 0.0);
        }
        if (!hasTangent())
        {
            fTangent = vec4(tangent.x, tangent.y, tangent.z, fTangent.w);
            fBinormal = vec4(binorml.x, binorml.y, binorml.z, fBinormal.w);
        }
        else
        {
            vec3 _188 = cross(fNormal.xyz, fTangent.xyz);
            fBinormal = vec4(_188.x, _188.y, _188.z, fBinormal.w);
        }
        vec3 _193 = normalize(fNormal.xyz);
        fNormal = vec4(_193.x, _193.y, _193.z, fNormal.w);
        vec3 _198 = normalize(fTangent.xyz);
        fTangent = vec4(_198.x, _198.y, _198.z, fTangent.w);
        vec3 _203 = normalize(fBinormal.xyz);
        fBinormal = vec4(_203.x, _203.y, _203.z, fBinormal.w);
        EmitVertex();
    }
    EndPrimitive();
}

