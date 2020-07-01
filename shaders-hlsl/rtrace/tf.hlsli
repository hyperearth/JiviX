uint bitfieldExtract(uint val, int off, int size) {
	// This built-in function is only support in OpenGL 4.0 and ES 3.1
	// Hopefully the shader compiler will get our meaning and emit the right instruction
	uint mask = uint((1 << size) - 1);
	return uint(val >> off) & mask;
};

struct Binding {
    //uint binding;
    uint bufvsd;
    uint stride;
    uint rate;
    //uint reserved;
};

struct Attribute {
    uint location;
    uint binding;
    uint format;
    uint offset;
};

struct MeshInfo {
    uint materialID;
    uint indexType;
    uint primitiveCount;
    uint flags;
};

struct DrawInfo { uint4 data; };

// 
#ifdef GEN_QUAD_INDEX
[[vk::binding(1,1)]] RWByteAddressBuffer buffers[] : register(u0, space1);
//[[vk::binding(1,0)]] RWBuffer<ubyte> buffers[] : register(u0, space1);
#else
//[[vk::binding(0,0)]]   ByteAddressBuffer buffers[] : register(u0, space0);
//[[vk::binding(0,1)]] RWBuffer buffers[] : register(t0, space0);
[[vk::binding(0,1)]] RWByteAddressBuffer buffers[] : register(u0, space0);
#endif

// 
[[vk::binding(2,0)]] RWStructuredBuffer<Binding> bindings : register(u0, space2);
[[vk::binding(3,0)]] RWStructuredBuffer<Attribute> attributes : register(u0, space3);
[[vk::push_constant]] ConstantBuffer<DrawInfo> drawInfo : register(b0, space4);

// 
bool hasTransform() {
    return bool(bitfieldExtract(drawInfo.data[3],0,1));
};

// 
bool hasNormal() {
    return bool(bitfieldExtract(drawInfo.data[3],1,1));
};

// 
bool hasTexcoord() {
    return bool(bitfieldExtract(drawInfo.data[3],2,1));
};

// 
bool hasTangent() {
    return bool(bitfieldExtract(drawInfo.data[3],3,1));
};

// 
void store_u32(in uint offset, in uint binding, in uint value) {
    buffers[binding].Store(offset, value);
};

// 
uint load_u32(in uint offset, in uint binding) {
    uint v8x4 = buffers[binding].Load(int(offset)).x;
    store_u32(offset, binding, v8x4);
    return v8x4;
};

// TODO: Add Uint16_t, uint, Float16_t Support
float4 get_float4(in uint idx, in uint loc) {
    Attribute attrib = attributes[loc];
    Binding  binding = bindings[attrib.binding];
    uint boffset = binding.stride * idx + attrib.offset;
    float4 vec = 0.f.xxxx;
    
    // 
    if (binding.stride >  0u) vec[0] = asfloat(load_u32(boffset +  0u, binding.bufvsd));
    if (binding.stride >  4u) vec[1] = asfloat(load_u32(boffset +  4u, binding.bufvsd));
    if (binding.stride >  8u) vec[2] = asfloat(load_u32(boffset +  8u, binding.bufvsd));
    if (binding.stride > 12u) vec[3] = asfloat(load_u32(boffset + 12u, binding.bufvsd));

    // 
    return vec;
};


float4 triangulate(in uint3 indices, in uint loc, in float3 barycenter){
    const float3x4 mc = float3x4(
        get_float4(indices[0],loc),
        get_float4(indices[1],loc),
        get_float4(indices[2],loc)
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
    return float4(mul(M,v),1.f);
};


#define IndexU8 1000265000
#define IndexU16 0
#define IndexU32 1
