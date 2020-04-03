// #
// Re-Sampling
#define DIFFUSED 0 // Indrect diffuse
#define SAMPLING 1 // Positions
#define RESCOLOR 2 // ???
#define REFLECTS 3 // Specular color
#define RENDERED 4 // Denoised
//#define DIFFUSED_FLIP1 0//2
//#define SAMPLING_FLIP1 1//3

// Rasterization or First Step
#define COLORING 0 // Diffuse color
#define POSITION 1 // Noisy positions
#define RAYQUERY 2 // Indirect Illumination
#define RFLVALUE 3 // Reflection Color
#define EMISSION 4 // Emissive 
#define SPECULAR 5 // PBR Data
#define NORMALED 6 // Mapped Normals
#define SAMPLEPT 7 // Conservative Positions


// TODO: Materials
struct RayPayloadData {
    uvec4 udata;
    vec4 fdata;
    vec4 position;
    vec4 texcoord;

    vec4 normals;
    //vec4 normalm;
    vec4 tangent;
    vec4 binorml;
    

    //vec4 diffuse;
    //vec4 specular;
    //vec4 emission;
    //vec4 tangents;
};

struct Binding {
    uint32_t binding;
    uint32_t stride;
    uint32_t rate;
    //uint32_t reserved;
};

struct Attribute {
    uint32_t location;
    uint32_t binding;
    uint32_t format;
    uint32_t offset;
};

struct MaterialUnit {
    vec4 diffuse;
    vec4 specular;
    vec4 normals;
    vec4 emission;

    int diffuseTexture;
    int specularTexture;
    int normalsTexture;
    int emissionTexture;

    uvec4 udata;
};

struct MeshInfo {
    uint materialID;
    uint indexType;
    //uint hasIndex;
    uint prmCount;
    uint flags;
    //uint hasTransform;
};

bool hasTransform(in MeshInfo meshInfo){
    return bool(bitfieldExtract(meshInfo.flags,0,1));
};

bool hasNormal(in MeshInfo meshInfo){
    return bool(bitfieldExtract(meshInfo.flags,1,1));
};

bool hasTexcoord(in MeshInfo meshInfo){
    return bool(bitfieldExtract(meshInfo.flags,2,1));
};

bool hasTangent(in MeshInfo meshInfo){
    return bool(bitfieldExtract(meshInfo.flags,3,1));
};

// color space utils
const float HDR_GAMMA = 2.2f;
vec3 fromLinear(in vec3 linearRGB) { return mix(vec3(1.055)*pow(linearRGB, vec3(1.0/2.4)) - vec3(0.055), linearRGB * vec3(12.92), lessThan(linearRGB, vec3(0.0031308))); }
vec3 toLinear(in vec3 sRGB) { return mix(pow((sRGB + vec3(0.055))/vec3(1.055), vec3(2.4)), sRGB/vec3(12.92), lessThan(sRGB, vec3(0.04045))); }
vec4 fromLinear(in vec4 linearRGB) { return vec4(fromLinear(linearRGB.xyz), linearRGB.w); }
vec4 toLinear(in vec4 sRGB) { return vec4(toLinear(sRGB.xyz), sRGB.w); }


// Mesh Data Buffers
//layout (binding = 0, set = 0, scalar) buffer Data0 { uint8_t data[]; } mesh0[];
//layout (binding = 1, set = 0, scalar) buffer Data1 { uint8_t data[]; } mesh1[];
//layout (binding = 2, set = 0, scalar) buffer Data2 { uint8_t data[]; } mesh2[];
//layout (binding = 3, set = 0, scalar) buffer Data3 { uint8_t data[]; } mesh3[];
//layout (binding = 4, set = 0, scalar) buffer Data4 { uint8_t data[]; } mesh4[];
//layout (binding = 5, set = 0, scalar) buffer Data5 { uint8_t data[]; } mesh5[];
//layout (binding = 6, set = 0, scalar) buffer Data6 { uint8_t data[]; } mesh6[];
//layout (binding = 7, set = 0, scalar) buffer Data7 { uint8_t data[]; } mesh7[];

// 
layout (binding = 0, set = 0, r8ui) readonly uniform uimageBuffer mesh0[];
//layout (binding = 1, set = 0, r8ui) readonly uniform uimageBuffer mesh1[];
//layout (binding = 2, set = 0, r8ui) readonly uniform uimageBuffer mesh2[];
//layout (binding = 3, set = 0, r8ui) readonly uniform uimageBuffer mesh3[];
//layout (binding = 4, set = 0, r8ui) readonly uniform uimageBuffer mesh4[];
//layout (binding = 5, set = 0, r8ui) readonly uniform uimageBuffer mesh5[];
//layout (binding = 6, set = 0, r8ui) readonly uniform uimageBuffer mesh6[];
//layout (binding = 7, set = 0, r8ui) readonly uniform uimageBuffer mesh7[];

#ifdef GEN_QUAD_INDEX // For Minecraft
layout (binding = 8, set = 0, r8ui) uniform uimageBuffer index[]; // indices compatible 
#else
layout (binding = 8, set = 0, r8ui) readonly uniform uimageBuffer index[]; // indices compatible 
#endif

// Bindings Set (Binding 2 is Acceleration Structure, may implemented in Inline Version)
layout (binding = 0, set = 1, scalar) readonly buffer Bindings   { Binding   data[]; } bindings  [];
layout (binding = 1, set = 1, scalar) readonly buffer Attributes { Attribute data[]; } attributes[];
layout (binding = 3, set = 1, scalar) uniform Matrices {
    mat4 projection;    
    mat4 projectionInv; 
    mat3x4 modelview;   
    mat3x4 modelviewInv; 
    uvec4 mdata;        // mesh mutation or modification data
    //uvec2 tdata, rdata; // first for time, second for randoms
    uvec2 tdata;
    uvec2 rdata;
};

// 
layout (binding = 4, set = 1, scalar) readonly buffer InstanceTransform { mat3x4 transform[]; } instances[];
layout (binding = 5, set = 1, scalar) readonly buffer MeshData { MeshInfo meshInfo[]; };

struct RTXInstance {
    mat3x4 transform;
    uint32_t dontcare0;
    uint32_t dontcare1;
    uvec2 handle;
};

layout (binding = 6, set = 1, scalar) readonly buffer RTXInstances { RTXInstance rtxInstances[]; };
layout (binding = 7, set = 1, scalar) readonly buffer InstanceMaps { uint instanceID[]; } meshIDs[];  // uint globalInstanceID = meshID[meshID].instanceID[instanceID]

// 
layout(push_constant) uniform pushConstants { uvec4 data; } drawInfo;

// System Specified
#define meshID nonuniformEXT(nodeMeshID)

// System Specified
uint8_t load_u8(in uint offset, in uint binding, in uint nodeMeshID) {
    //if (binding == 0u) { return mesh0[meshID].data[offset]; };
    //if (binding == 1u) { return mesh1[meshID].data[offset]; };
    //if (binding == 2u) { return mesh2[meshID].data[offset]; };
    //if (binding == 3u) { return mesh3[meshID].data[offset]; };
    //if (binding == 4u) { return mesh4[meshID].data[offset]; };
    //if (binding == 5u) { return mesh5[meshID].data[offset]; };
    //if (binding == 6u) { return mesh6[meshID].data[offset]; };
    //if (binding == 7u) { return mesh7[meshID].data[offset]; };

    if (binding == 0u) { return uint8_t(imageLoad(mesh0[meshID], int(offset)).x); };
    //if (binding == 1u) { return uint8_t(imageLoad(mesh1[meshID], int(offset)).x); };
    //if (binding == 2u) { return uint8_t(imageLoad(mesh2[meshID], int(offset)).x); };
    //if (binding == 3u) { return uint8_t(imageLoad(mesh3[meshID], int(offset)).x); };
    //if (binding == 4u) { return uint8_t(imageLoad(mesh4[meshID], int(offset)).x); };
    //if (binding == 5u) { return uint8_t(imageLoad(mesh5[meshID], int(offset)).x); };
    //if (binding == 6u) { return uint8_t(imageLoad(mesh6[meshID], int(offset)).x); };
    //if (binding == 7u) { return uint8_t(imageLoad(mesh7[meshID], int(offset)).x); };
    if (binding == 8u) { return uint8_t(imageLoad(index[meshID], int(offset)).x); };

    return uint8_t(0u);
};

// System Specified
uint16_t load_u16(in uint offset, in uint binding, in uint nodeMeshID) {
    return pack16(u8vec2(load_u8(offset,binding,nodeMeshID),load_u8(offset+1u,binding,nodeMeshID)));
};

// System Specified
uint32_t load_u32(in uint offset, in uint binding, in uint nodeMeshID) {
    return pack32(u16vec2(load_u16(offset,binding,nodeMeshID),load_u16(offset+2u,binding,nodeMeshID)));
};

// TODO: Add Uint16_t, Uint32_t, Float16_t Support
vec4 get_vec4(in uint idx, in uint loc, in uint nodeMeshID) {
    Attribute attrib = attributes[meshID].data[loc];
    Binding  binding = bindings[meshID].data[attrib.binding];
    //Attribute attrib = attributes[loc].data[meshID];
    //Binding  binding = bindings[attrib.binding].data[meshID];
    uint32_t boffset = binding.stride * idx + attrib.offset;
    vec4 vec = vec4(0.f);
    
    // 
    //if (binding.stride >  0u) vec = vec4(0.f,0.f,1.f,0.f);
    if (binding.stride >  0u) vec[0] = uintBitsToFloat(load_u32(boffset +  0u, attrib.binding, nodeMeshID));
    if (binding.stride >  4u) vec[1] = uintBitsToFloat(load_u32(boffset +  4u, attrib.binding, nodeMeshID));
    if (binding.stride >  8u) vec[2] = uintBitsToFloat(load_u32(boffset +  8u, attrib.binding, nodeMeshID));
    if (binding.stride > 12u) vec[3] = uintBitsToFloat(load_u32(boffset + 12u, attrib.binding, nodeMeshID));
    
    // 
    return vec;
};

vec4 triangulate(in uvec3 indices, in uint loc, in uint nodeMeshID, in vec3 barycenter){
    const mat3x4 mc = mat3x4(
        get_vec4(indices[0],loc,nodeMeshID),
        get_vec4(indices[1],loc,nodeMeshID),
        get_vec4(indices[2],loc,nodeMeshID)
    );
    return mc*barycenter;
};

mat4x4 regen4(in mat3x4 T) {
    return mat4x4(T[0],T[1],T[2],vec4(0.f.xxx,1.f));
}

mat3x3 regen3(in mat3x4 T) {
    return mat3x3(T[0].xyz,T[1].xyz,T[2].xyz);
}

vec4 mul4(in vec4 v, in mat3x4 M) {
    return vec4(v*M,1.f);
}

#define IndexU8 1000265000
#define IndexU16 0
#define IndexU32 1





// Deferred and Rasterization Set
layout (binding = 0, set = 2) uniform sampler2D frameBuffers[];
layout (binding = 1, set = 2) uniform sampler2D renderBuffers[]; // New for FXAA shading (based on writeImages)
//layout (binding = 0, set = 2) uniform texture2D frameBuffers[];

// Sampling And Ray Tracing Set (also, re-used from previous frame)
layout (binding = 0, set = 3, rgba32f) uniform image2D writeImages[];

// Material Set
layout (binding = 0, set = 4) uniform sampler2D textures[];
layout (binding = 1, set = 4, scalar) readonly buffer Materials { MaterialUnit data[]; } materials[];

// 
float raySphereIntersect(in vec3 r0, in vec3 rd, in vec3 s0, in float sr) {
    float a = dot(rd, rd);
    vec3 s0_r0 = r0 - s0;
    float b = 2.0 * dot(rd, s0_r0);
    float c = dot(s0_r0, s0_r0) - (sr * sr);
    if (b*b - 4.0*a*c < 0.0) {
        return -1.0;
    }
    return (-b - sqrt((b*b) - 4.0*a*c))/(2.0*a);
};

// NEXT standard consts in current
// Ray tracing NEXT capable shader standard development planned begin in 2019 year
const float PHI = 1.6180339887498948482f;
const float SFN = 0.00000011920928955078125f, SFO = 1.f+SFN;//1.00000011920928955078125f;
const float INFINITY = 1e+5f, N_INFINITY = (INFINITY*(1.f-SFN));
const float PI = 3.1415926535897932384626422832795028841971f;
const float TWO_PI = 6.2831853071795864769252867665590057683943f;
const float SQRT_OF_ONE_THIRD = 0.5773502691896257645091487805019574556476f;
const float E = 2.7182818284590452353602874713526624977572f;
const float INV_PI = 0.3183098861837907f;
const float TWO_INV_PI = 0.6366197723675814f;
const float INV_TWO_PI = 0.15915494309189535f;

// A single iteration of Bob Jenkins' One-At-A-Time hashing algorithm.
uint hash( uint x ) {
    x += ( x << 10u );
    x ^= ( x >>  6u );
    x += ( x <<  3u );
    x ^= ( x >> 11u );
    x += ( x << 15u );
    return x;
}

// Compound versions of the hashing algorithm I whipped together.
uint counter = 0u;
uint hash( uvec2 v ) { return hash( hash(counter++) ^ v.x ^ hash(v.y)                         ); }
uint hash( uvec3 v ) { return hash( hash(counter++) ^ v.x ^ hash(v.y) ^ hash(v.z)             ); }
uint hash( uvec4 v ) { return hash( hash(counter++) ^ v.x ^ hash(v.y) ^ hash(v.z) ^ hash(v.w) ); }



// Construct a float with half-open range [0:1] using low 23 bits.
// All zeroes yields 0.0, all ones yields the next smallest representable value below 1.0.
float floatConstruct( uint m ) {
    const uint ieeeMantissa = 0x007FFFFFu; // binary32 mantissa bitmask
    const uint ieeeOne      = 0x3F800000u; // 1.0 in IEEE binary32

    m &= ieeeMantissa;                     // Keep only mantissa bits (fractional part)
    m |= ieeeOne;                          // Add fractional part to 1.0

    float  f = uintBitsToFloat( m );       // Range [1:2]
    return fract(f - 1.0);                 // Range [0:1]
};

highp vec2 halfConstruct ( in uint  m ) { return fract(unpackHalf2x16((m & 0x03FF03FFu) | (0x3C003C00u))-1.f); }



// Pseudo-random value in half-open range [0:1].
//float random( float x ) { return floatConstruct(hash(floatBitsToUint(x))); }
//float random( vec2  v ) { return floatConstruct(hash(floatBitsToUint(v))); }
//float random( vec3  v ) { return floatConstruct(hash(floatBitsToUint(v))); }
//float random( vec4  v ) { return floatConstruct(hash(floatBitsToUint(v))); }
  #define QLOCK2 clockRealtime2x32EXT()
  uint SCLOCK = 0u;
//#define QLOCK2 uvec2(0u,0u)

float random(               ) {         return floatConstruct(hash(QLOCK2)); }
float random( inout uvec2 s ) { s += 1; return floatConstruct(hash(uvec4(QLOCK2,s))); }
float random( inout uint  s ) { s += 1; return floatConstruct(hash(uvec3(QLOCK2,s))); }

vec2 random2(               ) {         return halfConstruct(hash(QLOCK2)); }
vec2 random2( inout uvec2 s ) { s += 1; return halfConstruct(hash(uvec4(QLOCK2,s))); }
vec2 random2( inout uint  s ) { s += 1; return halfConstruct(hash(uvec3(QLOCK2,s))); }

float staticRandom () { SCLOCK += 1; return floatConstruct(hash(uvec4(SCLOCK,0u,rdata.xy))); }
vec2  staticRandom2() { SCLOCK += 1; return  halfConstruct(hash(uvec4(SCLOCK,0u,rdata.xy))); }

/*
float rand( inout uvec2 seed ) {
	seed += uvec2(1);
    uvec2 q = 1103515245U * ( (seed >> 1U) ^ (seed.yx) );
    uint  n = 1103515245U * ( (q.x) ^ (q.y >> 3U) );
	return float(n) * (1.0 / float(0xffffffffU));
};
*/

vec2 lcts(in vec3 direct) { return vec2(fma(atan(direct.z,direct.x),INV_TWO_PI,0.5f),acos(-direct.y)*INV_PI); };
vec3 dcts(in vec2 hr) { 
    hr = fma(hr,vec2(TWO_PI,PI),vec2(-PI,0.f)); 
    const float up=-cos(hr.y),over=sqrt(fma(up,-up,1.f)); 
    return vec3(cos(hr.x)*over,up,sin(hr.x)*over); 
};

// geometric random generators
//vec3 randomSphere() { return dcts(random2()); };
//vec3 randomSphere(in uint  s) { return dcts(random2(s)); };
//vec3 randomSphere(in uvec2 s) { return dcts(random2(s)); };


vec3 randomSphere( inout uvec2 seed ) {
    float up = random(seed) * 2.0 - 1.0; // range: -1 to +1
    float over = sqrt( max(0.0, 1.0 - up * up) );
    float around = random(seed) * TWO_PI;
    return normalize(vec3(cos(around) * over, up, sin(around) * over));	
};

vec3 randomHemisphereCosine(inout uvec2 seed) {
    const vec2 hmsm = random2(seed);
    const float phi = hmsm.x * TWO_PI, up = sqrt(1.0f - hmsm.y), over = sqrt(fma(up,-up,1.f));
    return normalize(vec3(cos(phi)*over,up,sin(phi)*over));
};

/*
vec3 randomHemisphereCosine(inout uvec2 seed, in mat3x3 TBN) {
    return normalize(TBN * randomHemisphereCosine(seed).xzy);
};
*/

/*
vec3 randomHemisphereCosine(inout uvec2 seed, in vec3 tangent, in vec3 n) {
    vec3 up = abs(n.z) < 0.999f ? vec3(0.f, 0.f, 1.f) : vec3(1.f, 0.f, 0.f);
	vec3 tan_x = normalize(cross(up, n));
	vec3 tan_y = cross(n, tan_x);
    vec3 hemi = randomHemisphereCosine(seed);
    return normalize(hemi.x * tan_x + hemi.y * tan_y + n * hemi.z);
};
*/

vec3 randomHemisphereCosine( inout uvec2 seed, in mat3x3 TBN)
{
    float up = random(seed); // uniform distribution in hemisphere
    float over = sqrt(max(0.0, 1.0 - up * up));
    float around = random(seed) * TWO_PI;
    // from "Building an Orthonormal Basis, Revisited" http://jcgt.org/published/0006/01/01/
    float signf = TBN[2].z >= 0.0 ? 1.0 : -1.0;
    float a = -1.0 / (signf + TBN[2].z);
    float b = TBN[2].x * TBN[2].y * a;
    vec3 T = vec3( 1.0 + signf * TBN[2].x * TBN[2].x * a, signf * b, -signf * TBN[2].x );
    vec3 B = vec3( b, signf + TBN[2].y * TBN[2].y * a, -TBN[2].y );
    return normalize(cos(around) * over * T + sin(around) * over * B + up * TBN[2]);
};

vec3 reflectGlossy(inout uvec2 seed, in vec3 I, in mat3x3 TBN, in float gloss){
    return normalize(mix(reflect(I,TBN[2]), randomHemisphereCosine(seed,TBN), gloss*sqrt(random(seed))));
};

bvec4 and(in bvec4 a, in bvec4 b){
    return bvec4(a.x&&b.x,a.y&&b.y,a.z&&b.z,a.w&&b.w);
};

bvec4 or(in bvec4 a, in bvec4 b){
    return bvec4(a.x||b.x,a.y||b.y,a.z||b.z,a.w||b.w);
};

bvec3 and(in bvec3 a, in bvec3 b){
    return bvec3(a.x&&b.x,a.y&&b.y,a.z&&b.z);
};

bvec3 or(in bvec3 a, in bvec3 b){
    return bvec3(a.x||b.x,a.y||b.y,a.z||b.z);
};

bool fequal(in float a, in float b){
    return 
        a <= b + 0.0001f && 
        a >= b - 0.0001f;
};

bvec4 fequal(in vec4 a, in vec4 b){
    return and(
        lessThanEqual(a, b + 0.0001f),
        greaterThanEqual(a, b - 0.0001f));
};

bvec3 fequal(in vec3 a, in vec3 b){
    return and(
        lessThanEqual(a, b + 0.0001f),
        greaterThanEqual(a, b - 0.0001f));
};



struct Box { vec3 min, max; };

vec2 boxIntersect(in vec3 rayOrigin, in vec3 rayDir, in vec3 boxMin, in vec3 boxMax) {
    vec3 tMin = (boxMin - rayOrigin) / rayDir;
    vec3 tMax = (boxMax - rayOrigin) / rayDir;
    vec3 t1 = min(tMin, tMax);
    vec3 t2 = max(tMin, tMax);
    float tNear = max(max(t1.x, t1.y), t1.z);
    float tFar = min(min(t2.x, t2.y), t2.z);
    return vec2(tNear, tFar);
    //return 0.f.xx;
};

vec3 boxNormal(in vec3 point, in vec3 boxMin, in vec3 boxMax) {
    const float kEpsilon = 0.0001f;
	vec3 center = (boxMax + boxMin) * 0.5;
	vec3 size = (boxMax - boxMin) * 0.5;
	vec3 pc = point - center;
	vec3 normal = vec3(0.0f);
	normal += vec3(sign(pc.x), 0.0f, 0.0f) * step(abs(abs(pc.x) - size.x), kEpsilon);
	normal += vec3(0.0f, sign(pc.y), 0.0f) * step(abs(abs(pc.y) - size.y), kEpsilon);
	normal += vec3(0.0f, 0.0f, sign(pc.z)) * step(abs(abs(pc.z) - size.z), kEpsilon);
	return normalize(normal);
};

float planeIntersect(in vec3 l0, in vec3 r, in vec3 p0, in vec3 n){
    return dot(p0-l0,n)/dot(r,n);
};

vec3 exchange(inout vec3 orig, in vec3 data) {
    vec3 old = orig; orig = data; return old;
};

vec3 divW(in vec4 vect) { return vect.xyz/vect.w; };
vec3 divW(in vec3 vect) {return vect.xyz; };

vec3 world2screen(in vec3 origin){
    return divW(vec4(vec4(origin,1.f) * modelview, 1.f) * projection);
};

vec3 screen2world(in vec3 origin){
    return vec4(divW(vec4(origin,1.f) * projectionInv),1.f)*modelviewInv;
};


// Some Settings
//const vec3 gSkyColor = vec3(0.9f,0.98,0.999f); // TODO: Use 1.f and texture shading (include from rasterization)
#define gSkyColor vec3(0.9f,0.98,0.999f)
#define DIFFUSE_COLOR (diffuseColor.xyz)
//#define BACKSKY_COLOR gSignal.xyz = max(fma(gEnergy.xyz, (i > 0u ? gSkyColor : 1.f.xxx), gSignal.xyz),0.f.xxx), gEnergy *= 0.f
#define BACKSKY_COLOR gSignal.xyz = max(fma(gEnergy.xyz, gSkyColor, gSignal.xyz),0.f.xxx), gEnergy *= 0.f
