#version 460 core // #
#extension GL_GOOGLE_include_directive  : require
#include "./driver.glsl"

// 
layout (location = 0) in vec4 fPosition;
layout (location = 1) in vec4 fTexcoord;
layout (location = 2) in vec4 fNormal;
layout (location = 3) in vec4 fTangent;
layout (location = 4) flat in uvec4 uData;

// 
layout (location = COLORING) out vec4 colored;
layout (location = POSITION) out vec4 samples;
layout (location = NORMALED) out vec4 normals;
layout (location = TANGENTS) out vec4 tangent;
layout (location = EMISSION) out vec4 emission;
layout (location = SPECULAR) out vec4 specular;

// 
void main() { // hasTexcoord(meshInfo[drawInfo.data.x])
    const MaterialUnit unit = materials[0u].data[meshInfo[drawInfo.data.x].materialID];
    vec4 diffuseColor = unit.diffuseTexture >= 0 ? texture(textures[nonuniformEXT(unit.diffuseTexture)],fTexcoord.xy,0) : unit.diffuse;
    vec4 normalsColor = unit.normalsTexture >= 0 ? texture(textures[nonuniformEXT(unit.normalsTexture)],fTexcoord.xy,0) : unit.normals;
    vec4 specularColor = unit.specularTexture >= 0 ? texture(textures[nonuniformEXT(unit.specularTexture)],fTexcoord.xy,0) : unit.specular;
    vec4 emissionColor = unit.emissionTexture >= 0 ? texture(textures[nonuniformEXT(unit.emissionTexture)],fTexcoord.xy,0) : unit.emission;
    
    // 
    vec3 gTangent = fTangent.xyz;
    vec3 gBinormal = cross(fNormal.xyz,fTangent.xyz);
    gBinormal -= dot(fNormal.xyz,gBinormal)*fNormal.xyz;
    
    mat3x3 TBN = mat3x3(normalize(gTangent.xyz),normalize(gBinormal),normalize(fNormal.xyz));
    vec3 gNormal = normalize(TBN*(normalsColor.xyz * 2.f - 1.f));

    if (diffuseColor.w > 0.001f) {
        colored = vec4(max(vec4(diffuseColor.xyz-clamp(emissionColor.xyz*emissionColor.w,0.f.xxx,1.f.xxx),0.f),0.f.xxxx).xyz,1.f);
        normals = vec4(gNormal.xyz,1.f);
        samples = vec4(fPosition.xyz,1.f);
        emission = vec4(emissionColor.xyz*emissionColor.w,1.f);
        specular = vec4(specularColor.xyz*specularColor.w,1.f);
        gl_FragDepth = gl_FragCoord.z;
        //emission = vec4(emissionColor.xyz,emissionColor.w);
    } else {
        colored = 0.f.xxxx;
        normals = vec4(0.f.xx,0.f.xx);
        samples = vec4(0.f.xxx,0.f.x);
        specular = 0.f.xxxx;
        emission = 0.f.xxxx;
        gl_FragDepth = 1.f;
    };
    //ivec2 txd = ivec2(gl_FragCoord.xy), txs = imageSize(writeImages[DIFFUSED]);
    //const vec4 dEmi = imageLoad(writeImages[DIFFUSED], ivec2(txd.x,txs.y-txd.y-1));
    //imageStore(writeImages[DIFFUSED], ivec2(txd.x,txs.y-txd.y-1), vec4(emissionColor.xyz*emissionColor.w,0.f)+dEmi);
};
