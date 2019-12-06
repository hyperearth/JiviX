#define SAMPLES 0
#define DIFFUSE 1
#define REFLECT 2
#define COLORED 3
#define NORMALS 4
#define NORMMOD 5
#define PARAMET 6
#define DENOISE 7
#define OUTPUTS 8
#define DEPTHST 9

// Using Transposed, DirectX styled Matrices
layout (binding = 0, scalar) uniform Matrices {
    mat4 prvproject;
    mat4 projection;
    mat4 projectionInv;
    mat4x3 prevmodel;
    mat4x3 modelview;
    mat4x3 modelviewInv;
};

// 
layout (binding = 2) uniform sampler2D frameBuffers[];
layout (binding = 3, scalar) buffer Attributes { vec4 data[]; } attris[];
layout (binding = 4, rgba32f) uniform image2D writeImages[];
layout (binding = 5) uniform texture2D textures[];
layout (binding = 6) uniform sampler samplers[];
