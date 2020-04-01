

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

layout (binding = 0, set = 0, r8ui) readonly uniform uimageBuffer buffers[8u];
layout (binding = 1, set = 0, r8ui) readonly uniform uimageBuffer indices;
layout (binding = 2, set = 0, scalar) readonly buffer Bindings   { Binding   bindings[]; };
layout (binding = 3, set = 0, scalar) readonly buffer Attributes { Attribute attributes[]; };
layout (push_constant) uniform pushConstants { uvec4 data; } drawInfo;

bool hasTransform(){
    return bool(bitfieldExtract(drawInfo.data[3],0,1));
};

bool hasNormal(){
    return bool(bitfieldExtract(drawInfo.data[3],1,1));
};

bool hasTexcoord(){
    return bool(bitfieldExtract(drawInfo.data[3],2,1));
};

bool hasTangent(){
    return bool(bitfieldExtract(drawInfo.data[3],3,1));
};


// System Specified
uint8_t load_u8(in uint offset, in uint binding, in bool idx) {
    if (idx) { return uint8_t(imageLoad(indices, int(offset)).x); };
    return uint8_t(imageLoad(buffers[nonuniformEXT(binding)], int(offset)).x);
};

// System Specified
uint16_t load_u16(in uint offset, in uint binding, in bool idx) {
    return pack16(u8vec2(load_u8(offset,binding,idx),load_u8(offset+1u,binding,idx)));
};

// System Specified
uint32_t load_u32(in uint offset, in uint binding, in bool idx) {
    return pack32(u16vec2(load_u16(offset,binding,idx),load_u16(offset+2u,binding,idx)));
};

// TODO: Add Uint16_t, Uint32_t, Float16_t Support
vec4 get_vec4(in uint idx, in uint loc) {
    Attribute attrib = attributes[loc];
    Binding  binding = bindings[attrib.binding];
    //Attribute attrib = attributes[loc].data[meshID];
    //Binding  binding = bindings[attrib.binding].data[meshID];
    uint32_t boffset = binding.stride * idx + attrib.offset;
    vec4 vec = vec4(0.f);
    
    // 
    //if (binding.stride >  0u) vec = vec4(0.f,0.f,1.f,0.f);
    if (binding.stride >  0u) vec[0] = uintBitsToFloat(load_u32(boffset +  0u, attrib.binding, false));
    if (binding.stride >  4u) vec[1] = uintBitsToFloat(load_u32(boffset +  4u, attrib.binding, false));
    if (binding.stride >  8u) vec[2] = uintBitsToFloat(load_u32(boffset +  8u, attrib.binding, false));
    if (binding.stride > 12u) vec[3] = uintBitsToFloat(load_u32(boffset + 12u, attrib.binding, false));
    
    // 
    return vec;
};

vec4 triangulate(in uvec3 indices, in uint loc, in vec3 barycenter){
    const mat3x4 mc = mat3x4(
        get_vec4(indices[0],loc),
        get_vec4(indices[1],loc),
        get_vec4(indices[2],loc)
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
