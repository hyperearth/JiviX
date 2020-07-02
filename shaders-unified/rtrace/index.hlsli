// #
#ifndef INDEX_HLSL
#define INDEX_HLSL

#include "./driver.hlsli"

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
float3x4 getMT3x4(in float4x3 data) { return transpose(data); };
float3x4 getMT3x4(in float3x4 data) { return data; };
float4x4 getMT4x4(in float4x4 data) { return data; };
float4x4 getMT4x4(in float3x4 data) { return float4x4(data, float4(0.f,0.f,0.f,1.f)); };
float4x4 getMT4x4(in float4x3 data) { return float4x4(transpose(data), float4(0.f,0.f,0.f,1.f)); };

// 
#if defined(HLSL) || !defined(GLSL)
float uintBitsToFloat(in uint a) { return asfloat(a); };
uint floatBitsToUint(in float a) { return asuint(a); };

// 
uint bitfieldExtract(uint val, int off, int size) {
	// This built-in function is only support in OpenGL 4.0 and ES 3.1
	// Hopefully the shader compiler will get our meaning and emit the right instruction
	uint mask = uint((1 << size) - 1);
	return uint(val >> off) & mask;
};

// 
uint packUint2x16(in uint2 up) {
    return (up.x&0xFFFFu) | ((up.y&0xFFFFu)<<16u);
};

// 
uint2 unpackUint2x16(in uint up) {
    return uint2((up&0xFFFFu), ((up>>16u)&0xFFFFu));
};

// 
uint packUnorm2x16(in float2 fp) {
    return packUint2x16(uint2(fp * 65536.f));
};

// 
float2 unpackUnorm2x16(in uint up) {
    return float2(unpackUint2x16(up)) / 65536.f;
};

#define SHARED groupshared
#define STATIC static 
#define mix lerp
#else // GLSL-side

#define SHARED shared
#define STATIC  
float3 mul(in float3x4 m, in float4 a) { return a * m; };
float3 mul(in float4x3 m, in float4 a) { return m * a; };
float3 mul(in float3 a, in float3x4 m) { return m * a; };
float3 mul(in float3 a, in float4x3 m) { return a * m; };
float4 mul(in float4x4 m, in float4 a) { return a * m; };
float4 mul(in float4 a, in float4x4 m) { return m * a; };
float3 mul(in float3x3 m, in float3 a) { return a * m; };
float3 mul(in float3 a, in float3x3 m) { return m * a; };
float asfloat(in uint a) { return uintBitsToFloat(a); };
uint asuint(in float a) { return floatBitsToUint(a); };
#endif

// 
bool hasTransform(in MeshInfo meshInfo){
    return bool(bitfieldExtract(meshInfo.flags,0,1));
};

// 
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
STATIC const float HDR_GAMMA = 2.2f;

// 
#ifdef GLSL
float3 fromLinear(in float3 linearRGB) { return mix(float3(1.055)*pow(linearRGB, float3(1.0/2.4)) - float3(0.055), linearRGB * float3(12.92), lessThan(linearRGB, float3(0.0031308))); }
float3 toLinear(in float3 sRGB) { return mix(pow((sRGB + float3(0.055))/float3(1.055), float3(2.4)), sRGB/float3(12.92), lessThan(sRGB, float3(0.04045))); }
float4 fromLinear(in float4 linearRGB) { return float4(fromLinear(linearRGB.xyz), linearRGB.w); }
float4 toLinear(in float4 sRGB) { return float4(toLinear(sRGB.xyz), sRGB.w); }
#else
float3 fromLinear(in float3 linearRGB) { return lerp(float3(1.055f.xxx)*pow(linearRGB, float3((1.0f/2.4f).xxx)) - float3(0.055f.xxx), linearRGB * float3(12.92f.xxx), (linearRGB < float3(0.0031308f.xxx))); }
float3 toLinear(in float3 sRGB) { return lerp(pow(sRGB + float3(0.055f.xxx)/float3(1.055f.xxx), float3(2.4f.xxx)), sRGB/float3(12.92f.xxx), (sRGB < float3(0.04045f.xxx))); }
float4 fromLinear(in float4 linearRGB) { return float4(fromLinear(linearRGB.xyz), linearRGB.w); }
float4 toLinear(in float4 sRGB) { return float4(toLinear(sRGB.xyz), sRGB.w); }
#endif

// 
// Vulkan  => DirectX 12
// binding == spaceN
// But arrays are allowed...

#ifdef GLSL
// 
//layout (binding = 0, set = 0) uniform utextureBuffer mesh0[];
  layout (binding = 0, set = 0) readonly buffer MeshData { uint8_t data[]; } mesh0[]; 
//layout (binding = 0, set = 0, r8ui) readonly uniform uimageBuffer mesh0[];
//layout (binding = 1, set = 0, r8ui) readonly uniform uimageBuffer index[];

// LSD Mapping (Shadows, Emission, Voxels, Ray-Tracing...)
layout (binding = 2, set = 0, scalar) readonly buffer MapData { uint data[]; } map;
layout (binding = 3, set = 0, r32ui)  readonly uniform  uimage2D mapImage[];
layout (binding = 4, set = 0)                  uniform texture2D mapColor[];

// Bindings Set (Binding 2 is Acceleration Structure, may implemented in Inline Version)
layout (binding = 5, set = 1, scalar) uniform Bindings   { Binding   data[8u]; } bindings  [];
layout (binding = 6, set = 1, scalar) uniform Attributes { Attribute data[8u]; } attributes[];

// 
  layout (binding = 7, set = 1, scalar) uniform InstanceTransform { float3x4 transform[16u]; } instances[];
  layout (binding = 8, set = 1, scalar) uniform MeshMaterial { uint materialID[16u]; } geomMTs[];
//layout (binding = 9, set = 1, scalar) uniform InstanceMaps { uint instanceID[]; } meshIDs[]; // uint globalInstanceID = meshID[meshID].instanceID[instanceID]

layout (binding = 9, set = 1, scalar) uniform Matrices {
    layout(column_major) float4x4 projection;    
    layout(column_major) float4x4 projectionInv; 
    layout(column_major) float3x4 modelview;   
    layout(column_major) float3x4 modelviewInv;
    layout(column_major) float3x4 modelviewPrev;   
    layout(column_major) float3x4 modelviewPrevInv; 
    uint4 mdata;        // mesh mutation or modification data
    //uint2 tdata, rdata; // first for time, second for randoms
    uint2 tdata;
    uint2 rdata;
} pushed;
layout (binding = 10, set = 1, scalar) readonly buffer MeshInfoData { MeshInfo meshInfo[]; };
layout (binding = 11, set = 1, scalar) readonly buffer RTXInstances { RTXInstance rtxInstances[]; };

#ifdef ENABLE_AS
layout (binding = 12, set = 1 ) uniform accelerationStructureEXT Scene;
#endif

// Deferred and Rasterization Set
layout (binding = 13, set = 2) uniform texture2D  frameBuffers[12u]; // Pre-resampled buffers
//layout (binding = 14, set = 2) uniform sampler2D renderBuffers[12u]; // Used by final rendering
layout (binding = 14, set = 2) uniform sampler   samplers[4u];
layout (binding = 15, set = 2) uniform texture2D rasterBuffers[ 8u]; // Used by rasterization

// 
layout (binding = 16, set = 3, rgba32f) uniform image2D writeBuffer[]; // Pre-resampled buffers, For EDIT!
layout (binding = 17, set = 3, rgba32f) uniform image2D writeImages[]; 
layout (binding = 18, set = 3, rgba32f) uniform image2D writeImagesBack[]; 

// Material Set
layout (binding = 20, set = 4, scalar) readonly buffer Materials { MaterialUnit materials[]; };
//layout (binding = 20, set = 4, scalar) uniform Materials { MaterialUnit data[64u]; } materials[];
layout (binding = 21, set = 4) uniform texture2D background;
layout (binding = 22, set = 4) uniform texture2D textures[];

// 
layout (push_constant) uniform pushConstants { uint4 data; } drawInfo;

//
#define textureSample(b, s, c) texture(sampler2D(b, s), c)
#define textureLodSample(b, s, c, m) textureLod(sampler2D(b, s), c, m)
//#define textureSample(b, s, c, m) textureLod(sampler2D(b, s), c, m)
#else

struct Matrices {
    column_major float4x4 projection;    
    column_major float4x4 projectionInv; 
    column_major float3x4 modelview;   
    column_major float3x4 modelviewInv;
    column_major float3x4 modelviewPrev;   
    column_major float3x4 modelviewPrevInv; 
    uint4 mdata;        // mesh mutation or modification data
    //uint2 tdata, rdata; // first for time, second for randoms
    uint2 tdata;
    uint2 rdata;
};

struct DrawInfo {
    uint4 data;
};

// 
[[vk::binding(0,0)]] RWByteAddressBuffer mesh0[] : register(u0, space0); 
//[[vk::binding(0,0)]] RWStructuredBuffer<uint> mesh0[] : register(u0, space0); 
// layout (binding = 0, set = 0) readonly buffer MeshData { uint8_t data[]; } mesh0[]; // SHOULD TO BE IN FUTURE!

// LSD Mapping (Shadows, Emission, Voxels, Ray-Tracing...)
[[vk::binding(2,0)]] RWStructuredBuffer<uint> map : register(u0, space2);
[[vk::binding(3,0)]] RWTexture2D<uint> mapImage[] : register(u0, space3);
[[vk::binding(4,0)]] Texture2D<float4> mapColor[] : register(t0, space4);

// Bindings Set (Binding 2 is Acceleration Structure, may implemented in Inline Version)
[[vk::binding(5,1)]] StructuredBuffer<Binding> bindings[] : register(t0, space5);
[[vk::binding(6,1)]] StructuredBuffer<Attribute> attributes[] : register(t0, space6);

// 
//[[vk::binding(7,1)]] StructuredBuffer<Transform3x4> tmatrices[] : register(t0, space7);
[[vk::binding(7,1)]] StructuredBuffer<float3x4> tmatrices[] : register(t0, space7);
[[vk::binding(8,1)]] StructuredBuffer<uint> materialID[] : register(t0, space8);

// 
[[vk::binding(9,1)]] ConstantBuffer<Matrices> pushed : register(b0, space9);
[[vk::binding(10,1)]] RWStructuredBuffer<MeshInfo> meshInfo : register(u0, space10);
[[vk::binding(11,1)]] RWStructuredBuffer<RTXInstance> rtxInstances : register(u0, space11);

// 
#ifdef ENABLE_AS
[[vk::binding(12,1)]] RaytracingAccelerationStructure Scene : register(t0, space12);
#endif

// Deferred and Rasterization Set
[[vk::binding(13,2)]] Texture2D<float4>  frameBuffers[12u] : register(t0, space13); // Pre-resampled buffers
[[vk::binding(15,2)]] Texture2D<float4> rasterBuffers[ 8u] : register(t0, space15); // Used by rasterization
[[vk::binding(14,2)]] SamplerState            samplers[4u] : register(t0, space14);

// 
[[vk::binding(16,3)]] RWTexture2D<float4> writeBuffer[] : register(u0, space16); // Pre-resampled buffers, For EDIT!
[[vk::binding(17,3)]] RWTexture2D<float4> writeImages[] : register(u0, space17); 
[[vk::binding(18,3)]] RWTexture2D<float4> writeImagesBack[] : register(u0, space18); 

// 
[[vk::binding(20,4)]] RWStructuredBuffer<MaterialUnit> materials : register(u0, space20);
[[vk::binding(21,4)]] Texture2D<float4> background : register(t0, space21);
[[vk::binding(22,4)]] Texture2D<float4> textures[] : register(t0, space22);
[[vk::push_constant]] ConstantBuffer<DrawInfo> drawInfo : register(b0, space23);

//
#define imageLoad(b, c) b[c]
#define imageStore(b, c, f) (b[c] = f)
#define texelFetch(b, c) b[c]
#define texelFetch(b, c, m) b[uint3(c,m)]
#define textureSample(b, s, c) b.Sample(s, c)
//#define textureSample(b, s, c, m) b.SampleLevel(s, c, m)
#define textureLodSample(b, s, c, m) b.SampleLevel(s, c, m)
#define nonuniformEXT(a) a

// 
uint2 imageSize(in RWTexture2D tex) { uint2 size = uint2(0,0); return tex.GetDimensions(size.x, size.y); return size; };
uint3 imageSize(in RWTexture3D tex) { uint3 size = uint3(0,0,0); return tex.GetDimensions(size.x, size.y, size.z); return size; };
uint2 textureSize(in Texture2D tex, in int lod) { uint2 size = uint2(0,0); return tex.GetDimensions(size.x, size.y); return size; };
uint3 textureSize(in Texture3D tex, in int lod) { uint3 size = uint3(0,0,0); return tex.GetDimensions(size.x, size.y, size.z); return size; };
//uint2 textureSize(in Texture2D tex, in int lod) { uint2 size = uint2(0,0); return tex.GetDimensions(uint(lod), size.x, size.y); return size; };
//uint3 textureSize(in Texture3D tex, in int lod) { uint3 size = uint3(0,0,0); return tex.GetDimensions(uint(lod), size.x, size.y, size.z); return size; };
#endif

// 
uint getMeshID(in RTXInstance instance){
    return bitfieldExtract(instance.instance_mask, 0, 24); // only hack method support
};

// System Specified
#ifdef GLSL
#define meshID nonuniformEXT(nodeMeshID)
#else
#define meshID nodeMeshID
#endif


// System Specified
#ifdef GLSL
uint8_t load_u8(in uint offset, in uint binding, in uint nodeMeshID) {
    if (binding == 0u) { return mesh0[nonuniformEXT(meshID)].data[offset]; };
    return uint8_t(0u);
};

// System Specified
uint16_t load_u16(in uint offset, in uint binding, in uint nodeMeshID) {
    return pack16(u8vec2(load_u8(offset,binding,nodeMeshID),load_u8(offset+1u,binding,nodeMeshID)));
};

// System Specified
uint load_u32(in uint offset, in uint binding, in uint nodeMeshID) {
    return pack32(u16vec2(load_u16(offset,binding,nodeMeshID),load_u16(offset+2u,binding,nodeMeshID)));
};
#else
// System Specified
uint load_u32(in uint offset, in uint binding, in uint nodeMeshID) {
    //return pack32(u16float2(load_u16(offset,binding,nodeMeshID),load_u16(offset+2u,binding,nodeMeshID)));
    return mesh0[nodeMeshID].Load(int(offset)).x;
};
#endif

// TODO: Add Uint16_t, uint, Float16_t Support
float4 get_float4(in uint idx, in uint loc, in uint nodeMeshID) {
#ifdef GLSL
    Attribute attrib = attributes[meshID].data[loc];
    Binding  binding = bindings[meshID].data[attrib.binding];
#else
    Attribute attrib = attributes[nodeMeshID][loc];
    Binding  binding = bindings[nodeMeshID][attrib.binding];
#endif

    //Attribute attrib = attributes[loc].data[meshID];
    //Binding  binding = bindings[attrib.binding].data[meshID];
    uint boffset = binding.stride * idx + attrib.offset;
    float4 vec = float4(0.f);
    
    // 
    //if (binding.stride >  0u) vec = float4(0.f,0.f,1.f,0.f);
    if (binding.stride >  0u) vec[0] = uintBitsToFloat(load_u32(boffset +  0u, attrib.binding, nodeMeshID));
    if (binding.stride >  4u) vec[1] = uintBitsToFloat(load_u32(boffset +  4u, attrib.binding, nodeMeshID));
    if (binding.stride >  8u) vec[2] = uintBitsToFloat(load_u32(boffset +  8u, attrib.binding, nodeMeshID));
    if (binding.stride > 12u) vec[3] = uintBitsToFloat(load_u32(boffset + 12u, attrib.binding, nodeMeshID));
    
    // 
    return vec;
};

float4 triangulate(in uint3 indices, in uint loc, in uint nodeMeshID, in float3 barycenter){
    const float3x4 mc = float3x4(
        get_float4(indices[0],loc,nodeMeshID),
        get_float4(indices[1],loc,nodeMeshID),
        get_float4(indices[2],loc,nodeMeshID)
    );
    return mul(barycenter, mc);
};

float4x4 regen4(in float3x4 T) {
    return float4x4(T[0],T[1],T[2],float4(0.f.xxx,1.f));
};

float3x3 regen3(in float3x4 T) {
    return float3x3(T[0].xyz,T[1].xyz,T[2].xyz);
};

float4 mul4(in float4 v, in float3x4 M) {
    return float4(mul(M, v),1.f);
};

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
STATIC const float PHI = 1.6180339887498948482f;
STATIC const float SFN = 0.00000011920928955078125f, SFO = 1.f+SFN;//1.00000011920928955078125f;
STATIC const float INFINITY = 1e+5f, N_INFINITY = (INFINITY*(1.f-SFN));
STATIC const float PI = 3.1415926535897932384626422832795028841971f;
STATIC const float TWO_PI = 6.2831853071795864769252867665590057683943f;
STATIC const float SQRT_OF_ONE_THIRD = 0.5773502691896257645091487805019574556476f;
STATIC const float E = 2.7182818284590452353602874713526624977572f;
STATIC const float INV_PI = 0.3183098861837907f;
STATIC const float TWO_INV_PI = 0.6366197723675814f;
STATIC const float INV_TWO_PI = 0.15915494309189535f;

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
STATIC uint counter = 0u;
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

#ifdef GLSL
    float  f = uintBitsToFloat( m );       // Range [1:2]
    return fract(f - 1.0);                 // Range [0:1]
#else
    float  f = asfloat( m );       // Range [1:2]
    return frac(f - 1.0);          // Range [0:1]
#endif
};

#ifdef GLSL
highp float2 halfConstruct ( in uint  m ) { return fract(unpackHalf2x16((m & 0x03FF03FFu) | (0x3C003C00u))-1.f); }
#else
float2 halfConstruct ( in uint  m ) { return frac(f16tof32((m & 0x03FF03FFu) | (0x3C003C00u))-1.f); }
#endif


// Pseudo-random value in half-open range [0:1].
//float random( float x ) { return floatConstruct(hash(floatBitsToUint(x))); }
//float random( float2  v ) { return floatConstruct(hash(floatBitsToUint(v))); }
//float random( float3  v ) { return floatConstruct(hash(floatBitsToUint(v))); }
//float random( float4  v ) { return floatConstruct(hash(floatBitsToUint(v))); }
#ifdef GLSL
#define QLOCK2 (clockRealtime2x32EXT()+clock2x32ARB())
#else
#define QLOCK2 0u.xx
#endif

STATIC uint SCLOCK = 0u;
//#define QLOCK2 uint2(0u,0u)

// 
float random(               ) {         return floatConstruct(hash(QLOCK2)); }
float random( inout uint2 s ) { s += 1; return floatConstruct(hash(uint4(QLOCK2,s))); }
float random( inout uint  s ) { s += 1; return floatConstruct(hash(uint3(QLOCK2,s))); }

// 
float2 random2(               ) {         return halfConstruct(hash(QLOCK2)); }
float2 random2( inout uint2 s ) { s += 1; return halfConstruct(hash(uint4(QLOCK2,s))); }
float2 random2( inout uint  s ) { s += 1; return halfConstruct(hash(uint3(QLOCK2,s))); }

// 
float  staticRandom () { SCLOCK += 1; return floatConstruct(hash(uint4(SCLOCK,0u, pushed.rdata.xy))); }
float2 staticRandom2() { SCLOCK += 1; return  halfConstruct(hash(uint4(SCLOCK,0u, pushed.rdata.xy))); }

// 
#ifdef GLSL
float2 lcts(in float3 direct) { return float2(fma(atan(direct.z,direct.x),INV_TWO_PI,0.5f),acos(-direct.y)*INV_PI); };
#else
float2 lcts(in float3 direct) { return float2(fma(atan2(direct.z,direct.x),INV_TWO_PI,0.5f),acos(-direct.y)*INV_PI); };
#endif

// 
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
    return normalize(mix(reflect(I,TBN[2]), randomHemisphereCosine(seed,TBN), gloss*sqrt(random(seed))));
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


#if defined(HLSL) || !defined(GLSL)

bool2 fequal(in float2 a, in float2 b){
    return a <= (b + 0.0001f.xx) && a >= (b - 0.0001f.xx);
};

bool3 fequal(in float3 a, in float3 b){
    return a <= (b + 0.0001f.xxx) && a >= (b - 0.0001f.xxx);
};

bool4 fequal(in float4 a, in float4 b){
    return a <= (b + 0.0001f.xxxx) && a >= (b - 0.0001f.xxxx);
};

#else

bool3 fequal(in float3 a, in float3 b){
    return and(
        lessThanEqual(a, b + 0.0001f),
        greaterThanEqual(a, b - 0.0001f));
};

bool4 fequal(in float4 a, in float4 b){
    return and(
        lessThanEqual(a, b + 0.0001f),
        greaterThanEqual(a, b - 0.0001f));
};

#endif


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

float3 boxNormal(in float3 mpoint, in float3 boxMin, in float3 boxMax) {
    const float kEpsilon = 0.0001f;
	float3 center = (boxMax + boxMin) * 0.5;
	float3 size = (boxMax - boxMin) * 0.5;
	float3 pc = mpoint - center;
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
    return divW(mul(pushed.projection, float4(mul(pushed.modelview, float4(origin,1.f)), 1.f)));
};

float3 screen2world(in float3 origin){
    return mul(pushed.modelviewInv, float4(divW(mul(pushed.projectionInv, float4(origin,1.f))), 1.f));
};


// Some Settings
//const float3 gSkyColor = float3(0.9f,0.98,0.999f); // TODO: Use 1.f and texture shading (include from rasterization)
//#define gSkyColor float3(0.9f,0.98,0.999f)
#define DIFFUSE_COLOR (diffuseColor.xyz)
//#define BACKSKY_COLOR gSignal.xyz = max(fma(gEnergy.xyz, (i > 0u ? gSkyColor : 1.f.xxx), gSignal.xyz),0.f.xxx), gEnergy *= 0.f
//#define BACKSKY_COLOR gSignal.xyz = max(fma(gEnergy.xyz, gSkyColor, gSignal.xyz),0.f.xxx), gEnergy *= 0.f

float4 gSkyShader(in float3 raydir, in float3 origin) {
//#ifdef GLSL
//    return float4(texture(sampler2D(background, samplers[3u]), flip(lcts(raydir.xyz))).xyz, 1.f);
//#else
//    return background.SampleLevel(samplers[3u], flip(lcts(raydir.xyz)).xy, 0);
//#endif
    return float4(textureSample(background, samplers[3u], flip(lcts(raydir.xyz))).xyz, 1.f);
};

#endif
