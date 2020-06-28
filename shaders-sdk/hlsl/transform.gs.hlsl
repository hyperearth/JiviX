#define TRANSFORM_FEEDBACK
#include "./driver.glsl"

// 
layout (triangles) in;
layout (triangle_strip, max_vertices = 3) out;
//layout ( binding = 4, set = 0 ) uniform accelerationStructureEXT Scene;

// 
layout (location = 0) in float4 gPosition[];
layout (location = 1) in float4 gTexcoord[];
layout (location = 2) in float4 gNormal[];
layout (location = 3) in float4 gTangent[];
//layout (location = 4) flat in int4 gIndexes;

// 
layout (location = 0, xfb_buffer = 0, xfb_stride = 80, xfb_offset = 0 ) out float4 fPosition;
layout (location = 1, xfb_buffer = 0, xfb_stride = 80, xfb_offset = 16) out float4 fTexcoord;
layout (location = 2, xfb_buffer = 0, xfb_stride = 80, xfb_offset = 32) out float4 fNormal;
layout (location = 3, xfb_buffer = 0, xfb_stride = 80, xfb_offset = 48) out float4 fTangent;
layout (location = 4, xfb_buffer = 0, xfb_stride = 80, xfb_offset = 64) out float4 fBinormal;
//layout (location = 5) flat out uint4 uData;

out gl_PerVertex {   // some subset of these members will be used
    float4 gl_Position;
    float gl_PointSize;
};

// Should to able used by OpenGL
void main() {
    const float4 dp1 = gPosition[1] - gPosition[0], dp2 = gPosition[2] - gPosition[0];
    const float4 tx1 = gTexcoord[1] - gTexcoord[0], tx2 = gTexcoord[2] - gTexcoord[0];
    const float3 normal = normalize(cross(dp1.xyz, dp2.xyz));
    //const float2 size  = textureSize(frameBuffers[IW_POSITION], 0);
    //const float2 pixelShift = (staticRandom2() - 0.5f) / size;

    [[unroll]] for (uint i=0u;i<3u;i++) { // 
        gl_Position = gl_in[i].gl_Position;

        // 
        fPosition = gPosition[i];
        fTexcoord = gTexcoord[i]; // TODO: move texcoord by sample position for anti-aliased
        fTangent = gTangent[i];
        fNormal = gNormal[i];

        // 
        const float coef = 1.f / (tx1.x * tx2.y - tx2.x * tx1.y);
        const float3 tangent = (dp1.xyz * tx2.yyy - dp2.xyz * tx1.yyy) * coef;
        const float3 binorml = (dp1.xyz * tx2.xxx - dp2.xyz * tx1.xxx) * coef;
        if (!hasNormal()) { fNormal  = float4(normal, 0.f); };

        // 
        if (!hasTangent()) { 
            fTangent .xyz = tangent; //- dot(fNormal.xyz,tangent.xyz)*fNormal.xyz;
            fBinormal.xyz = binorml; //- dot(fNormal.xyz,binorml.xyz)*fNormal.xyz;
        } else {
            fBinormal.xyz = cross(fNormal.xyz,fTangent.xyz);
            //fBinormal.xyz = fBinormal.xyz - dot(fNormal.xyz,fBinormal.xyz)*fNormal.xyz;
        };

        // 
        fNormal.xyz = normalize(fNormal.xyz);
        fTangent.xyz = normalize(fTangent.xyz);
        fBinormal.xyz = normalize(fBinormal.xyz);

        // 
        EmitVertex();
    };

    EndPrimitive();
};
