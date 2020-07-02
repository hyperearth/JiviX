#ifdef GLSL
#version 460 core // #
#extension GL_GOOGLE_include_directive  : require
#endif

#include "./driver.hlsli"

// 
#ifdef GLSL
layout (location = 0) out float4 gColor;
layout (location = 1) out float4 gSample;
layout (location = 2) out float4 gNormal;
layout (location = 3) out float4 wPosition;
layout (location = 4) out float4 gSpecular;
layout (location = 5) out float4 gRescolor;
layout (location = 6) out float4 gSmooth;

// some subset of these members will be used
out gl_PerVertex {
    float4 gl_Position;
    float gl_PointSize;
};

// 
struct GS_INPUT
{
    float4 vColor;
    float4 vSample;
    float4 vNormal;
    float4 vPosition;
    float4 vSpecular;
    float4 vRescolor;
    float4 vSmooth;
    float4 Position;
    float PointSize;
};
#else
// 
struct GS_INPUT
{
    float4 vColor    : COLOR0;
    float4 vSample   : COLOR1;
    float4 vNormal   : COLOR2;
    float4 vPosition : COLOR3;
    float4 vSpecular : COLOR4;
    float4 vRescolor : COLOR5;
    float4 vSmooth   : COLOR6;
    float4 Position  : SV_POSITION;
    float PointSize  : PSIZE0;
};
#endif

// 
#ifdef GLSL
void main() 
#else
GS_INPUT main(in uint VertexIndex : SV_VERTEXID, in uint InstanceIndex : SV_INSTANCEID) 
#endif
{
#ifdef GLSL
    const uint VertexIndex = gl_VertexIndex, InstanceIndex = gl_InstanceIndex;
#endif

    //const uint idx = gl_VertexIndex;
    const int2 size = imageSize(writeImages[0]);
    const int2 f2fx = int2(VertexIndex, InstanceIndex);
    const int2 i2fx = int2(size.x,size.y-f2fx.y-1);

    // FROM PREVIOUS FRAME!!
          float4 positions = imageLoad(writeImages[IW_POSITION],f2fx);
    const float4 diffcolor = imageLoad(writeImages[IW_INDIRECT],f2fx);
    const float4 normaling = imageLoad(writeImages[IW_GEONORML],f2fx);
    const float4 speccolor = imageLoad(writeImages[IW_REFLECLR],f2fx);
    const float4 smoothedc = imageLoad(writeImages[IW_SMOOTHED],f2fx);

    // 
    const uint2 iIndices = floatBitsToUint(imageLoad(writeImages[IW_GEOMETRY],f2fx).xy);

    // By Geometry Data
    const uint globalInstanceID = iIndices.y;
    const uint nodeMeshID = getMeshID(rtxInstances[globalInstanceID]);
    const uint geometryInstanceID = iIndices.x;

    // By Geometry Data
    float3x4 matras = float3x4(float4(1.f,0.f.xxx),float4(0.f,1.f,0.f.xx),float4(0.f.xx,1.f,0.f));
    float3x4 matra4 = rtxInstances[globalInstanceID].transform;
    if (hasTransform(meshInfo[nodeMeshID])) {
#ifdef GLSL
        matras = float3x4(instances[nodeMeshID].transform[geometryInstanceID]);
#else
        matras = float3x4(tmatrices[nodeMeshID][geometryInstanceID]);
#endif
    };

    // TODO: MESH USE TRANSFORMS!
#ifdef GLSL
    gl_PointSize = 0, gColor = 0.f.xxxx, gNormal.xxxx, wPosition = 0.f.xxxx;
#endif

    GS_INPUT output;
    output.PointSize = 0, output.vColor = 0.f.xxxx, output.vNormal.xxxx, output.vPosition = 0.f.xxxx;
    if (diffcolor.w > 0.f && imageLoad(writeImages[IW_MATERIAL],f2fx).z > 0.f && imageLoad(writeImages[nonuniformEXT(IW_INDIRECT)],f2fx).w > 0.01f) { // set into current 

        // Due real-time geometry, needs to transform!
        positions.xyz = mul4(mul4(float4(positions.xyz, 1.f), matras), matra4).xyz;

        //
        output.PointSize = 1.f;
        output.Position = float4(world2screen(positions.xyz),1.f), output.PointSize = 1.f;
        output.Position.y *= -1.f;
        output.vColor = clamp(diffcolor, 0.001f, 10000000.f);
        output.vSpecular = float4(speccolor.xyz,1.f);
        output.vSample = float4(output.Position.xyz,1.f);
        output.vNormal = float4(normaling.xyz,1.f);
        output.vSmooth = smoothedc;
        output.vPosition = positions;
    };

#ifdef GLSL
    vColor = output.vColor;
    vSpecular = output.vSpecular;
    vSample = output.vSample;
    vNormal = output.vNormal;
    vSmooth = output.vSmooth;
    vPosition = output.vPosition;
    gl_Position = output.Position;
    gl_PointSize = output.PointSize;
#else
    return output;
#endif
};
