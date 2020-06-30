struct MeshInfo
{
    uint materialID;
    uint indexType;
    uint primitiveCount;
    uint flags;
};

struct RTXInstance
{
    row_major float3x4 transform;
    uint instance_mask;
    uint offset_flags;
    uint2 handle;
};

struct Binding
{
    uint binding;
    uint stride;
    uint rate;
};

struct Attribute
{
    uint location;
    uint binding;
    uint format;
    uint offset;
};

struct MaterialUnit
{
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

static const float3 _193[3] = { float3(1.0f, 0.0f, 0.0f), float3(0.0f, 1.0f, 0.0f), float3(0.0f, 0.0f, 1.0f) };

ByteAddressBuffer instances[] : register(t7, space1);
ByteAddressBuffer _121 : register(t10, space1);
ByteAddressBuffer _142 : register(t11, space1);
cbuffer Matrices : register(b9, space1)
{
    row_major float4x4 _224_projection : packoffset(c0);
    row_major float4x4 _224_projectionInv : packoffset(c4);
    row_major float3x4 _224_modelview : packoffset(c8);
    row_major float3x4 _224_modelviewInv : packoffset(c11);
    row_major float3x4 _224_modelviewPrev : packoffset(c14);
    row_major float3x4 _224_modelviewPrevInv : packoffset(c17);
    uint4 _224_mdata : packoffset(c20);
    uint2 _224_tdata : packoffset(c21);
    uint2 _224_rdata : packoffset(c21.z);
};

ByteAddressBuffer map[1] : register(t2, space0);
ByteAddressBuffer bindings[] : register(t5, space1);
ByteAddressBuffer attributes[] : register(t6, space1);
ByteAddressBuffer geomMTs[1] : register(t8, space1);
ByteAddressBuffer materials[1] : register(t20, space4);
cbuffer pushConstants
{
    uint4 drawInfo_data : packoffset(c0);
};

RWBuffer<uint> mesh0[] : register(u0, space0);
RWTexture2D<uint> mapImage[1] : register(u3, space0);
Texture2D<float4> mapColor[1] : register(t4, space0);
uniform ??? Scene;
Texture2D<float4> frameBuffers[12] : register(t13, space2);
SamplerState samplers[4] : register(s14, space2);
Texture2D<float4> rasterBuffers[8] : register(t15, space2);
RWTexture2D<float4> writeBuffer[1] : register(u16, space3);
RWTexture2D<float4> writeImages[1] : register(u17, space3);
RWTexture2D<float4> writeImagesBack[1] : register(u18, space3);
Texture2D<float4> background : register(t21, space4);
Texture2D<float4> textures[1] : register(t22, space4);

static float4 gl_Position;
static int gl_VertexIndex;
static int gl_InstanceIndex;
static float4 fTexcoord;
static float2 iTexcoord;
static float4 fPosition;
static float3 iPosition;
static float4 fBarycent;
static uint4 uData;
static float3 iNormals;
static float4 iTangent;
static float4 iBinormal;

struct SPIRV_Cross_Input
{
    float3 iPosition : TEXCOORD0;
    float2 iTexcoord : TEXCOORD1;
    float3 iNormals : TEXCOORD2;
    float4 iTangent : TEXCOORD3;
    float4 iBinormal : TEXCOORD4;
    uint gl_VertexIndex : SV_VertexID;
    uint gl_InstanceIndex : SV_InstanceID;
};

struct SPIRV_Cross_Output
{
    float4 fPosition : TEXCOORD0;
    float4 fTexcoord : TEXCOORD1;
    float4 fBarycent : TEXCOORD2;
    nointerpolation uint4 uData : TEXCOORD3;
    float4 gl_Position : SV_Position;
};

static uint counter;
static uint SCLOCK;

uint SPIRV_Cross_bitfieldUExtract(uint Base, uint Offset, uint Count)
{
    uint Mask = Count == 32 ? 0xffffffff : ((1 << Count) - 1);
    return (Base >> Offset) & Mask;
}

uint2 SPIRV_Cross_bitfieldUExtract(uint2 Base, uint Offset, uint Count)
{
    uint Mask = Count == 32 ? 0xffffffff : ((1 << Count) - 1);
    return (Base >> Offset) & Mask;
}

uint3 SPIRV_Cross_bitfieldUExtract(uint3 Base, uint Offset, uint Count)
{
    uint Mask = Count == 32 ? 0xffffffff : ((1 << Count) - 1);
    return (Base >> Offset) & Mask;
}

uint4 SPIRV_Cross_bitfieldUExtract(uint4 Base, uint Offset, uint Count)
{
    uint Mask = Count == 32 ? 0xffffffff : ((1 << Count) - 1);
    return (Base >> Offset) & Mask;
}

int SPIRV_Cross_bitfieldSExtract(int Base, int Offset, int Count)
{
    int Mask = Count == 32 ? -1 : ((1 << Count) - 1);
    int Masked = (Base >> Offset) & Mask;
    int ExtendShift = (32 - Count) & 31;
    return (Masked << ExtendShift) >> ExtendShift;
}

int2 SPIRV_Cross_bitfieldSExtract(int2 Base, int Offset, int Count)
{
    int Mask = Count == 32 ? -1 : ((1 << Count) - 1);
    int2 Masked = (Base >> Offset) & Mask;
    int ExtendShift = (32 - Count) & 31;
    return (Masked << ExtendShift) >> ExtendShift;
}

int3 SPIRV_Cross_bitfieldSExtract(int3 Base, int Offset, int Count)
{
    int Mask = Count == 32 ? -1 : ((1 << Count) - 1);
    int3 Masked = (Base >> Offset) & Mask;
    int ExtendShift = (32 - Count) & 31;
    return (Masked << ExtendShift) >> ExtendShift;
}

int4 SPIRV_Cross_bitfieldSExtract(int4 Base, int Offset, int Count)
{
    int Mask = Count == 32 ? -1 : ((1 << Count) - 1);
    int4 Masked = (Base >> Offset) & Mask;
    int ExtendShift = (32 - Count) & 31;
    return (Masked << ExtendShift) >> ExtendShift;
}

// Returns the determinant of a 2x2 matrix.
float SPIRV_Cross_Det2x2(float a1, float a2, float b1, float b2)
{
    return a1 * b2 - b1 * a2;
}

// Returns the inverse of a matrix, by using the algorithm of calculating the classical
// adjoint and dividing by the determinant. The contents of the matrix are changed.
float3x3 SPIRV_Cross_Inverse(float3x3 m)
{
    float3x3 adj;	// The adjoint matrix (inverse after dividing by determinant)

    // Create the transpose of the cofactors, as the classical adjoint of the matrix.
    adj[0][0] =  SPIRV_Cross_Det2x2(m[1][1], m[1][2], m[2][1], m[2][2]);
    adj[0][1] = -SPIRV_Cross_Det2x2(m[0][1], m[0][2], m[2][1], m[2][2]);
    adj[0][2] =  SPIRV_Cross_Det2x2(m[0][1], m[0][2], m[1][1], m[1][2]);

    adj[1][0] = -SPIRV_Cross_Det2x2(m[1][0], m[1][2], m[2][0], m[2][2]);
    adj[1][1] =  SPIRV_Cross_Det2x2(m[0][0], m[0][2], m[2][0], m[2][2]);
    adj[1][2] = -SPIRV_Cross_Det2x2(m[0][0], m[0][2], m[1][0], m[1][2]);

    adj[2][0] =  SPIRV_Cross_Det2x2(m[1][0], m[1][1], m[2][0], m[2][1]);
    adj[2][1] = -SPIRV_Cross_Det2x2(m[0][0], m[0][1], m[2][0], m[2][1]);
    adj[2][2] =  SPIRV_Cross_Det2x2(m[0][0], m[0][1], m[1][0], m[1][1]);

    // Calculate the determinant as a combination of the cofactors of the first row.
    float det = (adj[0][0] * m[0][0]) + (adj[0][1] * m[1][0]) + (adj[0][2] * m[2][0]);

    // Divide the classical adjoint matrix by the determinant.
    // If determinant is zero, matrix is not invertable, so leave it unchanged.
    return (det != 0.0f) ? (adj * (1.0f / det)) : m;
}

bool hasTransform(MeshInfo meshInfo)
{
    return SPIRV_Cross_bitfieldUExtract(meshInfo.flags, 0, 1) != 0u;
}

float3x3 regen3(float3x4 T)
{
    return float3x3(float3(T[0].xyz), float3(T[1].xyz), float3(T[2].xyz));
}

float4 mul4(float4 v, float3x4 M)
{
    return float4(mul(M, v), 1.0f);
}

void vert_main()
{
    counter = 0u;
    SCLOCK = 0u;
    uint geometryInstanceID = uint(gl_InstanceIndex);
    uint nodeMeshID = drawInfo_data.x;
    uint globalInstanceID = drawInfo_data.z;
    uint idx = uint(gl_VertexIndex);
    float3x4 _112 = asfloat(uint3x4(instances[nodeMeshID].Load4(geometryInstanceID * 48 + 0), instances[nodeMeshID].Load4(geometryInstanceID * 48 + 16), instances[nodeMeshID].Load4(geometryInstanceID * 48 + 32)));
    float3x4 matras = float3x4(_112[0], _112[1], _112[2]);
    MeshInfo _126;
    _126.materialID = _121.Load(nodeMeshID * 16 + 0);
    _126.indexType = _121.Load(nodeMeshID * 16 + 4);
    _126.primitiveCount = _121.Load(nodeMeshID * 16 + 8);
    _126.flags = _121.Load(nodeMeshID * 16 + 12);
    MeshInfo _127;
    _127.materialID = _126.materialID;
    _127.indexType = _126.indexType;
    _127.primitiveCount = _126.primitiveCount;
    _127.flags = _126.flags;
    MeshInfo param = _127;
    if (!hasTransform(param))
    {
        matras = float3x4(float4(1.0f, 0.0f, 0.0f, 0.0f), float4(0.0f, 1.0f, 0.0f, 0.0f), float4(0.0f, 0.0f, 1.0f, 0.0f));
    }
    float3x4 _145 = asfloat(uint3x4(_142.Load4(globalInstanceID * 64 + 0), _142.Load4(globalInstanceID * 64 + 16), _142.Load4(globalInstanceID * 64 + 32)));
    float3x4 matra4 = _145;
    float3x4 param_1 = matras;
    float3x3 normalTransform = SPIRV_Cross_Inverse(transpose(regen3(param_1)));
    float3x4 param_2 = matra4;
    float3x3 normInTransform = SPIRV_Cross_Inverse(transpose(regen3(param_2)));
    fTexcoord = float4(iTexcoord, 0.0f.xx);
    float4 param_3 = float4(iPosition, 1.0f);
    float3x4 param_4 = matras;
    float4 param_5 = mul4(param_3, param_4);
    float3x4 param_6 = matra4;
    fPosition = mul4(param_5, param_6);
    fBarycent = float4(_193[idx % 3u], 0.0f);
    uData = uint4(uint(gl_InstanceIndex), uint3(0u, 0u, 0u));
    gl_Position = mul(_224_projection, float4(mul(_224_modelview, fPosition), 1.0f));
}

SPIRV_Cross_Output main(SPIRV_Cross_Input stage_input)
{
    gl_VertexIndex = int(stage_input.gl_VertexIndex);
    gl_InstanceIndex = int(stage_input.gl_InstanceIndex);
    iTexcoord = stage_input.iTexcoord;
    iPosition = stage_input.iPosition;
    iNormals = stage_input.iNormals;
    iTangent = stage_input.iTangent;
    iBinormal = stage_input.iBinormal;
    vert_main();
    SPIRV_Cross_Output stage_output;
    stage_output.gl_Position = gl_Position;
    stage_output.fTexcoord = fTexcoord;
    stage_output.fPosition = fPosition;
    stage_output.fBarycent = fBarycent;
    stage_output.uData = uData;
    return stage_output;
}
