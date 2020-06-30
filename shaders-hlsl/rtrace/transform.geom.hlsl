#define TRANSFORM_FEEDBACK
#include "./driver.hlsli"

// 
struct GS_INPUT {
    float4 gPosition : POSITION;
    float4 gTexcoord : TEXCOORD;
    float4 gNormal   : NORMAL;
    float4 gTangent  : TANGENT;
    float4 Position  : SV_POSITION;
    float PointSize  : PSIZE;
};

// 
struct TS_OUTPUT {
    float4 fPosition : POSITION;
    float4 fTexcoord : TEXCOORD;
    float4 fNormal   : NORMAL;
    float4 fTangent  : TANGENT;
    float4 fBinormal : BINORMAL;
};

// Should to able used by OpenGL
[maxvertexcount(3)]
void main(in triangle GS_INPUT input[3], inout TriangleStream<TS_OUTPUT> OutputStream) {
    const float4 dp1 = input[1].gPosition - input[0].gPosition, dp2 = input[2].gPosition - input[0].gPosition;
    const float4 tx1 = input[1].gTexcoord - input[0].gTexcoord, tx2 = input[2].gTexcoord - input[0].gTexcoord;
    const float3 normal = normalize(cross(dp1.xyz, dp2.xyz));
    //const float2 size  = textureSize(frameBuffers[IW_POSITION], 0);
    //const float2 pixelShift = (staticRandom2() - 0.5f) / size;

    TS_OUTPUT output;
    [unroll] for (uint i=0u;i<3u;i++) { // 
        //gl_Position = gl_in[i].gl_Position;

        // 
        output.fPosition = input[i].gPosition;
        output.fTexcoord = input[i].gTexcoord; // TODO: move texcoord by sample position for anti-aliased
        output.fTangent = input[i].gTangent;
        output.fNormal = input[i].gNormal;

        // 
        const float coef = 1.f / (tx1.x * tx2.y - tx2.x * tx1.y);
        const float3 tangent = (dp1.xyz * tx2.yyy - dp2.xyz * tx1.yyy) * coef;
        const float3 binorml = (dp1.xyz * tx2.xxx - dp2.xyz * tx1.xxx) * coef;
        if (!hasNormal()) { output.fNormal  = float4(normal, 0.f); };

        // 
        if (!hasTangent()) { 
            output.fTangent .xyz = tangent; //- dot(fNormal.xyz,tangent.xyz)*fNormal.xyz;
            output.fBinormal.xyz = binorml; //- dot(fNormal.xyz,binorml.xyz)*fNormal.xyz;
        } else {
            output.fBinormal.xyz = cross(output.fNormal.xyz, output.fTangent.xyz);
            //output.fBinormal.xyz = fBinormal.xyz - dot(fNormal.xyz,fBinormal.xyz)*fNormal.xyz;
        };

        // 
        output.fNormal.xyz = normalize(output.fNormal.xyz);
        output.fTangent.xyz = normalize(output.fTangent.xyz);
        output.fBinormal.xyz = normalize(output.fBinormal.xyz);
        OutputStream.Append(output);
    };
};
