
// Re-Sampling
#define DIFFUSED_FLIP0 0
#define SAMPLING_FLIP0 1
#define DIFFUSED_FLIP1 2
#define SAMPLING_FLIP1 3

// Rasterization or First Step
#define COLORING 0
#define SAMPLING 1
#define NORMALED 2
#define TANGENTS 3

// 
struct RayPayloadData {
     vec4 fdata;
     vec4 normal;
    uvec4 udata;
};

struct Binding {
    uint32_t binding;
    uint32_t stride;
    uint32_t rate;
    uint32_t reserved;
};

struct Attribute {
    uint32_t location;
    uint32_t binding;
    uint32_t format;
    uint32_t offset;
};

struct MaterialUnit {
    f32vec4 diffuse;
};

// Mesh Data Buffers
layout (binding = 0, set = 0, scalar) buffer Data0 { uint8_t data[]; } mesh0[];
layout (binding = 1, set = 0, scalar) buffer Data1 { uint8_t data[]; } mesh1[];
layout (binding = 2, set = 0, scalar) buffer Data2 { uint8_t data[]; } mesh2[];
layout (binding = 3, set = 0, scalar) buffer Data3 { uint8_t data[]; } mesh3[];
layout (binding = 4, set = 0, scalar) buffer Data4 { uint8_t data[]; } mesh4[];
layout (binding = 5, set = 0, scalar) buffer Data5 { uint8_t data[]; } mesh5[];
layout (binding = 6, set = 0, scalar) buffer Data6 { uint8_t data[]; } mesh6[];
layout (binding = 7, set = 0, scalar) buffer Data7 { uint8_t data[]; } mesh7[];

// Bindings Set (Binding 2 is Acceleration Structure, may implemented in Inline Version)
layout (binding = 0, set = 1, scalar) uniform Bindings   { Binding   data[8]; } bindings  [];
layout (binding = 1, set = 1, scalar) uniform Attributes { Attribute data[8]; } attributes[];
layout (binding = 3, set = 1, scalar) uniform Matrices {
    mat4 projection;
    mat4 projectionInv;
    mat3x4 modelview;
    mat3x4 modelviewInv;
};

// Deferred and Rasterization Set
layout (binding = 0, set = 2) uniform sampler2D frameBuffers[];

// Sampling And Ray Tracing Set (also, re-used from previous frame)
layout (binding = 0, set = 3, rgba32f) uniform image2D writeImages[];

// Material Set
layout (binding = 0, set = 4) uniform sampler2D textures[];
layout (binding = 1, set = 4, scalar) buffer Materials { MaterialUnit data[]; } materials[];

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
    return f - 1.0;                        // Range [0:1]
}

highp vec2 halfConstruct ( in uint  m ) { return fract(unpackHalf2x16((m & 0x03FF03FFu) | (0x3C003C00u))-1.f); }



// Pseudo-random value in half-open range [0:1].
//float random( float x ) { return floatConstruct(hash(floatBitsToUint(x))); }
//float random( vec2  v ) { return floatConstruct(hash(floatBitsToUint(v))); }
//float random( vec3  v ) { return floatConstruct(hash(floatBitsToUint(v))); }
//float random( vec4  v ) { return floatConstruct(hash(floatBitsToUint(v))); }
float random(            ) { return floatConstruct(hash(clockRealtime2x32EXT())); }
float random( in uvec2 s ) { return floatConstruct(hash(uvec4(clockRealtime2x32EXT(),s))); }
float random( in uint  s ) { return floatConstruct(hash(uvec3(clockRealtime2x32EXT(),s))); }

vec2 random2(            ) { return halfConstruct(hash(clockRealtime2x32EXT())); }
vec2 random2( in uvec2 s ) { return halfConstruct(hash(uvec4(clockRealtime2x32EXT(),s))); }
vec2 random2( in uint  s ) { return halfConstruct(hash(uvec3(clockRealtime2x32EXT(),s))); }

vec2 lcts(in vec3 direct) { return vec2(fma(atan(direct.z,direct.x),INV_TWO_PI,0.5f),acos(-direct.y)*INV_PI); };
vec3 dcts(in vec2 hr) { 
    hr = fma(hr,vec2(TWO_PI,PI),vec2(-PI,0.f)); 
    const float up=-cos(hr.y),over=sqrt(fma(up,-up,1.f)); 
    return vec3(cos(hr.x)*over,up,sin(hr.x)*over); 
};

// geometric random generators
vec3 randomSphere() { return dcts(random2()); };
vec3 randomSphere(in uint  s) { return dcts(random2(s)); };
vec3 randomSphere(in uvec2 s) { return dcts(random2(s)); };


vec3 randomHemisphereCosine(in uvec2 seed) {
    const vec2 hmsm = random2(seed);
    const float phi = hmsm.x * TWO_PI, up = sqrt(1.0f - hmsm.y), over = sqrt(fma(up,-up,1.f));
    return vec3(cos(phi)*over,up,sin(phi)*over);
};

vec3 randomHemisphereCosine(in vec3 n, in uvec2 seed) {
    vec3 rand = vec3(random(seed),random2(seed))*2.f-1.f;
    float r = rand.x * 0.5 + 0.5; // [-1..1) -> [0..1)
    float angle = (rand.y + 1.0) * PI; // [-1..1] -> [0..2*PI)
    float sr = sqrt(r);
    vec2 p = vec2(sr * cos(angle), sr * sin(angle));
    vec3 ph = vec3(p.xy, sqrt(1.0 - p*p));
    vec3 tangent = normalize(rand);
    vec3 bitangent = cross(tangent, n);
    tangent = cross(bitangent, n);
    return tangent * ph.x + bitangent * ph.y + n * ph.z;
};


bool fequal(in float a, in float b){
    return 
        a <= b + 0.0001f && 
        a >= b - 0.0001f;
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
