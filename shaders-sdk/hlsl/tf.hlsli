

struct Binding {
    //uint32_t binding;
    uint32_t bufvsd;
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

// 
#ifdef GEN_QUAD_INDEX
layout (binding = 0, set = 1, r8ui)          uniform uimageBuffer buffers[256u];
#else
layout (binding = 0, set = 1, r8ui) readonly uniform uimageBuffer buffers[256u];
#endif

// 
layout (binding = 0, set = 0, scalar) readonly buffer Bindings   { Binding   bindings[]; };
layout (binding = 1, set = 0, scalar) readonly buffer Attributes { Attribute attributes[]; };
layout (push_constant) uniform pushConstants { uint4 data; } drawInfo;

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


// System Specified
uint8_t load_u8(in uint offset, in uint bufferID) {
    return uint8_t(imageLoad(buffers[nonuniformEXT(bufferID)], int(offset)).x);
};

// System Specified
uint16_t load_u16(in uint offset, in uint bufferID) {
    return pack16(u8float2(load_u8(offset,bufferID),load_u8(offset+1u,bufferID)));
};

// System Specified
uint32_t load_u32(in uint offset, in uint bufferID) {
    return pack32(u16float2(load_u16(offset,bufferID),load_u16(offset+2u,bufferID)));
};

// TODO: Add Uint16_t, Uint32_t, Float16_t Support
float4 get_float4(in uint idx, in uint loc) {
    Attribute attrib = attributes[loc];
    Binding  binding = bindings[attrib.binding];
    //Attribute attrib = attributes[loc].data[meshID];
    //Binding  binding = bindings[attrib.binding].data[meshID];
    uint32_t boffset = binding.stride * idx + attrib.offset;
    float4 vec = float4(0.f);
    
    // 
    //if (binding.stride >  0u) vec = float4(0.f,0.f,1.f,0.f);
    if (binding.stride >  0u) vec[0] = uintBitsToFloat(load_u32(boffset +  0u, binding.bufvsd));
    if (binding.stride >  4u) vec[1] = uintBitsToFloat(load_u32(boffset +  4u, binding.bufvsd));
    if (binding.stride >  8u) vec[2] = uintBitsToFloat(load_u32(boffset +  8u, binding.bufvsd));
    if (binding.stride > 12u) vec[3] = uintBitsToFloat(load_u32(boffset + 12u, binding.bufvsd));
    
    // 
    return vec;
};

float4 triangulate(in uint3 indices, in uint loc, in float3 barycenter){
    const float3x4 mc = float3x4(
        get_float4(indices[0],loc),
        get_float4(indices[1],loc),
        get_float4(indices[2],loc)
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

#define IndexU8 1000265000
#define IndexU16 0
#define IndexU32 1
