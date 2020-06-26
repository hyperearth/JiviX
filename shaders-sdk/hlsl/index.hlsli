#ifndef INDEX_HLSL
#define INDEX_HLSL

//
#include "./driver.hlsli"
#include "./matrix.hlsli"

// Ray-Tracing Data (With resampling output support!)
#define IW_INDIRECT 0  // Indrect Diffuse
#define IW_SMOOTHED 1  // Anti-Aliased diffuse colors
#define IW_REFLECLR 2  // Previous Frame Reflection
#define IW_TRANSPAR 3  // Semi-Transparent Mixing
#define IW_MATERIAL 4  // Texcoord, Material ID, Skybox Mask
#define IW_GEONORML 5  // Geometry Normals
#define IW_ADAPTIVE 6  // Adaptive Data (reflection length, etc.)
#define IW_RESERVED 7  // 
// Ray-Tracing Data (Without resampling output support!)
#define IW_GEOMETRY 8  
#define IW_MAPNORML 9  // Mapped Normals
#define IW_POSITION 10 // Ray-Traced Position (for resampling)

// Last Action Data (another binding only)
#define BW_INDIRECT 0  
#define BW_SMOOTHED 1  
#define BW_REFLECLR 2  
#define BW_TRANSPAR 3  
#define BW_MATERIAL 4  // Texcoord, Material ID, Skybox Mask
#define BW_GEONORML 5  // Geometry Normal
#define BW_ADAPTIVE 6  // Final Rendering Result
#define BW_RESERVED 7
#define BW_RENDERED 8  
#define BW_GROUNDPS 9  // Deep Layer!!
#define BW_POSITION 10 // Position Data


// From Rasterization Phase!
#define RS_MATERIAL 0
#define RS_GEOMETRY 1
#define RS_POSITION 2
#define RS_BARYCENT 3
//#define RS_DIFFUSED 4



// TODO: Materials
struct RayPayloadData {
    uint4 udata;
     float4 fdata;
     float4 position;
     float4 texcoord;

     float4 normals;
     float4 tangent;
     float4 binorml;
};

struct Binding {
    uint binding;
    uint stride;
    uint rate;
};

struct Attribute {
    uint location;
    uint binding;
    uint format;
    uint offset;
};

struct MaterialUnit {
    float4 diffuse;
    float4 specular;
    float4 normals;
    float4 emission;

    int diffuseTexture;
    int specularTexture;
    int normalsTexture;
    int emissionTexture;

    uint4 udata;
};

struct MeshInfo {
    uint materialID;
    uint indexType;
    uint primitiveCount;
    //uint indexID;

    uint flags;
};

// 
struct RTXInstance {
    float3x4 transform;
    uint instance_mask;
    uint offset_flags;
    uint2 handle;
};

// 
uint packUnorm2x16(in float2 fp) {
    uint2 up = uint2(fp * 65536.f);
    return (up.x&0xFFFFu) | ((up.y&0xFFFFu)<<16u);
};

// 
float2 unpackUnorm2x16(in uint up) {
    uint2 pu = uint2((up&0xFFFFu), ((up>>16u)&0xFFFFu));
    return float2(pu) / 65536.f;
};

uint bitfieldExtract(uint val, int off, int size) {
	// This built-in function is only support in OpenGL 4.0 and ES 3.1
	// Hopefully the shader compiler will get our meaning and emit the right instruction
	uint mask = uint((1 << size) - 1);
	return uint(val >> off) & mask;
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
float3 fromLinear(in float3 linearRGB) { return lerp(float3(1.055)*pow(linearRGB, float3(1.0/2.4)) - float3(0.055), linearRGB * float3(12.92), (linearRGB < float3(0.0031308f.xxx))); }
float3 toLinear(in float3 sRGB) { return lerp(pow((sRGB + float3(0.055))/float3(1.055), float3(2.4)), sRGB/float3(12.92), (sRGB < float3(0.04045.xxx))); }
float4 fromLinear(in float4 linearRGB) { return float4(fromLinear(linearRGB.xyz), linearRGB.w); }
float4 toLinear(in float4 sRGB) { return float4(toLinear(sRGB.xyz), sRGB.w); }

// 
// Vulkan  => DirectX 12
// binding == spaceN
// But arrays are allowed...
// TODO: dedicated sampler

// Due Updated Version 
//[[vk::ignore]] SamplerState dummySampler; 

// 
[[vk::binding(0,0)]] ByteAddressBuffer mesh0[] : register(t0, space0);
[[vk::binding(1,0)]] ByteAddressBuffer index[] : register(t0, space1);

// LSD Mapping (Shadows, Emission, Voxels, Ray-Tracing...)
[[vk::binding(2,0)]] StructuredBuffer<uint> map[] : register(u0, space2);
[[vk::binding(3,0)]] RWTexture2D<uint> mapImage[] : register(t0, space3);
[[vk::binding(4,0)]] Texture2D<float4> mapColor[] : register(t0, space4);

// Bindings Set (Binding 2 is Acceleration Structure, may implemented in Inline Version)
[[vk::binding(5,1)]] StructuredBuffer<Binding> bindings[] : register(u0, space5);
[[vk::binding(6,1)]] StructuredBuffer<Attribute> attributes[] : register(u0, space6);

// 
[[vk::binding(7,1)]] StructuredBuffer<float3x4> transforms[] : register(u0, space7);
[[vk::binding(8,1)]] StructuredBuffer<uint> materialID[] : register(u0, space8);

// 
struct Matrices {
    float4x4 projection;    
    float4x4 projectionInv; 
    float3x4 modelview;   
    float3x4 modelviewInv; 
    float3x4 modelviewPrev;   
    float3x4 modelviewPrevInv; 
    uint4 mdata;        // mesh mutation or modification data
    //uint2 tdata, rdata; // first for time, second for randoms
    uint2 tdata;
    uint2 rdata;
};

// 
[[vk::binding(9,1)]] ConstantBuffer<Matrices> pushed : register(b0, space9);
[[vk::binding(10,1)]] StructuredBuffer<MeshInfo> meshInfo : register(u0, space10);
[[vk::binding(11,1)]] StructuredBuffer<RTXInstance> rtxInstances : register(u0, space11);

// 
#ifdef ENABLE_AS
[[vk::binding(12,1)]] RaytracingAccelerationStructure MyAccelerationStructure : register(t0, space12);
#endif

// Deferred and Rasterization Set
[[vk::binding(13,2)]] Texture2D<float4>  frameBuffers[12u] : register(t0, space13); // Pre-resampled buffers
[[vk::binding(14,2)]] SamplerState samplers[4u] : register(t0, space14);
[[vk::binding(15,2)]] Texture2D<float4> rasterBuffers[ 8u] : register(t0, space15); // Used by rasterization

// 
[[vk::binding(16,3)]] RWTexture2D<float4> writeBuffer[] : register(t0, space16); // Pre-resampled buffers, For EDIT!
[[vk::binding(17,3)]] RWTexture2D<float4> writeImages[] : register(t0, space17); 
[[vk::binding(18,3)]] RWTexture2D<float4> writeImagesBack[] : register(t0, space18); 

// 
[[vk::binding(20,4)]] StructuredBuffer<MaterialUnit> materials[] : register(u0, space20);
[[vk::binding(21,4)]] Texture2D<float4> background : register(t0, space21);
[[vk::binding(22,4)]] Texture2D<float4> textures[] : register(t0, space22);

// 
uint getMeshID(in RTXInstance instance){
    return bitfieldExtract(instance.instance_mask, 0, 24); // only hack method support
};

// 
[[vk::push_constant]] ConstantBuffer<uint4> drawInfo : register(b0, space23);


// System Specified
#define meshID nonuniformEXT(nodeMeshID)

// System Specified
/*
uint8_t load_u8(in uint offset, in uint binding, in uint nodeMeshID) {
    if (binding == 0u) { return uint8_t(imageLoad(mesh0[meshID], int(offset)).x); };
    return uint8_t(0u);
};

// System Specified
uint16_t load_u16(in uint offset, in uint binding, in uint nodeMeshID) {
    return pack16(u8float2(load_u8(offset,binding,nodeMeshID),load_u8(offset+1u,binding,nodeMeshID)));
};
*/

// System Specified
uint load_u32(in uint offset, in uint binding, in uint nodeMeshID) {
    //return pack32(u16float2(load_u16(offset,binding,nodeMeshID),load_u16(offset+2u,binding,nodeMeshID)));
    return mesh0[nodeMeshID].Load(offset);
};

// TODO: Add Uint16_t, uint, Float16_t Support
float4 get_float4(in uint idx, in uint loc, in uint nodeMeshID) {
    Attribute attrib = attributes[nodeMeshID][loc];
    Binding  binding = bindings[nodeMeshID][attrib.binding];
    //Attribute attrib = attributes[loc].data[meshID];
    //Binding  binding = bindings[attrib.binding].data[meshID];
    uint boffset = binding.stride * idx + attrib.offset;
    float4 vec = float4(0.f);
    
    // 
    //if (binding.stride >  0u) vec = float4(0.f,0.f,1.f,0.f);
    if (binding.stride >  0u) vec[0] = asfloat(load_u32(boffset +  0u, attrib.binding, nodeMeshID));
    if (binding.stride >  4u) vec[1] = asfloat(load_u32(boffset +  4u, attrib.binding, nodeMeshID));
    if (binding.stride >  8u) vec[2] = asfloat(load_u32(boffset +  8u, attrib.binding, nodeMeshID));
    if (binding.stride > 12u) vec[3] = asfloat(load_u32(boffset + 12u, attrib.binding, nodeMeshID));
    
    // 
    return vec;
};

float4 triangulate(in uint3 indices, in uint loc, in uint nodeMeshID, in float3 barycenter){
    const float3x4 mc = float3x4(
        get_float4(indices[0],loc,nodeMeshID),
        get_float4(indices[1],loc,nodeMeshID),
        get_float4(indices[2],loc,nodeMeshID)
    );
    return mc*barycenter;
};

float4x4 regen4(in float3x4 T) {
    return float4x4(T[0],T[1],T[2],float4(0.f.xxx,1.f));
}

float3x3 regen3(in float3x4 T) {
    return float3x3(T[0].xyz,T[1].xyz,T[2].xyz);
}

float4 mul4(in float4 v, in float3x4 M) {
    return float4(v*M,1.f);
}

// 
#define IndexU8 1000265000
#define IndexU16 0
#define IndexU32 1


// 
float raySphereIntersect(in float3 r0, in float3 rd, in float3 s0, in float sr) {
    float a = dot(rd, rd);
    float3 s0_r0 = r0 - s0;
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
uint hash( uint2 v ) { return hash( hash(counter++) ^ v.x ^ hash(v.y)                         ); }
uint hash( uint3 v ) { return hash( hash(counter++) ^ v.x ^ hash(v.y) ^ hash(v.z)             ); }
uint hash( uint4 v ) { return hash( hash(counter++) ^ v.x ^ hash(v.y) ^ hash(v.z) ^ hash(v.w) ); }



// Construct a float with half-open range [0:1] using low 23 bits.
// All zeroes yields 0.0, all ones yields the next smallest representable value below 1.0.
float floatConstruct( uint m ) {
    const uint ieeeMantissa = 0x007FFFFFu; // binary32 mantissa bitmask
    const uint ieeeOne      = 0x3F800000u; // 1.0 in IEEE binary32

    m &= ieeeMantissa;                     // Keep only mantissa bits (fractional part)
    m |= ieeeOne;                          // Add fractional part to 1.0

    float  f = asfloat( m );       // Range [1:2]
    return frac(f - 1.0);          // Range [0:1]
};

float2 halfConstruct ( in uint  m ) { return frac(f16tof32((m & 0x03FF03FFu) | (0x3C003C00u))-1.f); }



// Pseudo-random value in half-open range [0:1].
//float random( float x ) { return floatConstruct(hash(floatBitsToUint(x))); }
//float random( float2  v ) { return floatConstruct(hash(floatBitsToUint(v))); }
//float random( float3  v ) { return floatConstruct(hash(floatBitsToUint(v))); }
//float random( float4  v ) { return floatConstruct(hash(floatBitsToUint(v))); }
  //#define QLOCK2 (clockRealtime2x32EXT()+clock2x32ARB())
  #define QLOCK2 0u.xx
  uint SCLOCK = 0u;
//#define QLOCK2 uint2(0u,0u)

float random(               ) {         return floatConstruct(hash(QLOCK2)); }
float random( inout uint2 s ) { s += 1; return floatConstruct(hash(uint4(QLOCK2,s))); }
float random( inout uint  s ) { s += 1; return floatConstruct(hash(uint3(QLOCK2,s))); }

float2 random2(               ) {         return halfConstruct(hash(QLOCK2)); }
float2 random2( inout uint2 s ) { s += 1; return halfConstruct(hash(uint4(QLOCK2,s))); }
float2 random2( inout uint  s ) { s += 1; return halfConstruct(hash(uint3(QLOCK2,s))); }

float staticRandom () { SCLOCK += 1; return floatConstruct(hash(uint4(SCLOCK,0u,pushed.rdata.xy))); }
float2  staticRandom2() { SCLOCK += 1; return halfConstruct(hash(uint4(SCLOCK,0u,pushed.rdata.xy))); }

// 
float2 lcts(in float3 direct) { return float2(fma(atan2(direct.z,direct.x),INV_TWO_PI,0.5f),acos(-direct.y)*INV_PI); };
float3 dcts(in float2 hr) { 
    hr = fma(hr,float2(TWO_PI,PI),float2(-PI,0.f)); 
    const float up=-cos(hr.y),over=sqrt(fma(up,-up,1.f)); 
    return float3(cos(hr.x)*over,up,sin(hr.x)*over); 
};

// geometric random generators
//float3 randomSphere() { return dcts(random2()); };
//float3 randomSphere(in uint  s) { return dcts(random2(s)); };
//float3 randomSphere(in uint2 s) { return dcts(random2(s)); };


float3 randomSphere( inout uint2 seed ) {
    float up = random(seed) * 2.0 - 1.0; // range: -1 to +1
    float over = sqrt( max(0.0, 1.0 - up * up) );
    float around = random(seed) * TWO_PI;
    return normalize(float3(cos(around) * over, up, sin(around) * over));	
};

float3 randomHemisphereCosine(inout uint2 seed) {
    const float2 hmsm = random2(seed);
    const float phi = hmsm.x * TWO_PI, up = sqrt(1.0f - hmsm.y), over = sqrt(fma(up,-up,1.f));
    return normalize(float3(cos(phi)*over,up,sin(phi)*over));
};

float2 flip(in float2 texcoord) { return float2(texcoord.x, 1.f - texcoord.y); };

/*
float3 randomHemisphereCosine(inout uint2 seed, in float3x3 TBN) {
    return normalize(TBN * randomHemisphereCosine(seed).xzy);
};
*/

/*
float3 randomHemisphereCosine(inout uint2 seed, in float3 tangent, in float3 n) {
    float3 up = abs(n.z) < 0.999f ? float3(0.f, 0.f, 1.f) : float3(1.f, 0.f, 0.f);
	float3 tan_x = normalize(cross(up, n));
	float3 tan_y = cross(n, tan_x);
    float3 hemi = randomHemisphereCosine(seed);
    return normalize(hemi.x * tan_x + hemi.y * tan_y + n * hemi.z);
};
*/

float3 randomHemisphereCosine( inout uint2 seed, in float3x3 TBN)
{
    float up = random(seed); // uniform distribution in hemisphere
    float over = sqrt(max(0.0, 1.0 - up * up));
    float around = random(seed) * TWO_PI;
    // from "Building an Orthonormal Basis, Revisited" http://jcgt.org/published/0006/01/01/
    float signf = TBN[2].z >= 0.0 ? 1.0 : -1.0;
    float a = -1.0 / (signf + TBN[2].z);
    float b = TBN[2].x * TBN[2].y * a;
    float3 T = float3( 1.0 + signf * TBN[2].x * TBN[2].x * a, signf * b, -signf * TBN[2].x );
    float3 B = float3( b, signf + TBN[2].y * TBN[2].y * a, -TBN[2].y );
    return normalize(cos(around) * over * T + sin(around) * over * B + up * TBN[2]);
};

float3 reflectGlossy(inout uint2 seed, in float3 I, in float3x3 TBN, in float gloss){
    return normalize(lerp(reflect(I,TBN[2]), randomHemisphereCosine(seed,TBN), gloss*sqrt(random(seed))));
};

bool4 and(in bool4 a, in bool4 b){
    return bool4(a.x&&b.x,a.y&&b.y,a.z&&b.z,a.w&&b.w);
};

bool4 or(in bool4 a, in bool4 b){
    return bool4(a.x||b.x,a.y||b.y,a.z||b.z,a.w||b.w);
};

bool3 and(in bool3 a, in bool3 b){
    return bool3(a.x&&b.x,a.y&&b.y,a.z&&b.z);
};

bool3 or(in bool3 a, in bool3 b){
    return bool3(a.x||b.x,a.y||b.y,a.z||b.z);
};

bool fequal(in float a, in float b){
    return 
        a <= b + 0.0001f && 
        a >= b - 0.0001f;
};

bool4 fequal(in float2 a, in float2 b){
    return a <= (b + 0.0001f) && a >= (b - 0.0001f);
};

bool4 fequal(in float3 a, in float3 b){
    return a <= (b + 0.0001f) && a >= (b - 0.0001f);
};

bool4 fequal(in float4 a, in float4 b){
    return a <= (b + 0.0001f) && a >= (b - 0.0001f);
};



struct Box { float3 min, max; };

float2 boxIntersect(in float3 rayOrigin, in float3 rayDir, in float3 boxMin, in float3 boxMax) {
    float3 tMin = (boxMin - rayOrigin) / rayDir;
    float3 tMax = (boxMax - rayOrigin) / rayDir;
    float3 t1 = min(tMin, tMax);
    float3 t2 = max(tMin, tMax);
    float tNear = max(max(t1.x, t1.y), t1.z);
    float tFar = min(min(t2.x, t2.y), t2.z);
    return float2(tNear, tFar);
    //return 0.f.xx;
};

float3 boxNormal(in float3 pnt, in float3 boxMin, in float3 boxMax) {
    const float kEpsilon = 0.0001f;
	float3 center = (boxMax + boxMin) * 0.5;
	float3 size = (boxMax - boxMin) * 0.5;
	float3 pc = pnt - center;
	float3 normal = float3(0.0f);
	normal += float3(sign(pc.x), 0.0f, 0.0f) * step(abs(abs(pc.x) - size.x), kEpsilon);
	normal += float3(0.0f, sign(pc.y), 0.0f) * step(abs(abs(pc.y) - size.y), kEpsilon);
	normal += float3(0.0f, 0.0f, sign(pc.z)) * step(abs(abs(pc.z) - size.z), kEpsilon);
	return normalize(normal);
};

float planeIntersect(in float3 l0, in float3 r, in float3 p0, in float3 n){
    return dot(p0-l0,n)/dot(r,n);
};

float3 exchange(inout float3 orig, in float3 data) {
    float3 old = orig; orig = data; return old;
};

float3 divW(in float4 vect) { return vect.xyz/vect.w; };
float3 divW(in float3 vect) {return vect.xyz; };

float3 world2screen(in float3 origin){
    return divW(mul(float4(mul(float4(origin,1.f), pushed.modelview), 1.f), pushed.projection));
};

float3 screen2world(in float3 origin){
    return mul(float4(divW(mul(float4(origin,1.f), pushed.projectionInv)), 1.f), pushed.modelviewInv);
};


// Some Settings
//const float3 gSkyColor = float3(0.9f,0.98,0.999f); // TODO: Use 1.f and texture shading (include from rasterization)
//#define gSkyColor float3(0.9f,0.98,0.999f)
#define DIFFUSE_COLOR (diffuseColor.xyz)
//#define BACKSKY_COLOR gSignal.xyz = max(fma(gEnergy.xyz, (i > 0u ? gSkyColor : 1.f.xxx), gSignal.xyz),0.f.xxx), gEnergy *= 0.f
//#define BACKSKY_COLOR gSignal.xyz = max(fma(gEnergy.xyz, gSkyColor, gSignal.xyz),0.f.xxx), gEnergy *= 0.f

float4 gSkyShader(in float3 raydir, in float3 origin) {
    //return float4(texture(background, flip(lcts(raydir.xyz))).xyz, 1.f);
    return background.Sample(samplers[3u], flip(lcts(raydir.xyz)).xy);
};

#endif
