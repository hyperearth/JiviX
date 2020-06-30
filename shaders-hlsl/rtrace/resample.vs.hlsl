#include "./driver.hlsli"
#include "./global.hlsli"

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
    float PointSize  : PSIZE0;
    float4 Position  : SV_Position;
};

// 
GS_INPUT main(in uint VertexIndex : SV_VERTEXID, in uint InstanceIndex : SV_INSTANCEID) {
    //const uint idx = gl_VertexIndex;
          int2 size = int2(0,0); writeImages[0].GetDimensions(size.x, size.y);
    const int2 f2fx = int2(VertexIndex, InstanceIndex);
    const int2 i2fx = int2(size.x,size.y-f2fx.y-1);

    // FROM PREVIOUS FRAME!!
          float4 positions = writeImages[IW_POSITION][f2fx];
    const float4 diffcolor = writeImages[IW_INDIRECT][f2fx];
    const float4 normaling = writeImages[IW_GEONORML][f2fx];
    const float4 speccolor = writeImages[IW_REFLECLR][f2fx];
    const float4 smoothedc = writeImages[IW_SMOOTHED][f2fx];

    // 
    const uint2 iIndices = asuint(writeImages[IW_GEOMETRY][f2fx].xy);

    // By Geometry Data
    const uint globalInstanceID = iIndices.y;
    const uint nodeMeshID = getMeshID(rtxInstances[globalInstanceID]);
    const uint geometryInstanceID = iIndices.x;
    float3x4 matras = transforms[nodeMeshID][geometryInstanceID];
    if (!hasTransform(meshInfo[nodeMeshID])) {
        matras = float3x4(float4(1.f,0.f.xxx),float4(0.f,1.f,0.f.xx),float4(0.f.xx,1.f,0.f));
    };

    // TODO: MESH USE TRANSFORMS!
    GS_INPUT output;
    output.PointSize = 0, output.vColor = 0.f.xxxx, output.vNormal.xxxx, output.vPosition = 0.f.xxxx;
    if (diffcolor.w > 0.f && writeImages[IW_MATERIAL][f2fx].z > 0.f && writeImages[IW_INDIRECT][f2fx].w > 0.01f) { // set into current 

        // Due real-time geometry, needs to transform!
        positions.xyz = mul4(mul4(float4(positions.xyz, 1.f), matras), rtxInstances[globalInstanceID].transform).xyz;

        //
        output.Position = float4(world2screen(positions.xyz),1.f), output.PointSize = 1.f;
        output.Position.y *= -1.f;
        output.vColor = clamp(diffcolor, 0.001f, 10000000.f);
        output.vSpecular = float4(speccolor.xyz,1.f);
        output.vSample = float4(output.Position.xyz,1.f);
        output.vNormal = float4(normaling.xyz,1.f);
        output.vSmooth = smoothedc;
        output.vPosition = positions;
    };
    return output;
};
