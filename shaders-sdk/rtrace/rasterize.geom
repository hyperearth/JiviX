#version 460 core // #
#extension GL_GOOGLE_include_directive  : require
#include "./driver.glsl"

layout (triangles) in;
layout (triangle_strip, max_vertices = 3) out;

// 
layout (location = 0) in vec4 gPosition[];
layout (location = 1) in vec4 gTexcoord[];
layout (location = 2) in vec4 gNormal[];
layout (location = 3) in vec4 gTangent[];
//layout (location = 4) flat in ivec4 gIndexes;

// 
layout (location = 0) out vec4 fPosition;
layout (location = 1) out vec4 fTexcoord;
layout (location = 2) out vec4 fNormal;
layout (location = 3) out vec4 fTangent;
layout (location = 4) flat out uvec4 uData;

// 
void main() {
    const MaterialUnit unit = materials[0u].data[meshInfo[drawInfo.data.x].materialID];
    const vec4 dp1 = gPosition[1] - gPosition[0], dp2 = gPosition[2] - gPosition[0];
    const vec4 tx1 = gTexcoord[1] - gTexcoord[0], tx2 = gTexcoord[2] - gTexcoord[0];
    const vec3 normal = normalize(cross(dp1.xyz, dp2.xyz));

    [[unroll]] for (uint i=0u;i<3u;i++) {
        gl_Position = gl_in[i].gl_Position;
        fPosition = gPosition[i];
        fTexcoord = gTexcoord[i];
        fTangent = gTangent[i];
        fNormal = gNormal[i];

        const float coef = 1.f / (tx1.x * tx2.y - tx2.x * tx1.y);
        const vec3 tangent = (dp1.xyz * tx2.yyy + dp2.xyz * tx1.yyy) * coef;

        if (!hasNormal (meshInfo[drawInfo.data.x])) { fNormal  = vec4(normal,0.f); };
        if (!hasTangent(meshInfo[drawInfo.data.x])) { fTangent = vec4(tangent,0.f); };

        EmitVertex();
    };
    
    EndPrimitive();
};
