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

mat4x4 regen4(in mat3x4 T) {
    return mat4x4(T[0],T[1],T[2],vec4(0.f.xxx,1.f));
}

mat3x3 regen3(in mat3x4 T) {
    return mat3x3(T[0].xyz,T[1].xyz,T[2].xyz);
}

vec4 mul4(in vec4 v, in mat3x4 M) {
    return vec4(v*M,1.f);
}

// 
void main() { // Cross-Lake
    const uint globalInstanceID = meshIDs[nonuniformEXT(drawInfo.data.x)].instanceID[gl_InstanceIndex];

    // By Geometry Data
    mat3x4 matras = mat3x4(instances[drawInfo.data.x].transform[gl_InstanceIndex]);
    if (!hasTransform(meshInfo[drawInfo.data.x])) {
        matras = mat3x4(vec4(1.f,0.f.xxx),vec4(0.f,1.f,0.f.xx),vec4(0.f.xx,1.f,0.f));
    };

    // By Instance Data
    const mat3x4 matra4 = rtxInstances[globalInstanceID].transform;

    // Native Normal Transform
    const mat3x3 normalTransform = inverse(transpose(regen3(matras)));
    const mat3x3 normInTransform = inverse(transpose(regen3(matra4)));

    // Just Remap Into... 
      fTexcoord = vec4(iTexcoord.xy, 0.f.xx);
      fPosition = mul4(mul4(vec4(iPosition.xyz, 1.f), matras), matra4); // CORRECT
      fNormal = vec4(normalize(iNormals.xyz * normalTransform * normInTransform), 0.f);
      fTangent = vec4(normalize(iTangent.xyz * normalTransform * normInTransform), 0.f);
      fBinormal = vec4(normalize(iBinormal.xyz * normalTransform * normInTransform), 0.f);

    // 
    gl_Position = vec4(fPosition * modelview, 1.f) * projection;
};
