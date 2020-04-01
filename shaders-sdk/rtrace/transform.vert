#version 460 core // #
#extension GL_GOOGLE_include_directive  : require
#define TRANSFORM_FEEDBACK
#include "./driver.glsl"

// Left Oriented
layout (location = 0) in vec3 iPosition;
layout (location = 1) in vec2 iTexcoord;
layout (location = 2) in vec3 iNormals;
layout (location = 3) in vec4 iTangent;
layout (location = 4) in vec4 fBinormal;

// Right Oriented
layout (location = 0) out vec4 gPosition;
layout (location = 1) out vec4 gTexcoord;
layout (location = 2) out vec4 gNormal;
layout (location = 3) out vec4 gTangent;

// 
void main() { // Cross-Lake
    // Full Instance ID of Node (BY GEOMETRY INSTATNCE!!)
    //const uint nodeMeshID = drawInfo.data.x; // Mesh ID from Node Mesh List (because indexing)
    //const uint geometryInstanceID = gl_InstanceIndex; // TODO: Using In Ray Tracing (and Query) shaders!
    //const uint globalInstanceID = meshIDs[nonuniformEXT(nodeMeshID)].instanceID[geometryInstanceID];

    // 
    //const int IdxType = int(drawInfo.data[1])-1;
    //uint32_t idx = uint32_t(gl_VertexIndex.x); // Default Index of Vertice
    //if (IdxType == IndexU8 ) { idx = load_u8 (idx, 0u, true); };
    //if (IdxType == IndexU16) { idx = load_u16(idx, 0u, true); };
    //if (IdxType == IndexU32) { idx = load_u32(idx, 0u, true); };

    // Use Apple-Like Attributes
    //const vec4 iPosition = get_vec4(idx, 0u);
    //const vec4 iTexcoord = get_vec4(idx, 1u);
    //const vec4 iNormals  = get_vec4(idx, 2u);
    //const vec4 iTangent  = get_vec4(idx, 3u);
    //const vec4 iBinormal = get_vec4(idx, 4u);

    // 
    gTexcoord.xy = iTexcoord.xy;
    gPosition = vec4(iPosition.xyz,1.f);
    gNormal = vec4(iNormals.xyz,0.f);
    gTangent = vec4(iTangent.xyz,0.f);
    gl_Position = vec4(iPosition.xyz, 1.f);
};
