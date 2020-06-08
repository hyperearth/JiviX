
// 
struct RCData {
    uvec4 udata;
    vec4 fdata;
};

// BUT DEFAULT OVERRIDEN!
ivec2 launchSize = ivec2(1600, 1200);

// 
struct XHIT {
     vec4  diffuseColor;
     vec4 emissionColor;
     vec4  normalsColor;
     vec4 specularColor;

     vec4 geoNormal;
     vec4 mapNormal;
     vec4 origin;
     vec4 txcmid;
     
     vec4 direct;
     vec4 gTangent;
     vec4 gBinormal;
    uvec4 gIndices;
    //mat4x4 gObject;
};

// 
uint packed = 0u;
uvec2 seed = uvec2(0u.xx);

// RESERVED FOR OTHER OPERATIONS
vec3 refractive(in vec3 dir) {
    return dir;
};

// 
mat4x4 inverse(in mat3x4 imat) {
    //return inverse(transpose(mat4x4(imat[0],imat[1],imat[2],vec4(0.f,0.f,0.f,1.f))));
    return transpose(inverse(mat4x4(imat[0],imat[1],imat[2],vec4(0.f,0.f,0.f,1.f))));
};
