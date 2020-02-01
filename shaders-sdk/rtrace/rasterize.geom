#version 460 core // #
#extension GL_GOOGLE_include_directive  : require
#include "./driver.glsl"

layout (triangles) in;
layout (triangle_strip, max_vertices = 3) out;

// 
layout (location = 0) in vec4 gPosition[];
layout (location = 1) in vec4 gTexcoord[];
layout (location = 2) in vec4 gNormal[];
layout (location = 3) in vec4 gTangents[];
//layout (location = 4) flat in ivec4 gIndexes;

// 
layout (location = 0) out vec4 fPosition;
layout (location = 1) out vec4 fTexcoord;
layout (location = 2) out vec4 fNormal;
layout (location = 3) out vec4 fTangents;

// 
void main() {
    const MaterialUnit unit = materials[0u].data[meshInfo[drawInfo.data.x].materialID];
    const mat3x3 vertex = mat3x3(gPosition[0], gPosition[1], gPosition[2]);
    const vec3 normal = normalize(cross(vertex[1] - vertex[0], vertex[2] - vertex[0]));

    [[unroll]] for (uint i=0u;i<3u;i++) {
        gl_Position = gl_in[i].gl_Position;
        fPosition = gPosition[i];
        fTexcoord = gTexcoord[i];
        fTangents = gTangents[i];
        if (hasNormal(meshInfo[drawInfo.data.x])) {
            fNormal = gNormal[i];
        } else {
            fNormal = vec4(normal,0.f);
        };
        EmitVertex();
    };
    
    EndPrimitive();
};
