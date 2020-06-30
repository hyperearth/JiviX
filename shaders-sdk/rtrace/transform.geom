#version 460 core // #
#extension GL_GOOGLE_include_directive  : require
#extension GL_EXT_ray_query             : require
#define TRANSFORM_FEEDBACK
#include "./driver.glsl"

// 
layout (triangles) in;
layout (triangle_strip, max_vertices = 3) out;
//layout ( binding = 4, set = 0 ) uniform accelerationStructureEXT Scene;

// 
layout (location = 0) in vec4 gPosition[];
layout (location = 1) in vec4 gTexcoord[];
layout (location = 2) in vec4 gNormal[];
layout (location = 3) in vec4 gTangent[];
//layout (location = 4) flat in ivec4 gIndexes;

// 
//layout (location = 0, xfb_buffer = 0, xfb_stride = 80, xfb_offset = 0 ) out vec4 fPosition;
//layout (location = 1, xfb_buffer = 0, xfb_stride = 80, xfb_offset = 16) out vec4 fTexcoord;
//layout (location = 2, xfb_buffer = 0, xfb_stride = 80, xfb_offset = 32) out vec4 fNormal;
//layout (location = 3, xfb_buffer = 0, xfb_stride = 80, xfb_offset = 48) out vec4 fTangent;
//layout (location = 4, xfb_buffer = 0, xfb_stride = 80, xfb_offset = 64) out vec4 fBinormal;
//layout (location = 5) flat out uvec4 uData;

//
layout (location = 0) out vec4 out_fPosition;
layout (location = 1) out vec4 out_fTexcoord;
layout (location = 2) out vec4 out_fNormal;
layout (location = 3) out vec4 out_fTangent;
layout (location = 4) out vec4 out_fBinormal;

//
out gl_PerVertex {   // some subset of these members will be used
    vec4 gl_Position;
    float gl_PointSize;
};

// Should to able used by OpenGL
void main() {
    const vec4 dp1 = gPosition[1] - gPosition[0], dp2 = gPosition[2] - gPosition[0];
    const vec4 tx1 = gTexcoord[1] - gTexcoord[0], tx2 = gTexcoord[2] - gTexcoord[0];
    const vec3 normal = normalize(cross(dp1.xyz, dp2.xyz));
    //const vec2 size  = textureSize(frameBuffers[IW_POSITION], 0);
    //const vec2 pixelShift = (staticRandom2() - 0.5f) / size;

    [[unroll]] for (uint i=0u;i<3u;i++) { // 
        gl_Position = gl_in[i].gl_Position;

        // 
        out_fPosition = gPosition[i];
        out_fTexcoord = gTexcoord[i]; // TODO: move texcoord by sample position for anti-aliased
        out_fTangent = gTangent[i];
        out_fNormal = gNormal[i];

        // 
        const float coef = 1.f / (tx1.x * tx2.y - tx2.x * tx1.y);
        const vec3 tangent = (dp1.xyz * tx2.yyy - dp2.xyz * tx1.yyy) * coef;
        const vec3 binorml = (dp1.xyz * tx2.xxx - dp2.xyz * tx1.xxx) * coef;
        if (!hasNormal()) { out_fNormal  = vec4(normal, 0.f); };

        // 
        if (!hasTangent()) { 
            out_fTangent .xyz = tangent; //- dot(fNormal.xyz,tangent.xyz)*fNormal.xyz;
            out_fBinormal.xyz = binorml; //- dot(fNormal.xyz,binorml.xyz)*fNormal.xyz;
        } else {
            out_fBinormal.xyz = cross(out_fNormal.xyz,out_fTangent.xyz);
            //fBinormal.xyz = fBinormal.xyz - dot(fNormal.xyz,fBinormal.xyz)*fNormal.xyz;
        };

        // 
        out_fNormal.xyz = normalize(out_fNormal.xyz);
        out_fTangent.xyz = normalize(out_fTangent.xyz);
        out_fBinormal.xyz = normalize(out_fBinormal.xyz);

        // 
        EmitVertex();
    };

    EndPrimitive();
};
