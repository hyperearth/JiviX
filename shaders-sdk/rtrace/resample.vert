#version 460 core // #
#extension GL_GOOGLE_include_directive  : require
#include "./driver.glsl"

// 
layout (location = 0) out vec4 gColor;
layout (location = 1) out vec4 gSample;
layout (location = 2) out vec4 gNormal;
layout (location = 3) out vec4 wPosition;
layout (location = 4) out vec4 gSpecular;
layout (location = 5) out vec4 gRescolor;

out gl_PerVertex {   // some subset of these members will be used
    vec4 gl_Position;
    float gl_PointSize;
};

// 
void main() {
    //const uint idx = gl_VertexIndex;
    const ivec2 size = imageSize(writeImages[0]  );
    const ivec2 f2fx = ivec2(gl_VertexIndex, gl_InstanceIndex);
    const ivec2 i2fx = ivec2(size.x,size.y-f2fx.y-1);

    // FROM PREVIOUS FRAME!!
          vec4 positions = imageLoad(writeImages[IW_POSITION],f2fx);
    const vec4 diffcolor = imageLoad(writeImages[IW_INDIRECT],f2fx);
    const vec4 normaling = imageLoad(writeImages[IW_GEONORML],f2fx);
    const vec4 speccolor = imageLoad(writeImages[IW_REFLECLR],f2fx);

    // 
    const uvec2 iIndices = floatBitsToUint(imageLoad(writeImages[IW_GEOMETRY],f2fx).xy);

    // By Geometry Data
    const uint globalInstanceID = iIndices.y;
    const uint nodeMeshID = getMeshID(rtxInstances[globalInstanceID]);
    const uint geometryInstanceID = iIndices.x;
    mat3x4 matras = mat3x4(instances[nodeMeshID].transform[geometryInstanceID]);
    if (!hasTransform(meshInfo[nodeMeshID])) {
        matras = mat3x4(vec4(1.f,0.f.xxx),vec4(0.f,1.f,0.f.xx),vec4(0.f.xx,1.f,0.f));
    };

    // By Instance Data
    const mat3x4 matra4 = rtxInstances[globalInstanceID].transform;

    // TODO: MESH USE TRANSFORMS!
    gl_PointSize = 0, gColor = 0.f.xxxx, gNormal.xxxx, wPosition = 0.f.xxxx;
    if (diffcolor.w > 0.f && imageLoad(writeImages[IW_MATERIAL],f2fx).z > 0.f && imageLoad(writeImages[nonuniformEXT(IW_INDIRECT)],f2fx).w > 0.01f) { // set into current 

        // Due real-time geometry, needs to transform!
        positions.xyz = mul4(mul4(vec4(positions.xyz, 1.f), matras), matra4).xyz;

        //
        gl_Position = vec4(world2screen(positions.xyz),1.f), gl_Position.y *= -1.f, gl_PointSize = 1.f;
        gColor = clamp(diffcolor, 0.001f, 10000000.f);
        gSpecular = vec4(speccolor.xyz,1.f);
        gSample = vec4(gl_Position.xyz,1.f), gSample.y *= -1.f;
        gNormal = vec4(normaling.xyz,1.f);
        wPosition = positions;
    };
};
