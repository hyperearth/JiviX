#version 460 core // #
#extension GL_GOOGLE_include_directive  : require
#include "./driver.hlsli"

// 
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
void main() {
    //const uint idx = gl_VertexIndex;
    const int2 size = imageSize(writeImages[0]  );
    const int2 f2fx = int2(gl_VertexIndex, gl_InstanceIndex);
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
        matras = float3x4(instances[nodeMeshID].transform[geometryInstanceID]);
    };

    // TODO: MESH USE TRANSFORMS!
    gl_PointSize = 0, gColor = 0.f.xxxx, gNormal.xxxx, wPosition = 0.f.xxxx;
    if (diffcolor.w > 0.f && imageLoad(writeImages[IW_MATERIAL],f2fx).z > 0.f && imageLoad(writeImages[nonuniformEXT(IW_INDIRECT)],f2fx).w > 0.01f) { // set into current 

        // Due real-time geometry, needs to transform!
        positions.xyz = mul4(mul4(float4(positions.xyz, 1.f), matras), rtxInstances[globalInstanceID].transform).xyz;

        //
        gl_Position = float4(world2screen(positions.xyz),1.f), gl_PointSize = 1.f;
        gl_Position.y *= -1.f;
        gColor = clamp(diffcolor, 0.001f, 10000000.f);
        gSpecular = float4(speccolor.xyz,1.f);
        gSample = float4(gl_Position.xyz,1.f);
        gNormal = float4(normaling.xyz,1.f);
        gSmooth = smoothedc;
        wPosition = positions;
    };
};
