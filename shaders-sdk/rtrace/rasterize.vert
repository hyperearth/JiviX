#version 460 core // #
#extension GL_GOOGLE_include_directive  : require
#include "./driver.glsl"

// Left Oriented
//layout (location = 0) in vec3 iPosition;
//layout (location = 1) in vec2 iTexcoord;
//layout (location = 2) in vec3 iNormals;
//layout (location = 3) in vec4 iTangent;
//layout (location = 4) in vec4 iBinormal;

// Right Oriented
layout (location = 0) out vec4 fPosition;
layout (location = 1) out vec4 fTexcoord;
layout (location = 2) out vec4 fNormal;
layout (location = 3) out vec4 fTangent;
layout (location = 4) out vec4 fBinormal;
layout (location = 5) flat out uvec4 uData;

// 
void main() {
    // Full Instance ID of Node (BY GEOMETRY INSTATNCE!!)
    const uint nodeMeshID = drawInfo.data.x; // Mesh ID from Node Mesh List (because indexing)
    const uint geometryInstanceID = gl_InstanceIndex; // TODO: Using In Ray Tracing (and Query) shaders!
    const uint globalInstanceID = meshIDs[nonuniformEXT(nodeMeshID)].instanceID[geometryInstanceID];

    // 
    const int IdxType = int(meshInfo[nodeMeshID].indexType)-1;
    uint32_t idx = uint32_t(gl_VertexIndex.x); // Default Index of Vertice
    if (IdxType == IndexU8 ) { idx = load_u8 (idx, 8u, nodeMeshID); };
    if (IdxType == IndexU16) { idx = load_u16(idx, 8u, nodeMeshID); };
    if (IdxType == IndexU32) { idx = load_u32(idx, 8u, nodeMeshID); };

    // Use Apple-Like Attributes
    const vec4 iPosition = get_vec4(idx, 0u, nodeMeshID);
    const vec4 iTexcoord = get_vec4(idx, 1u, nodeMeshID);
    const vec4 iNormals  = get_vec4(idx, 2u, nodeMeshID);
    const vec4 iTangent  = get_vec4(idx, 3u, nodeMeshID);
    const vec4 iBinormal = get_vec4(idx, 4u, nodeMeshID);

    // By Geometry Data
    mat3x4 matras = mat3x4(instances[nodeMeshID].transform[geometryInstanceID]);
    if (!hasTransform(meshInfo[nodeMeshID])) {
        matras = mat3x4(vec4(1.f,0.f.xxx),vec4(0.f,1.f,0.f.xx),vec4(0.f.xx,1.f,0.f));
    };

    // By Instance Data
    const mat3x4 matra4 = rtxInstances[globalInstanceID].transform;

    // Native Normal Transform
    const mat3x3 normalTransform = inverse(transpose(regen3(matras))); // Geometry ID (Mesh)
    const mat3x3 normInTransform = inverse(transpose(regen3(matra4))); // Instance ID (Node)

    // Just Remap Into... 
      fTexcoord = vec4(iTexcoord.xy, 0.f.xx);
      fPosition = mul4(mul4(vec4(iPosition.xyz, 1.f), matras), matra4); // CORRECT
      fNormal = vec4(normalize(iNormals.xyz * normalTransform * normInTransform), 0.f);
      fTangent = vec4(normalize(iTangent.xyz * normalTransform * normInTransform), 0.f);
      fBinormal = vec4(normalize(iBinormal.xyz * normalTransform * normInTransform), 0.f);

    // 
    gl_Position = vec4(fPosition * modelview, 1.f) * projection;
};
