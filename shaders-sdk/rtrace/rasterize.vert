#version 460 core
#extension GL_GOOGLE_include_directive  : require
#include "./driver.glsl"

// Left Oriented
layout (location = 0) in vec3 iPosition;
layout (location = 1) in vec2 iTexcoords;
layout (location = 2) in vec3 iNormals;
layout (location = 3) in vec4 iTangents;

// Right Oriented
layout (location = 0) out vec4 gPosition;
layout (location = 1) out vec4 gTexcoords;
layout (location = 2) out vec4 gNormal;
layout (location = 3) out vec4 gTangents;

// 
void main() { // Cross-Lake
    mat3x4 matras = mat3x4(instances[drawInfo.data.x].transform[gl_InstanceIndex]);
    if (meshInfo[drawInfo.data.x].hasTransform == 0) {
        matras = mat3x4(vec4(1.f,0.f.xxx),vec4(0.f,1.f,0.f.xx),vec4(0.f.xx,1.f,0.f));
    };

    // 
    gTexcoords.xy = iTexcoords;
    //gPosition = vec4(vec4(vec4(iPosition.xyz,1.f) * rtxInstances[drawInfo.data.y].transform,1.f) * matras,1.f); // INVALID
      gPosition = vec4(vec4(vec4(iPosition.xyz,1.f) * matras,1.f) * rtxInstances[drawInfo.data.y].transform,1.f); // CORRECT
    gNormal.xyz = iNormals;
    gTangents = iTangents;

    // 
    gl_Position = vec4(gPosition * modelview, 1.f) * projection;
};
