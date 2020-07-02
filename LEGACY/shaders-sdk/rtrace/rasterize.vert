#version 460 core // #
#extension GL_GOOGLE_include_directive  : require
#include "./driver.glsl"

// Left Oriented
layout (location = 0) in vec3 iPosition;
layout (location = 1) in vec2 iTexcoord;
layout (location = 2) in vec3 iNormals;
layout (location = 3) in vec4 iTangent;
layout (location = 4) in vec4 iBinormal;

// Right Oriented
layout (location = 0) out vec4 fPosition;
layout (location = 1) out vec4 fTexcoord;
layout (location = 2) out vec4 fBarycent;
layout (location = 3) flat out uvec4 uData;

// 
const vec3 bary[3] = { vec3(1.f,0.f,0.f), vec3(0.f,1.f,0.f), vec3(0.f,0.f,1.f) };
void main() {
    // Full Instance ID of Node (BY GEOMETRY INSTATNCE!!)
    //const uint primitiveID = uint(gl_PrimitiveIndex.x);
    const uint geometryInstanceID = uint(gl_InstanceIndex.x);
    const uint nodeMeshID = drawInfo.data.x;
    const uint globalInstanceID = drawInfo.data.z;
    const uint idx = uint(gl_VertexIndex.x);

    // By Geometry Data
    mat3x4 matras = mat3x4(vec4(1.f,0.f.xxx),vec4(0.f,1.f,0.f.xx),vec4(0.f.xx,1.f,0.f));
    mat3x4 matra4 = rtxInstances[globalInstanceID].transform;
    if (hasTransform(meshInfo[nodeMeshID])) {
        matras = mat3x4(instances[nodeMeshID].transform[geometryInstanceID]);
    };

    // Native Normal Transform
    const mat3x3 normalTransform = inverse(transpose(regen3(matras))); // Geometry ID (Mesh)
    const mat3x3 normInTransform = inverse(transpose(regen3(matra4))); // Instance ID (Node)

    // Just Remap Into... 
      fTexcoord = vec4(iTexcoord.xy, 0.f.xx);
      fPosition = mul4(mul4(vec4(iPosition.xyz, 1.f), matras), matra4); // CORRECT
      fBarycent = vec4(bary[idx%3u], 0.f);
      uData = uvec4(gl_InstanceIndex, 0u.xxx);

    // 
    gl_Position = vec4(fPosition * modelview, 1.f) * projection;
    gl_Position.y *= -1.f;
};
