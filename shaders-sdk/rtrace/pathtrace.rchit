#version 460 core // #
#extension GL_GOOGLE_include_directive : require
#extension GL_EXT_ray_tracing           : require
#include "./driver.glsl"

layout(location = 0) rayPayloadInEXT RayPayloadData PrimaryRay;
hitAttributeEXT vec2 baryCoord;

#define IndexU8 1000265000
#define IndexU16 0
#define IndexU32 1

// TODO: add Any-Hit shaders for remove transparent intersections
void main() {
    const uint globalInstanceID = gl_InstanceID;

    mat3x4 matras = mat3x4(instances[gl_InstanceCustomIndexEXT].transform[gl_HitKindEXT]);
    if (!hasTransform(meshInfo[gl_InstanceCustomIndexEXT])) {
        matras = mat3x4(vec4(1.f,0.f.xxx),vec4(0.f,1.f,0.f.xx),vec4(0.f.xx,1.f,0.f));
    };
    //const mat3x4 matras = mat3x4(vec4(1.f,0.f.xxx),vec4(0.f,1.f,0.f.xx),vec4(0.f.xx,1.f,0.f));
    const mat4x4 matra4 = mat4x4(matras[0],matras[1],matras[2],vec4(0.f.xxx,1.f));

    const mat3x4 transp = rtxInstances[globalInstanceID].transform;
    const mat4x4 trans4 = mat4x4(transp[0],transp[1],transp[2],vec4(0.f.xxx,1.f));

    const mat4x4 normalTransform = (inverse(transpose(matra4)));
    const mat4x4 normInTransform = (inverse(transpose(trans4)));

    // type definition
    int IdxType = int(meshInfo[gl_InstanceCustomIndexEXT].indexType)-1;
    uvec3 idx3 = uvec3(gl_PrimitiveID*3u+0u,gl_PrimitiveID*3u+1u,gl_PrimitiveID*3u+2u);
    if (IdxType == IndexU8 ) { idx3 = uvec3(load_u8 (idx3.x*1u, 8u, gl_InstanceCustomIndexEXT),load_u32(idx3.y*1u, 8u, gl_InstanceCustomIndexEXT),load_u32(idx3.z*1u, 8u, gl_InstanceCustomIndexEXT)); };
    if (IdxType == IndexU16) { idx3 = uvec3(load_u16(idx3.x*2u, 8u, gl_InstanceCustomIndexEXT),load_u16(idx3.y*2u, 8u, gl_InstanceCustomIndexEXT),load_u16(idx3.z*2u, 8u, gl_InstanceCustomIndexEXT)); };
    if (IdxType == IndexU32) { idx3 = uvec3(load_u32(idx3.x*4u, 8u, gl_InstanceCustomIndexEXT),load_u32(idx3.y*4u, 8u, gl_InstanceCustomIndexEXT),load_u32(idx3.z*4u, 8u, gl_InstanceCustomIndexEXT)); };

    // mesh definition 
    PrimaryRay.position.xyz = vec4(vec4(triangulate(idx3, 0u, gl_InstanceCustomIndexEXT,vec3(1.f-baryCoord.x-baryCoord.y,baryCoord)).xyz,1.f)*matras,1.f)*transp;
    PrimaryRay.fdata.xyz    = vec3(baryCoord, gl_HitTEXT);
    PrimaryRay.udata        = uvec4(idx3, gl_InstanceCustomIndexEXT);

    // 
    vec4 gNormal = vec4(triangulate(idx3, 2u, gl_InstanceCustomIndexEXT,vec3(1.f-baryCoord.x-baryCoord.y,baryCoord)).xyz,0.f);
    vec4 gTangent = vec4(triangulate(idx3, 3u, gl_InstanceCustomIndexEXT,vec3(1.f-baryCoord.x-baryCoord.y,baryCoord)).xyz,0.f);
    vec4 gTexcoord = vec4(triangulate(idx3, 1u, gl_InstanceCustomIndexEXT,vec3(1.f-baryCoord.x-baryCoord.y,baryCoord)).xyz,0.f);
    vec4 gBinormal = vec4(0.f.xxx,0.f);

    // 
    gNormal = vec4(normalize((vec4(gNormal.xyz,0.f) * normalTransform * normInTransform).xyz),0.f);
    gTangent = vec4(normalize((vec4(gTangent.xyz,0.f) * normalTransform * normInTransform).xyz),0.f);

    // 
    const mat3x3 mc = mat3x3(
        vec4(vec4(get_vec4(idx3[0], 0u, gl_InstanceCustomIndexEXT).xyz,1.f)*matras,1.f)*transp,
        vec4(vec4(get_vec4(idx3[1], 0u, gl_InstanceCustomIndexEXT).xyz,1.f)*matras,1.f)*transp,
        vec4(vec4(get_vec4(idx3[2], 0u, gl_InstanceCustomIndexEXT).xyz,1.f)*matras,1.f)*transp
    );

    // 
    const mat3x3 tx = mat3x3(
        vec4(get_vec4(idx3[0], 1u, gl_InstanceCustomIndexEXT).xyz,1.f),
        vec4(get_vec4(idx3[1], 1u, gl_InstanceCustomIndexEXT).xyz,1.f),
        vec4(get_vec4(idx3[2], 1u, gl_InstanceCustomIndexEXT).xyz,1.f)
    );

    
    /*
    // normals
    if (!(dot(gNormal.xyz,gNormal.xyz) > 0.001f && hasNormal(meshInfo[gl_InstanceCustomIndexEXT]))) {
        gNormal.xyz = normalize(cross(mc[1].xyz-mc[0].xyz,mc[2].xyz-mc[0].xyz));
    } else {
        gNormal.xyz = normalize((vec4(gNormal.xyz,0.f) * normalTransform * normInTransform).xyz);
    };

    // tangents
    if (!(dot(gTangent.xyz,gTangent.xyz) > 0.001f && hasTangent(meshInfo[gl_InstanceCustomIndexEXT]))) {
        gTangent .xyz = tangent-dot(gNormal.xyz,tangent.xyz)*gNormal.xyz;
        gBitnorml.xyz = binorml-dot(gNormal.xyz,binorml.xyz)*gNormal.xyz;
    } else {
        gTangent .xyz = (vec4(gTangent.xyz, 0.f) * normalTransform * normInTransform).xyz;
        gBitnorml.xyz = cross(gNormal.xyz,gTangent.xyz);
        //gBitnorml.xyz = gBitnorml.xyz - dot(gNormal.xyz,gBitnorml.xyz)*gNormal.xyz;
    };*/

    const vec3 dp1 = mc[1] - mc[0], dp2 = mc[2] - mc[0];
    const vec3 tx1 = tx[1] - tx[0], tx2 = tx[2] - tx[0];
    const float coef = 1.f / (tx1.x * tx2.y - tx2.x * tx1.y);
    const vec3 normal = normalize(cross(dp1.xyz, dp2.xyz));
    const vec3 tangent = (dp1.xyz * tx2.yyy - dp2.xyz * tx1.yyy) * coef;
    const vec3 binorml = (dp1.xyz * tx2.xxx - dp2.xyz * tx1.xxx) * coef;
    if (!hasNormal (meshInfo[gl_InstanceCustomIndexEXT])) { gNormal = vec4(normal, 0.f); };
    if (!hasTangent(meshInfo[gl_InstanceCustomIndexEXT])) { 
        gTangent .xyz = tangent;
        gBinormal.xyz = binorml;
    } else {
        gBinormal.xyz = cross(gNormal.xyz, gTangent.xyz);
        //gBinormal.xyz = gBinormal.xyz - dot(gNormal.xyz,gBinormal.xyz)*gNormal.xyz;
    };

    gTangent .xyz -= dot(gNormal.xyz,tangent.xyz)*gNormal.xyz;
    gBinormal.xyz -= dot(gNormal.xyz,binorml.xyz)*gNormal.xyz;


    // 
    const mat3x3 TBN = mat3x3(normalize(gTangent.xyz),normalize(gBinormal.xyz),normalize(gNormal.xyz));

    // 
    const MaterialUnit unit = materials[0u].data[meshInfo[gl_InstanceCustomIndexEXT].materialID];
    vec4  diffuseColor = toLinear(unit. diffuseTexture >= 0 ? texture(textures[nonuniformEXT(unit. diffuseTexture)],gTexcoord.xy) : unit.diffuse);
    vec4  normalsColor = unit. normalsTexture >= 0 ? texture(textures[nonuniformEXT(unit. normalsTexture)],gTexcoord.xy) : unit.normals;
    vec4 specularColor = unit.specularTexture >= 0 ? texture(textures[nonuniformEXT(unit.specularTexture)],gTexcoord.xy) : unit.specular;
    vec4 emissionColor = toLinear(unit.emissionTexture >= 0 ? texture(textures[nonuniformEXT(unit.emissionTexture)],gTexcoord.xy) : unit.emission);

    // 
    PrimaryRay.normals.xyz = normalize(gNormal.xyz);
    PrimaryRay.texcoord.xy = gTexcoord.xy;
    //PrimaryRay.diffuse     = diffuseColor;
    //PrimaryRay.specular    = specularColor;
    //PrimaryRay.emission    = emissionColor;
    PrimaryRay.binorml.xyz = normalize(gBinormal.xyz);
    PrimaryRay.tangent.xyz = normalize(gTangent.xyz);
    //PrimaryRay.normalm.xyz = normalize( TBN * normalize(normalsColor.xyz * 2.f - 1.f) );
};
