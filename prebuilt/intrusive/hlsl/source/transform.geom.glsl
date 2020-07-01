#version 450
#extension GL_EXT_nonuniform_qualifier : require
layout(triangles) in;
layout(max_vertices = 3, triangle_strip) out;

layout(push_constant, std430) uniform type_PushConstant_DrawInfo
{
    uvec4 data;
} drawInfo;

layout(location = 0) in vec4 in_var_POSITION[3];
layout(location = 1) in vec4 in_var_TEXCOORD[3];
layout(location = 2) in vec4 in_var_NORMAL[3];
layout(location = 3) in vec4 in_var_TANGENT[3];
layout(location = 4) in float in_var_PSIZE[3];
layout(location = 0) out vec4 out_var_POSITION;
layout(location = 1) out vec4 out_var_TEXCOORD;
layout(location = 2) out vec4 out_var_NORMAL;
layout(location = 3) out vec4 out_var_TANGENT;
layout(location = 4) out vec4 out_var_BINORMAL;

vec4 _41;

void main()
{
    vec4 _61 = in_var_TEXCOORD[1] - in_var_TEXCOORD[0];
    vec4 _62 = in_var_TEXCOORD[2] - in_var_TEXCOORD[0];
    vec3 _63 = (in_var_POSITION[1] - in_var_POSITION[0]).xyz;
    vec3 _64 = (in_var_POSITION[2] - in_var_POSITION[0]).xyz;
    vec3 _66 = normalize(cross(_63, _64));
    float _74 = 1.0 / ((_61.x * _62.y) - (_62.x * _61.y));
    vec3 _80 = ((_63 * _62.yyy) - (_64 * _61.yyy)) * _74;
    vec3 _86 = ((_63 * _62.xxx) - (_64 * _61.xxx)) * _74;
    uint _90 = uint(1) & 31u;
    uint _92 = (1u << _90) - 1u;
    bool _96 = !(((drawInfo.data.w >> _90) & _92) != 0u);
    vec4 _103;
    if (_96)
    {
        _103 = vec4(_66, 0.0);
    }
    else
    {
        _103 = in_var_NORMAL[0];
    }
    bool _109 = !(((drawInfo.data.w >> (uint(3) & 31u)) & _92) != 0u);
    vec4 _119;
    vec4 _120;
    if (_109)
    {
        _119 = vec4(_86.x, _86.y, _86.z, _41.w);
        _120 = vec4(_80.x, _80.y, _80.z, in_var_TANGENT[0].w);
    }
    else
    {
        vec3 _117 = cross(_103.xyz, in_var_TANGENT[0].xyz);
        _119 = vec4(_117.x, _117.y, _117.z, _41.w);
        _120 = in_var_TANGENT[0];
    }
    vec3 _122 = normalize(_103.xyz);
    vec3 _125 = normalize(_120.xyz);
    vec3 _128 = normalize(_119.xyz);
    out_var_POSITION = in_var_POSITION[0];
    out_var_TEXCOORD = in_var_TEXCOORD[0];
    out_var_NORMAL = vec4(_122.x, _122.y, _122.z, _103.w);
    out_var_TANGENT = vec4(_125.x, _125.y, _125.z, _120.w);
    out_var_BINORMAL = vec4(_128.x, _128.y, _128.z, _119.w);
    EmitVertex();
    vec4 _136;
    if (_96)
    {
        _136 = vec4(_66, 0.0);
    }
    else
    {
        _136 = in_var_NORMAL[1];
    }
    vec4 _146;
    vec4 _147;
    if (_109)
    {
        _146 = vec4(_86.x, _86.y, _86.z, _119.w);
        _147 = vec4(_80.x, _80.y, _80.z, in_var_TANGENT[1].w);
    }
    else
    {
        vec3 _142 = cross(_136.xyz, in_var_TANGENT[1].xyz);
        _146 = vec4(_142.x, _142.y, _142.z, _119.w);
        _147 = in_var_TANGENT[1];
    }
    vec3 _149 = normalize(_136.xyz);
    vec3 _152 = normalize(_147.xyz);
    vec3 _155 = normalize(_146.xyz);
    out_var_POSITION = in_var_POSITION[1];
    out_var_TEXCOORD = in_var_TEXCOORD[1];
    out_var_NORMAL = vec4(_149.x, _149.y, _149.z, _136.w);
    out_var_TANGENT = vec4(_152.x, _152.y, _152.z, _147.w);
    out_var_BINORMAL = vec4(_155.x, _155.y, _155.z, _146.w);
    EmitVertex();
    vec4 _163;
    if (_96)
    {
        _163 = vec4(_66, 0.0);
    }
    else
    {
        _163 = in_var_NORMAL[2];
    }
    vec4 _173;
    vec4 _174;
    if (_109)
    {
        _173 = vec4(_86.x, _86.y, _86.z, _146.w);
        _174 = vec4(_80.x, _80.y, _80.z, in_var_TANGENT[2].w);
    }
    else
    {
        vec3 _169 = cross(_163.xyz, in_var_TANGENT[2].xyz);
        _173 = vec4(_169.x, _169.y, _169.z, _146.w);
        _174 = in_var_TANGENT[2];
    }
    vec3 _176 = normalize(_163.xyz);
    vec3 _179 = normalize(_174.xyz);
    vec3 _182 = normalize(_173.xyz);
    out_var_POSITION = in_var_POSITION[2];
    out_var_TEXCOORD = in_var_TEXCOORD[2];
    out_var_NORMAL = vec4(_176.x, _176.y, _176.z, _163.w);
    out_var_TANGENT = vec4(_179.x, _179.y, _179.z, _174.w);
    out_var_BINORMAL = vec4(_182.x, _182.y, _182.z, _173.w);
    EmitVertex();
}

