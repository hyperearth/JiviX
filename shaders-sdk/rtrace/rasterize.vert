#version 460 core // #
#extension GL_GOOGLE_include_directive  : require
#include "./driver.glsl"

// Left Oriented
layout (location = 0) in vec3 iPosition;
layout (location = 1) in vec2 iTexcoord;
layout (location = 2) in vec3 iNormals;
layout (location = 3) in vec4 iTangent;
layout (location = 4) in vec4 iBinormal;
//layout (location = 4) in vec4 fBinormal;

// Right Oriented
layout (location = 0) out vec4 fPosition;
layout (location = 1) out vec4 fTexcoord;
layout (location = 2) out vec4 fNormal;
layout (location = 3) out vec4 fTangent;
layout (location = 4) out vec4 fBinormal;
layout (location = 5) flat out uvec4 uData;

// 
void main() { // Cross-Lake
    const uint globalInstanceID = meshIDs[nonuniformEXT(drawInfo.data.x)].instanceID[gl_InstanceIndex];
    //const uint globalInstanceID = drawInfo.data.y;

    mat3x4 matras = mat3x4(instances[drawInfo.data.x].transform[gl_InstanceIndex]);
    if (!hasTransform(meshInfo[drawInfo.data.x])) {
        matras = mat3x4(vec4(1.f,0.f.xxx),vec4(0.f,1.f,0.f.xx),vec4(0.f.xx,1.f,0.f));
    };

    // By Geometry ID
    //const mat3x4 matras = mat3x4(vec4(1.f,0.f.xxx),vec4(0.f,1.f,0.f.xx),vec4(0.f.xx,1.f,0.f));
    const mat4x4 matra4 = mat4x4(matras[0],matras[1],matras[2],vec4(0.f.xxx,1.f));

    // By Insttance Data
    const mat3x4 transp = rtxInstances[globalInstanceID].transform;
    const mat4x4 trans4 = mat4x4(transp[0],transp[1],transp[2],vec4(0.f.xxx,1.f));

    // Native Normal Transform
    const mat4x4 normalTransform = (inverse(transpose(matra4)));
    const mat4x4 normInTransform = (inverse(transpose(trans4)));

    // Just Remap Into... 
      fTexcoord = vec4(iTexcoord.xy, 0.f.xx);
      fPosition = vec4(vec4(vec4(iPosition.xyz,1.f) * matras,1.f) * transp,1.f); // CORRECT
      fNormal = vec4(normalize((vec4(iNormals.xyz,0.f) * normalTransform * normInTransform).xyz),0.f);
      fTangent = vec4(iTangent.xyz,0.f) * normalTransform * normInTransform;
      fBinormal = vec4(iBinormal.xyz,0.f) * normalTransform * normInTransform;

    // 
    gl_Position = vec4(fPosition * modelview, 1.f) * projection;
};
